# Natu line: skill validation and repairs

Reviewed 2026-10-26 with the comprehensive gba-pokemon-sprites skill. The current runtime consumers in src/data/graphics/pokemon.h and species/follower definitions supplied the contract. Comparisons used the unchanged original Natu/Xatu art and the current approved aquatic designs.

## Result

All static visual criteria pass after the three repairs below. All 15 audited sheets pass the technical audit, and the project's gbagfx converter successfully exports all 15 PNGs plus nine external palettes. Timed playback and in-game acceptance remain **not checked**: both browser control and the native UI helper timed out. The supplied GIF is a review artifact, not evidence that playback was observed. No ROM was built or transferred.

The review covers two 64x64 battle-front frames, a 64x64 back, six 32x32 follower frames, and two 32x32 party-icon frames per species. Both full-art and shared-palette icon sheets were audited. Native grids, nearest-neighbor closeups, light/dark backgrounds, and actual runtime palettes with RGB555 approximations were inspected. Water shiny colors currently equal normal colors by design.

## Repairs

| Failure before repair | Final change | Preserved |
| --- | --- | --- |
| Xatu icon: the first pose crossed the 32-row cell boundary. Its foot pixels at sheet y=32 became a floating mark at local y=0 in frame 1. | Repacked frame 0 one pixel upward, including its complete foot, and removed that overflow from frame 1. Updated icon.png and runtime icon_game.png. | Every opaque palette index and its count; second pose; shared engine palette 3. Both poses now have complete feet ending at local y=31. |
| Nimbatu back: native x=33..52/y=27..37 contained a blue band on the lit left side and an overly broad band under the head, reading as a second contour and ring shading. | Imported 31 generated material-color changes within the head. Teal now reaches the single outer contour on the lit side; the blue form shadow sits predominantly below/right. | Exact silhouette, dark contour pixels, crest, cheek, beak, collar, wings, palette, crop and registration. |
| Nimbatu front frame 1: opaque bounds y=25..63 versus frame 0 y=12..58. Head and feet drifted despite the recorded subtle-flap design. | Derived frame 1 from unchanged frame 0, importing only generated outer-wingtip rectangles x=2..12/y=15..30 and x=51..62/y=15..30. The two frames differ in 140 pixels, confined to those selections. | Head, crest, torso, shoulders, tail, legs and feet exactly identical between the frames. Ground and body registration match; frame 0 is unchanged. |

Image edits used built-in imagegen. Native export selected the nominated generated regions, quantized them to the existing palette, and packed the indexed sheets. The unrestricted generated images were not used as replacements. Exact prompts and selection policies are in repair-prompts.json and the main imagegen-prompts.json.

## Visual acceptance record

These are manual visual judgments, separate from the automated technical result.

| Criterion | Natu | Xatu | Nimbatu | Evidence / interpretation |
| --- | --- | --- | --- | --- |
| Known native grid | Pass | Pass | Pass | Indexed final exports; 64x64 battle, 32x32 follower/icon cells; no soft resampling. |
| Construction, scale, camera | Pass | Pass | Pass | Distinct stage silhouettes; rear portraits retain their approved camera and crop. |
| Single contour width | Pass | Pass | Pass after repair | Traced head, back, wing and tail boundaries in enlarged final pixels; no unsupported duplicate contour lanes remain. |
| Joins, curves and pointed features | Pass | Pass | Pass | One-pixel steps; crest and flight-feather tips are intentional points, not rounded-body jaggies. |
| Dark-region role | Pass | Pass | Pass | Filled pupils, dark collar plumage, feather interiors, leg fills and compact overlap shadows are supported anatomy/materials, not strokes to erode. |
| Lighting and volume | Pass | Pass | Pass after repair | Above/upper-left light; compact turning-away/overlap shadows. Nimbatu's head band is reduced without lightening its navy collar. |
| Shade clusters and materials | Pass | Pass | Pass after repair | Large feather/plumage regions, restrained highlights; gold stripes are markings, not specular perimeter rings. |
| Normal/shiny/palette roles | Pass | Pass | Pass | Actual external palettes; RGB555 proof; current normal-equals-shiny policy retained. Transparent slot 0 RGB is irrelevant to the GBA mask. |
| Cross-view identity | Pass | Pass | Pass | Aquatic colors, crests, cheek markings, collar/gold chest details and stage-specific tail/wing shapes correspond across views. |
| Static frame registration | Pass | Pass after icon repair | Pass after front repair | Natu follower edits preserved. Standing Nimbatu head/core/ground now fixed; Xatu icon no longer crosses a cell. |
| Timed animation playback | Not checked | Not checked | Not checked | UI controls failed to initialize; no playback or in-game claim. |
| Indexed export regression | Pass | Pass | Pass | Final indexed images re-inspected on contrasting backgrounds; audit and actual converter pass. |

## Technical and preservation checks

- 15 indexed-sheet audits passed: anim_front.png, back.png, overworld.png, icon.png and icon_game.png for each of the three species.
- All follower pairs and front/icon pairs remain distinct; index 0 is transparent; used indices fit the 16-entry budget.
- Actual gbagfx output sizes: front 4096 bytes, back 2048, icon 1024, follower 3072 using -mwidth 4 -mheight 4. All nine 32-byte palette binaries match expected RGB555 bytes.
- Remaining edge warnings are explained: Xatu's complete icon feet end at y=31; Nimbatu's rear torso intentionally continues through the bottom crop. Nimbatu's front no longer touches the bottom edge.
- All Natu assets, every follower sheet, every external palette, Nimbatu's first front frame, and unrelated runtime sprites are byte-identical to the initial snapshot.
- Editable source-board cells match every current battle/icon export. Raw generation history remains untouched; current previews and asset_manifest.json are refreshed.
- No game code, dependencies, species configuration, save data or build configuration changed.

verification.json contains hashes, exact head-pixel changes, per-sheet results and converter results. Original inputs and raw generated candidates remain in outputs/natu-line-skill-review-20261026/ for local rollback and inspection.

## Inspect

- before-after.png: corrected icon overflow, head shading, and second battle pose.
- nimbatu-front-comparison.gif: old/new two-frame comparison at a deliberately slowed 500 ms per frame.
- natu-final-rgb555.png, xatu-final-rgb555.png, nimbatu-final-rgb555.png: final runtime-colored views on light/dark backgrounds.

Repository checks: npm format and npm lint were attempted, but npm is not installed on this host; this C/GBA checkout also has no package.json scripts. No JavaScript files changed. git diff --check passes. The successful native asset audit/converter checks are recorded above.
