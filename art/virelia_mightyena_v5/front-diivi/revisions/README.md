# Virelian Mightyena v5 - one front battle sprite

This is one static 64x64 front frame, drawn through diivi's Aseprite MCP in the active Aseprite Live session. It uses the first frame of the repository's original GBA Mightyena front sprite as its pose/anatomy base, with the Virelian concept v2 as the design reference.

The adult pose, facial landmarks, foot registration and raised tail are retained. Regional edits add the olive/moss coat, charcoal face and lower paws, larger leaf-fur planes on the forehead, mane, chest and tail, and small mint ghost wisps. The effect uses two palette entries and 30 hand-placed pixels; it remains secondary to the leaf coat.

- mightyena-front-v1.aseprite: layered, editable native source.
- front.png: 64x64 indexed PNG, four-bit encoding and transparent index 0.
- normal.pal: ordered 16-entry JASC palette, including 15 opaque colors.
- front-rgba.png: direct native-size diivi export before indexing.
- front-8x.png: enlarged preview.
- comparison-6x.png: original GBA sprite and the new sprite on light/dark backgrounds.
- proofs-front-v1/: native-size/RGB555 proofs and read-only technical audit.
- front-design.json: palette roles, drawing patches, wisps, metrics and review record.
- original-gba-front.png: the unscaled source frame used for construction; original-front.png records the other existing front-art option studied.

The GBA audit passes. Visible colors and alpha are preserved exactly during indexed encoding; no resampling or palette quantization was used. The body and whole composite each have one connected component, with no detached or lone opaque pixels. Lint's color-neighbor warnings are review coordinates for existing outline/eye/claw details, leaf transitions and intentional wisps, not an automatic deletion mask.

Static proofs were inspected at 1x and enlarged, on light/dark backgrounds and with RGB555 colors. This is an art deliverable for review; no additional animation frame, shiny redesign, ROM build or runtime replacement is included.

For this running session, open http://localhost:7778/ and select mightyena-front-v1.aseprite to see the saved drawing passes. Ports can change after a restart.
