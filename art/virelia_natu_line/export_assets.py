"""Lossless game-format packing of the already edited native-grid sprite sheets.

Artwork editing is performed with imagegen; this script only normalizes palette
indices and packs existing cells into the repository's PNG/JASC formats.
"""
from pathlib import Path
from PIL import Image
import hashlib
import json

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parent.parent
DESTINATIONS = {
    'natu': 'graphics/pokemon/natu/virelia',
    'xatu': 'graphics/pokemon/xatu/virelia',
    'nimbatu': 'graphics/pokemon/nimbatu',
}
RECTS = {
    'front.png': (0, 0, 64, 64),
    'front_2.png': (0, 64, 64, 128),
    'anim_front.png': (0, 0, 64, 128),
    'back.png': (64, 0, 128, 64),
    'icon.png': (128, 0, 160, 64),
}

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def encode_palette(source):
    with Image.open(source) as original:
        assert original.mode == 'P' and original.size == (192, 128)
        rgba = original.convert('RGBA')
        alpha = set(rgba.getchannel('A').tobytes())
        assert alpha == {0, 255}, (source, alpha)
        pixels = list(original.get_flattened_data())
        used = sorted(set(pixels))
        raw_pal = original.getpalette()
        transparent = [i for i in used if rgba.getpixel((pixels.index(i) % 192, pixels.index(i) // 192))[3] == 0]
        assert len(transparent) == 1
        order = transparent + [i for i in used if i not in transparent]
        assert len(order) <= 16
        remap = {old: new for new, old in enumerate(order)}
        palette = [component for i in order for component in raw_pal[i*3:i*3+3]]
        palette += [0] * (48-len(palette))
        packed = Image.new('P', original.size)
        packed.putpalette(palette)
        packed.putdata([remap[i] for i in pixels])
        packed.info['transparency'] = 0
        assert packed.convert('RGBA').tobytes() == rgba.tobytes()
        return packed, palette

report = {'artwork_tool': 'built-in imagegen', 'gameplay_modified': False,
          'species_registration_modified': False, 'normal_colors_only': True,
          'source_snapshot': '01_originals', 'species': {}}
for name, relative in DESTINATIONS.items():
    dest = REPO / relative
    dest.mkdir(parents=True, exist_ok=True)
    packed, palette = encode_palette(ROOT / '03_concept_edits' / (name + '.png'))
    assets = {}
    for filename, rect in RECTS.items():
        cell = packed.crop(rect)
        cell.putpalette(palette)
        cell.info['transparency'] = 0
        cell.save(dest / filename, bits=4, transparency=0, optimize=False)
        with Image.open(dest / filename) as check:
            assert check.mode == 'P' and check.info.get('transparency') == 0
            assert check.getpalette()[:48] == palette
            assert check.size == (rect[2]-rect[0], rect[3]-rect[1])
            assert set(check.convert('RGBA').getchannel('A').tobytes()) <= {0, 255}
            assert any(check.convert('RGBA').getchannel('A').tobytes())
            assets[filename] = {'size': check.size, 'colors': len(check.getcolors()), 'sha256': digest(dest/filename)}
    assert (dest/'front.png').read_bytes() != (dest/'front_2.png').read_bytes()
    rows = [palette[i:i+3] for i in range(0, 48, 3)]
    (dest / 'normal.pal').write_text('JASC-PAL\n0100\n16\n' + ''.join(' '.join(map(str,row))+'\n' for row in rows), encoding='ascii')
    report['species'][name] = {'destination': relative, 'palette_colors': len(packed.getcolors()), 'assets': assets}

xatu_copy = ROOT/'02_recolored/xatu.png'
nimbatu_copy = ROOT/'02_recolored/nimbatu.png'
assert xatu_copy.read_bytes() == nimbatu_copy.read_bytes()
report['nimbatu_recolor_origin_equals_xatu'] = True
for base in ['natu', 'xatu']:
    for source in (ROOT/'01_originals'/base).iterdir():
        assert source.read_bytes() == (REPO/'graphics/pokemon'/base/source.name).read_bytes()
report['originals_unchanged'] = True
(ROOT/'asset_manifest.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report,indent=2))
