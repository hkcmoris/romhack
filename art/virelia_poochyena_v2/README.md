# Virelian Poochyena v2

**Rejected art pass.** The pre-v2 runtime assets and species offsets have been
restored. This folder preserves the rejected work; running its exporter would
reinstall v2. The subsequent review candidate is in `../virelia_poochyena_v3/`.

An original pixel-art redraw authored through diivi's Aseprite MCP, formerly
installed in `graphics/pokemon/poochyena/virelia/`. The design is a stocky moss-hyena
puppy with sage fur, a fern ruff, charcoal muzzle and paws, an amber eye,
a pale throat patch and a leaf-shaped tail. This replaces Poochyena's art
only; the other regional stages and ordinary Poochyena retain their assets.

## Complete set

| Runtime asset | Layout |
| --- | --- |
| `front.png`, `front_2.png` | Two 64x64 poses; stationary paws, one-pixel breath and tail follow-through |
| `anim_front.png` | The two front frames stacked vertically, 64x128 |
| `back.png` | 64x64 close head/shoulders portrait; torso meets the bottom crop |
| `overworld.png` | Six 32x32 walking cells in a 192x32 strip: down, down step, up, up step, left, left step |
| `icon.png` | Two 32x32 party-icon cells in a 32x64 strip, using unchanged engine palette 1 |
| `normal.pal`, `overworld_normal.pal` | The same 16-entry sage/fern palette |
| `shiny.pal`, `overworld_shiny.pal` | The same 16-entry blue-sage/violet-leaf shiny palette |

The engine mirrors the side walking cells for the right-facing direction.
Footprint and cry still use ordinary Poochyena, as registered by the species.
The battle front offset is 8; the back portrait offset is 0 with 48x56
content dimensions. Existing battle animation commands remain in use.

## Editable sources and export

- `source/` contains layered `front.aseprite` and `back.aseprite`, plus
  animated `overworld.aseprite` and `icon.aseprite`.
- `pixel-design.json` records the hand-authored pixel rows, palette meanings,
  layer positions and the front animation's second-frame offsets.
- `exports/` contains the eleven native-size PNG frames exported by MCP.
  `icon_full_palette.png` preserves the custom-color two-frame icon artwork.
- `before/` preserves the ten original runtime files for comparison.
- `previews/review.png` shows the actual normal, shiny, overworld and runtime
  icon assets; `before-after.png` compares the battle views. Animated front
  and custom-color icon previews are also included.

After editing a source, export each frame through Aseprite MCP at scale 1
using its existing name in `exports/`, then run:

```powershell
python art/virelia_poochyena_v2/export_assets.py
python tools/validate_virelia_overworld.py
```

The exporter requires Pillow. It packs existing pixels without resampling
or quantization, writes 4-bit indexed PNGs with only index 0 transparent, and
checks dimensions, exact palette matches, fixed front paws, distinct pose
pairs, RGB555 colors, follower baseline and the back crop. Only the runtime
party icon uses an explicit semantic mapping to shared palette 1; the
shared palette itself is unchanged. `verification.json` records the results
and runtime file hashes.

The eleven authoring frames and all six runtime PNGs were visually checked.
Native gbagfx conversion and index round-trip checks are recorded separately
in `native-verification.json`. To repeat these checks, run
`bash art/virelia_poochyena_v2/validate-native.sh` in the Linux build environment,
then `python art/virelia_poochyena_v2/validate_native.py` with Pillow available.
A full ROM build and emulator playtest have
not been performed for this artwork change.
