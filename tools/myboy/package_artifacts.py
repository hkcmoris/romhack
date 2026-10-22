#!/usr/bin/env python3
"""Package one exact optimized diagnostic ROM with its matching evidence."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess


def command(*args):
    return subprocess.check_output(args, text=True).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--variant", required=True, choices=("control", "event-only"))
    parser.add_argument("--rom", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    mode = {"control": 1, "event-only": 2}[args.variant]
    if f"-myboy-{args.variant}-" not in args.rom.name or not args.rom.name.endswith("-release.gba"):
        parser.error("ROM filename must identify the requested optimized diagnostic variant")
    args.output.mkdir(parents=True, exist_ok=True)
    files = {}
    for extension in (".gba", ".elf", ".map", ".sym"):
        source = args.rom.with_suffix(extension)
        content = source.read_bytes()
        files[source.name] = {"bytes": len(content), "sha256": hashlib.sha256(content).hexdigest()}
        shutil.copy2(source, args.output / source.name)
    for name in ("decode_state.py", "README_ROLLBACK_CS.txt"):
        source = Path(__file__).parent / name
        content = source.read_bytes()
        files[name] = {"bytes": len(content), "sha256": hashlib.sha256(content).hexdigest()}
        shutil.copy2(source, args.output / name)
    manifest = {
        "source_commit": command("git", "rev-parse", "HEAD"),
        "variant": args.variant,
        "diagnostic_mode": mode,
        "configuration": {
            "GAME_VERSION": "EMERALD",
            "GAME_LANGUAGE": os.environ.get("GAME_LANGUAGE", "ENGLISH"),
            "ROM_VERSION": os.environ.get("ROM_VERSION"),
            "VIRELIA_DEVELOPMENT_MODE": 0,
            "RELEASE": 1,
            "USE_LTO_ON_RELEASE": 1,
            "MYBOY_DIAGNOSTIC_MODE": mode,
        },
        "build_command": f"make -j$(nproc) release syms MYBOY_DIAGNOSTIC_MODE={mode}",
        "toolchain": {
            "gcc": command("arm-none-eabi-gcc", "--version").splitlines()[0],
            "ld": command("arm-none-eabi-ld", "--version").splitlines()[0],
        },
        "callback_address_policy": "raw tagged ROM callbacks, matching 0.3.58; no primary-ROM normalization",
        "preserved_fixes": ["checked picture allocations and cleanup", "graphics decompression from ROM", "existing runtime diagnostics"],
        "hypothesis": (
            "Determine whether the known failure requires creation of laboratory starter pictures; "
            "compare against the control built at the same source revision with the same diagnostics."
            if mode == 2 else
            "Test the narrow rollback of callback address normalization to 0.3.58 behavior, with diagnostics and original pictures intact. Other fixes are preserved; Android menu recovery is not guaranteed."
        ),
        "expected_behavior": (
            "Original Czech descriptions and YES/NO selection, no laboratory starter picture; original rival battle."
            if mode == 2 else
            "Original starter pictures, Czech selection, rival movement and battle."
        ),
        "android_result": "NOT TESTED: requires the affected My Boy! Android device",
        "workflow_run": os.environ.get("GITHUB_RUN_ID"),
        "files": files,
    }
    (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    checksums = "".join(f"{entry['sha256']}  {name}\n" for name, entry in files.items())
    (args.output / "SHA256SUMS").write_text(checksums, encoding="ascii")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()
