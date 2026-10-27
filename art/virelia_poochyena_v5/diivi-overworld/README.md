# Virelian Poochyena v5 - approved diivi overworld

Drawn at the native pixel grid through diivi's Aseprite MCP. The original Poochyena overworld poses, scale, face placement and alternating paw contacts were studied using its external normal palette. The regional design retains the approved v5 moss/olive coat, charcoal face and paws, red nose, yellow/red eyes, small fangs and a compact layered leaf-fan tail.

The down/up footprints are 13x17 at (9,13), matching the original. Side views are 23x18 at (7,12), allowing one extra ear pixel and the regional leaf tail. All feet finish at y=29. Each direction has two 180 ms frames; right-facing movement uses the engine's left-frame mirroring.

- overworld-v5.aseprite: editable six-frame source, with Down/Up/Left tags.
- overworld.png: 192x32, six horizontal 32x32 cells, ordered down A/B, up A/B, left A/B.
- overworld_normal.pal: approved ordered 16-entry palette; index 0 transparent.
- overworld-rgba.png: exact native RGB export from diivi before indexing.
- sheet-6x.png: enlarged presentation proof.
- comparison.png: original, previous v5, and this pass at the same scale.
- down.gif, up.gif, left.gif: exported walking pairs.
- proofs/: native/light/dark/RGB555 proofs, pair GIFs and the technical audit.
- review.json: drawing method, frame bounds, pair differences, palette statistics, lint and review limits.

The indexed export preserves all visible colors and alpha exactly; no quantization or resampling was used for the asset. The GBA auditor passed with four-bit PNG encoding, 13 opaque colors, index 0 transparency and distinct pairs. All six MCP lint reports have one connected component and no detached or lone opaque pixels. Single-color-neighbor warnings include intentional eyes, fangs, nose, ear tips and shade transitions; they are not an automatic cleanup mask.

Native-size and enlarged static proofs were inspected, including normal/RGB555 and light/dark backgrounds. Timed visual playback was not independently verified because the browser and native UI automation helpers timed out. The user approved this walking sheet. It is installed as `graphics/pokemon/poochyena/virelia/overworld.png` by `tools/virelia/prepare_canine_v5_assets.py`, which supersedes the older v5 walking export and remaps the existing blue shiny palette to the approved index roles. No manual in-game inspection is recorded.

The working Aseprite Live session for this run is http://localhost:7778/; select overworld-v5.aseprite. Its saved timeline contains the drawing passes. The port is session-specific and may change after a restart.
