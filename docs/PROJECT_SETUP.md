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

## Virelia story and development modes

The current Emerald release starts the [Virelia prologue](VIRELIA_PROLOGUE.md).
The old quick start and starter/Link test NPCs require the explicit build option
`VIRELIA_DEVELOPMENT_MODE=1`. This adds `-devmode` to the output filename and
uses separate object directories. The default `emerald-development` artifact
uses story mode with ordinary compiler settings; `emerald-release` uses story
mode with release optimizations. No multiplayer implementation is changed.

## Windows development with GitHub Actions

Edit this checkout with Windows tools. GitHub's Linux runners provide the ARM
compiler and run the engine tests, so WSL and Ubuntu are optional on your PC.

1. Edit maps, scripts, graphics, or configuration in this checkout.
2. Commit your changes and push your branch to `origin`. Automatic CI runs on
   `main`, branches beginning with `codex/`, and pull requests. The prepared
   branch is `codex/emerald-expansion-setup`.
3. Open the repository's
   [CI runs](https://github.com/hkcmoris/romhack/actions/workflows/build.yml)
   and choose the run for your commit and branch.
4. Wait for the entire run, including its aggregate `build` check, to succeed.
   Each ROM is uploaded when its own compilation succeeds, so an artifact may
   appear while tests or other build checks are still running.
5. Under **Artifacts**, download `emerald-release`, extract the ZIP, and open
   `oraculum-v0.2.N-release.gba` in the Windows version of mGBA.

Each successful compile provides one of these artifacts for 14 days:

| Artifact | File | Use |
| --- | --- | --- |
| `emerald-release` | `oraculum-v0.2.N-release.gba` | Normal playtesting with the optimized release build |
| `emerald-development` | `oraculum-v0.2.N.gba` | Testing the unoptimized story build |

`N` is the GitHub Actions CI run number. For example, CI run 12 produces
`oraculum-v0.2.12.gba` and `oraculum-v0.2.12-release.gba`. Both artifacts share
the same version. Each new CI run advances the number across branches and pull
requests; retrying a run retains its version. Failed or cancelled runs can
leave gaps. The version belongs to this ROM hack, independently of the
upstream engine version.

The workflow's `ROM_VERSION` setting in `.github/workflows/build.yml` controls
the `0.2` series; `config.mk` defines the `oraculum` filename prefix. Artifact
names remain `emerald-release` and `emerald-development` for download scripts.
If no version is supplied outside CI, the Makefile uses `0.2.0-local`, which
is a local placeholder rather than an automatically numbered release.
FireRed and LeafGreen checks add `-firered` or `-leafgreen` before the version.

Emulators commonly associate saves with the ROM filename. When upgrading,
back up the save and import it for the new ROM, or copy it to the matching new
filename as your emulator requires. A new build number does not guarantee
save compatibility.

Use the artifact from the commit you intend to test. Sign in to GitHub to
download it. A failed run's job logs identify the compiler or test error;
fix it locally, commit, and push again to start another run.

The GitHub CLI can also download the release artifact from PowerShell:

```powershell
gh run list --repo hkcmoris/romhack --workflow build.yml --branch codex/emerald-expansion-setup
gh run download RUN_ID --repo hkcmoris/romhack --name emerald-release --dir playtest
```

Replace `RUN_ID` with the successful run's ID. Generated `.gba` files are already
ignored by Git. Once the workflow exists on the default branch (`main`), GitHub
also offers **Run workflow** on the CI page for manual runs against a selected
branch. Until then, pushing to a `codex/` branch triggers the workflow directly.

CI retains the engine's Emerald, FireRed, LeafGreen, release, test, and
documentation checks. Artifacts contain compiled ROMs for playtesting, rather
than a GitHub Release or Pages deployment. Upstream documentation deployment
and label workflows remain restricted to the upstream repository.

## Optional local builds with WSL2

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

## Optional local build and validation commands

Run these commands from the repository root in Ubuntu:

```bash
# Strict development build; produces oraculum-v0.2.0-local.gba.
make -j"$(nproc)" -O all UNUSED_ERROR=1 DEPRECATED_ERROR=1

# Optimized release build; produces oraculum-v0.2.0-local-release.gba.
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

## Editing tools and source locations

- [Porymap](https://github.com/huderlem/porymap): open the repository root to
  edit maps, events, warps, and wild encounters. With the GitHub Actions workflow,
  open this Windows checkout. If using WSL2 instead, open the Linux checkout
  through `\\wsl.localhost\Ubuntu\home\<user>\projects\romhack`.
- [Poryscript](https://github.com/huderlem/poryscript): optional higher-level
  event scripting. Its build integration must be configured before using
  `.pory` scripts; the baseline continues to use the included script files.
- [Porytiles](https://github.com/grunt-lucas/porytiles): optional custom tileset
  compilation; follow its integration guide when adding custom map art.
- [mGBA](https://mgba.io/): open `oraculum-v0.2.N.gba` or
  `oraculum-v0.2.N-release.gba` to playtest the corresponding build.
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
required save/data migrations, then push the update branch and wait for all CI
checks to pass before merging. Local builds are optional. Keep updates
incremental as described in the
[upstream installation and migration guide](../INSTALL.md).
