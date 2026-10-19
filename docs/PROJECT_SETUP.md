# Project setup

## Baseline

- Project repository: <https://github.com/hkcmoris/romhack>
- Engine remote: `upstream`, <https://github.com/rh-hideout/pokeemerald-expansion>
- Imported release: `expansion/1.17.1`
- Engine commit: `bb1093a7c72d3961d9a37d8e02ad2abc2c866682`

The release was merged with its full history into the original repository.
Keep this ancestry when integrating the setup branch into `main`: use a regular
merge, rather than a squash merge. No game-specific feature settings have been
changed from the release baseline.

## Windows development with WSL2

Use Ubuntu under WSL2 and keep the working checkout in the Linux filesystem.
The upstream [WSL guide](install/windows/WSL.md) explains the supported setup.
The upstream MSYS2 and Cygwin guides currently report build problems with those
environments.

If WSL is missing, run this in an administrator PowerShell terminal:

```powershell
wsl --install -d Ubuntu
```

Restart Windows if prompted, launch Ubuntu, and finish its username/password
setup. Confirm that Ubuntu uses WSL2 with `wsl --list --verbose` in PowerShell;
use `wsl --set-version Ubuntu 2` if necessary.

Inside Ubuntu, install the development toolchain:

```bash
sudo apt-get update
sudo apt-get install -y build-essential binutils-arm-none-eabi gcc-arm-none-eabi libnewlib-arm-none-eabi git libpng-dev python3
```

To copy this prepared repository, including local commits that have not been
pushed, into WSL's filesystem:

```bash
mkdir -p ~/projects
git clone --no-hardlinks /mnt/c/Users/morav/Documents/github/romhack ~/projects/romhack
cd ~/projects/romhack
git remote set-url origin https://github.com/hkcmoris/romhack
git remote add upstream https://github.com/rh-hideout/pokeemerald-expansion
git config core.autocrlf input
```

Commit any outstanding Windows-side edits before cloning: cloning transfers
commits, not uncommitted working files. Use the Linux checkout for subsequent
development and open that same folder in Porymap or a WSL-aware editor. The
Windows and Linux checkouts do not automatically synchronize edits.

After this branch has been pushed, a fresh clone can instead use
`git clone https://github.com/hkcmoris/romhack ~/projects/romhack` and then check
out the desired branch. Git remotes other than `origin` are local settings, so
add `upstream` again in each fresh clone.

## Build and validate

Run these commands from the repository root in Ubuntu:

```bash
# Strict development build; produces pokeemerald.gba.
make -j"$(nproc)" -O all UNUSED_ERROR=1 DEPRECATED_ERROR=1

# Optimized release build; produces pokeemerald-release.gba locally.
make -j"$(nproc)" release UNUSED_ERROR=1 DEPRECATED_ERROR=1

# Automated engine tests using the bundled headless mGBA test harness.
make -j"$(nproc)" check UNUSED_ERROR=1 DEPRECATED_ERROR=1

# Documentation index and patch whitespace checks.
python3 .github/docs_validate/inclusive_summary.py
git diff --check
```

`make debug` builds with debug symbols. See the upstream
[testing guide](tutorials/how_to_testing_system.md) for selecting individual tests.
Test maps, progression, saves, and time behavior in-game as well; battle tests
do not cover an entire playthrough.

This is a C/GBA project with a Makefile. It has no `package.json`, npm
dependencies, or npm format/lint/test scripts. The compiler's warning checks,
engine tests, and upstream style guide are the native validation tools.
Preserve the imported source style and the existing `.gitattributes` rules,
including the palette files' required CRLF endings.

CI validates builds and engine tests on pushes to `main` and `codex/**`, on
pull requests, and when manually dispatched. It retains upstream FireRed and
LeafGreen build checks for compatibility. Builds remain inside the runner;
the workflow does not upload ROMs. The aggregate `build` check also requires
the documentation index check. Upstream documentation deployment and label
workflows are restricted to the upstream repository.

## Editing tools and source locations

- [Porymap](https://github.com/huderlem/porymap): open the repository root to
  edit maps, events, warps, and wild encounters. On WSL2, open the Linux checkout
  through `\\wsl.localhost\Ubuntu\home\<user>\projects\romhack`.
- [Poryscript](https://github.com/huderlem/poryscript): optional higher-level
  event scripting. Its build integration must be configured before using
  `.pory` scripts; the baseline continues to use the included script files.
- [Porytiles](https://github.com/grunt-lucas/porytiles): optional custom tileset
  compilation; follow its integration guide when adding custom map art.
- [mGBA](https://mgba.io/): open `pokeemerald.gba` or
  `pokeemerald-release.gba` to playtest the corresponding build.
- `include/config/`: engine switches, species selection, caps, AI, and DexNav.
- `data/maps/`: map data and event scripts; use Porymap for structural edits.
- `src/`: engine implementation and game data.
- `graphics/` and `sound/`: visual and audio assets.
- `test/`: engine regression tests.

Keep generated ROMs and emulator saves out of Git; the inherited `.gitignore`
already excludes them. Preserve [RHH and contributor credits](../CREDITS.md)
when adding project-specific credits.

## Updating the engine

Start with a clean working tree and create a separate update branch. Check the
release notes and migration instructions before selecting the next release:

```bash
git switch -c codex/update-expansion
git fetch upstream --tags
git merge expansion/X.Y.Z
```

Replace `X.Y.Z` with the release you reviewed. Resolve conflicts, perform any
required save/data migrations, and repeat the build and test commands above
before merging the update. Keep updates incremental as described in the
[upstream installation and migration guide](../INSTALL.md).
