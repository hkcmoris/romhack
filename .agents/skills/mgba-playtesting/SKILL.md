---
name: mgba-playtesting
description: Playtest a Virelia ROM in mGBA. Use for reproducing in-game bugs, checking maps, visuals or progression with controller input, verifying FLASH Continue, or selecting the matching headless engine regression.
---

# mGBA playtesting

Paths and shell commands below assume the repository root. Identify the scenario, the starting state, and the observation that distinguishes success from failure.

## Select the build

- For ordinary story play, use the requested revision's `emerald-release` artifact or build the working tree with `make -j4 release` in a configured Linux/WSL checkout. Record the commit, any local changes, and the SHA-256 of the actual ROM (`sha256sum path/to/game.gba` or PowerShell `Get-FileHash -Algorithm SHA256 path/to/game.gba`). A CI artifact covers only the source in that run.
- For CI, use the artifact names and mode switches in `.github/workflows/build.yml`. On Windows, `docs/PROJECT_SETUP.md` describes the download workflow. Select the run by commit and extract into a fresh directory. The version embedded in the filename can change. The CI job that uploads `emerald-development` also uploads its `emerald-symbols`. Match every symbol or ELF file used for memory inspection to the exact ROM build, including optimization mode.
- `emerald-development` plays the ordinary story. For the Link test NPCs and quick start, build with `make -j4 all VIRELIA_DEVELOPMENT_MODE=1`. Use `docs/DUO_MULTIPLAYER_TEST.md` for the two-player scenario with separate saves and identical ROMs.

## Exercise the requested behavior

1. Open the selected `.gba` in desktop mGBA, or use an available mGBA libretro frontend for scripted controller input. Keep test saves, states and captures together in a fresh run directory under ignored `outputs/`. Preserve the player's original save. Start with New Game for changed species, save layouts or progression. Record any prerequisite flags, warps or injected items used to reach a scene.
2. Follow the relevant chapter's route in `docs/VIRELIA_PROLOGUE.md`, `docs/VIRELIA_FIRST_GYM.md` or `docs/VIRELIA_RAINFALL_SEABREAK.md`. Drive the transition being tested with ordinary input. For example, approach a bridge from both sides with the follower visible and capture the entry, midpoint and exit at 240×160. For a quest reward, check inventory and the result of a second interaction.
3. For a persistence claim, use the in-game Save command, wait for completion, close the core, then boot the same ROM using only the resulting FLASH save and choose Continue. Verify the location and changed party, inventory, storage or quest state. Keep emulator states for replay checkpoints within the same build.
4. For repeatable desktop input in mGBA 0.10 or later, load the ROM and open **Tools → Scripting**. A Lua `frame` callback can schedule `emu:addKey(C.GBA_KEY.A)` and `emu:clearKey(C.GBA_KEY.A)` on separate frames and capture `emu:screenshot(path)` at a checkpoint. Trace the input frame, map or screen, and expected effect. Match the API to the installed frontend. [Official mGBA scripting API](https://mgba.io/docs/scripting.html)

For a libretro replay, inspect the tracked evidence example `docs/images/virelia-rainfall/native-verification.json`. Local helpers under `outputs/rainfall/native/`, when present, contain build-specific paths, memory layouts and checkpoints. Inspect these before reuse and direct new captures to the current run.

## Select and report checks

For a compiled engine assertion, find its test name under `test/`. In a configured Linux/WSL checkout, `make -j4 check TESTS="Virelia Maplebrook"` selects the named prefix in `test/virelia_maplebrook.c`. Check the executed count and skips. `make -j4 check` runs the full suite using the bundled headless runner. Add `VIRELIA_DEVELOPMENT_MODE=1` when exercising that variant. See `docs/tutorials/how_to_testing_system.md` and `tools/mgba/README.md`.

Report engine assertions, controller observations and save/reload results separately. For each observed issue give the build hash, emulator/frontend version, starting save or fixture, shortest input sequence, expected and actual result, and relevant screenshot or frame. Distinguish a seeded scene check from an uninterrupted playthrough. For device-specific reports, identify the device/emulator coverage separately using `docs/MYBOY_COMPATIBILITY.md`.
