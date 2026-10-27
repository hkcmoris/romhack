# Quarry quest tools - scratch redraw

Four static 16x16 sprites were drawn from blank RGB canvases through diivi's
Aseprite MCP: wrench, hammer, pick and tool_hoard. Original graphics were viewed
as references for the runtime size and palette; their pixels were not reused.
The new silhouettes were checked as flat masks before adding material colors.

- source/: editable native .aseprite files, one frame each.
- exports/: native-size RGBA frame exports used by the game packer.
- native-pixels.json: exact final pixel rows and color legend.
- drawing-plan.json: silhouettes, palette roles and material decisions.
- references/original/: the previous four runtime images for comparison.
- previews/review.png: previous/new at 1x and 8x, plus actual quarry tile context.
- aseprite-audit.json: final color counts and topology checks.
- export-verification.json: source/export geometry, index and alpha checks.

The active live viewer for this drawing session is http://localhost:7778/.
Select a file from source/ to inspect the saved drawing passes. The port can
change after restarting the local server.

## Export and install

Export frame 1 through diivi to exports/<name>.png at scale 1, then run:

```text
python tools/virelia/quarry_objects.py
python tools/validate_virelia_quarry.py
python tools/validate_virelia_missing_tools.py
bash art/quarry_tools_v2/validate-native.sh
```

The packer uses only exact palette lookup. It performs no drawing, resizing,
nearest-color quantization, antialiasing or dithering on these tools. Runtime
files are graphics/object_events/pics/quarry/<name>.png, indexed at four bits
with transparent index 0. They use the existing moving_box palette and 128-byte
static 16x16 object graphics. The separately generated ore_crate is unchanged.

The five-to-eight opaque colors per sprite distinguish cool metal from warm
wood under upper-left lighting. Every drawing has a one-pixel canvas margin,
one connected component, and no detached or lone opaque pixels. Same-color
neighbor warnings mark intentional jaw corners, small highlights, grip caps
and crate nails; those were reviewed rather than removed automatically.
The bottom preview row simulates RGB555 on real map tiles; it is an offline
map render, not an emulator screenshot.

## Validation

All four Missing Tools native quest tests and both quarry validators pass.
Native gbagfx round-trips preserve every index in all four 128-byte sprites,
and the shared palette matches its expected RGB555 words. The playable ROM
build succeeds; each new graphics payload was verified inside the ROM byte
for byte. The live viewer tracks all four saved sources without render errors.
Static native-size, enlarged and map-context proofs were reviewed. No manual
emulator playtest was performed. npm format and npm lint are unavailable on
this PC because npm is not installed.
