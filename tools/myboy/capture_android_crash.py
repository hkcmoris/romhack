#!/usr/bin/env python3
"""Read the Android crash buffer into RAM and persist only exact-package crash blocks.

No adb command runs during import. No log clearing, device writes, root, downloads,
full-system log dumps, or uploads are implemented.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
from datetime import datetime, timezone
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import threading

MAX_BYTES = 8 * 1024 * 1024
MAX_BLOCK_BYTES = 1024 * 1024
PACKAGES = ('com.fastemulator.gba', 'com.fastemulator.gbafree')
SOURCES = (
    'https://developer.android.com/tools/logcat',
    'https://developer.android.com/tools/adb',
    'https://source.android.com/docs/core/tests/debug/native-crash',
    'https://developer.android.com/tools/dumpsys',
)
HEADER = re.compile(r'^\[\s*\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+(\d+)\s*:\s*(\d+)\s+([VDIWEFSA])/([^\]]+?)\s*\]\s*$')
NATIVE_ID = re.compile(r'^\s*pid:\s*\d+\s*,\s*tid:\s*\d+.*?>>>\s*([^\s<>]+)\s*<<<\s*$')
JAVA_ID = re.compile(r'^\s*Process:\s*([^\s,]+)\s*,\s*PID:\s*\d+\s*$')
NATIVE_START = re.compile(r'^\s*\*\*\*(?:\s+\*\*\*){3,}\s*$')
JAVA_START = re.compile(r'^\s*FATAL EXCEPTION:')
PRIVATE_METADATA = re.compile(r'^\s*(?:Build fingerprint|Revision|Hardware(?: name)?|Device|Model|Serial(?: number)?|Bootloader|Radio|Kernel)\s*:', re.I)


class CaptureError(ValueError):
    pass


@dataclass(frozen=True)
class Record:
    header: str
    pid: int
    tid: int
    priority: str
    tag: str
    body: tuple[str, ...]

    @property
    def emitter(self):
        return self.pid, self.tid, self.tag


def parse_long(text: str) -> list[Record]:
    """Reject unknown headers rather than merging their bodies into an owned record."""
    if len(text.encode('utf-8')) > MAX_BYTES:
        raise CaptureError('Crash buffer exceeds the local 8 MiB limit; nothing was saved.')
    records = []
    current = None
    body = []

    def flush():
        nonlocal current, body
        if current is not None:
            header, match = current
            records.append(Record(header, int(match[1]), int(match[2]), match[3], match[4].strip(), tuple(body)))
        current, body = None, []

    for line in text.splitlines():
        match = HEADER.fullmatch(line)
        if match:
            flush()
            current = line, match
        elif line.lstrip().startswith('[') and line.rstrip().endswith(']'):
            flush()
        elif line.startswith('--------- beginning of '):
            flush()
        elif current is not None:
            body.append(line)
    flush()
    return records


def _identities(record: Record, kind: str):
    matcher = NATIVE_ID if kind == 'native' else JAVA_ID
    return {match[1] for line in record.body if (match := matcher.fullmatch(line))}


def filter_crashes(text: str, package: str) -> tuple[list[dict], dict]:
    """Group by the actual log emitter, never the app PID alone.

    Native ownership requires a debuggerd-style pid/tid >>> exact.package <<<
    identity. Java ownership requires a Process: exact.package, PID: identity.
    Names with suffixes, subprocess suffixes, or quoted mentions do not match.
    """
    if package not in PACKAGES:
        raise CaptureError('Only the two known My Boy! GBA package identifiers are accepted.')
    groups, active = [], {}

    def finish(key):
        group = active.pop(key, None)
        if group is not None:
            groups.append(group)

    for record in parse_long(text):
        kind = 'native' if record.tag in {'DEBUG', 'debuggerd', 'crash_dump32', 'crash_dump64'} else 'java' if record.tag == 'AndroidRuntime' else None
        if kind is None:
            continue
        key = record.emitter
        start = any((NATIVE_START if kind == 'native' else JAVA_START).match(line) for line in record.body)
        identities = _identities(record, kind)
        if len(identities) > 1:
            # Ambiguous multi-process records cannot grant or retain ownership.
            finish(key)
            continue
        owner = next(iter(identities)) if identities else None
        # A new separator or explicit identity for another process ends ownership.
        if start or (owner is not None and key in active and active[key]['owner'] is not None and active[key]['owner'] != owner):
            finish(key)
        if key not in active:
            if not start and owner is None:
                continue
            active[key] = {'kind': kind, 'owner': owner, 'records': []}
        group = active[key]
        if owner is not None:
            if group['owner'] is not None and group['owner'] != owner:
                # Defensive guard if the grouping rules above are changed later.
                finish(key)
                active[key] = group = {'kind': kind, 'owner': owner, 'records': []}
            group['owner'] = owner
        group['records'].append(record)
    for key in list(active):
        finish(key)

    selected = []
    redacted = 0
    for group in groups:
        if group['owner'] != package:
            continue
        formatted, count = [], 0
        for record in group['records']:
            body = []
            for line in record.body:
                if PRIVATE_METADATA.match(line):
                    redacted += 1
                else:
                    body.append(line)
            if not any(line.strip() for line in body):
                continue
            formatted.append(record.header + '\n' + '\n'.join(body).rstrip() + '\n')
            count += 1
        block = '\n'.join(formatted).rstrip() + '\n'
        if len(block.encode('utf-8')) > MAX_BLOCK_BYTES:
            raise CaptureError('A matching crash exceeds 1 MiB; no partial backtrace was saved.')
        selected.append({'kind': group['kind'], 'package': package, 'record_count': count, 'text': block})
    return selected, {'matching_blocks': len(selected), 'matching_records': sum(b['record_count'] for b in selected), 'removed_device_metadata_lines': redacted}


def select_device(text: str, serial: str | None) -> str:
    devices = []
    for line in text.splitlines():
        parts = line.split()
        if len(parts) >= 2 and not line.startswith(('List of devices', '*')):
            devices.append((parts[0], parts[1]))
    if serial is not None:
        matches = [entry for entry in devices if entry[0] == serial]
        if len(matches) != 1 or matches[0][1] != 'device':
            raise CaptureError('The explicitly selected device is not connected and authorized. Check the USB/RSA prompt.')
        return serial
    if len(devices) != 1:
        raise CaptureError('Exactly one connected device is required; otherwise select one explicitly with --serial.')
    if devices[0][1] != 'device':
        raise CaptureError('The connected device is not authorized or ready. Check the USB/RSA prompt.')
    return devices[0][0]


def select_package(text: str, requested: str | None) -> str:
    installed = {line.removeprefix('package:').strip() for line in text.splitlines() if line.startswith('package:')}
    candidates = sorted(installed.intersection(PACKAGES))
    if requested is not None:
        if requested not in PACKAGES or requested not in candidates:
            raise CaptureError('The explicitly selected My Boy! GBA package was not found.')
        return requested
    if len(candidates) == 0:
        raise CaptureError('No supported My Boy! GBA package was found; My OldBoy!/GBC packages are excluded.')
    if len(candidates) != 1:
        raise CaptureError('Both My Boy! GBA editions are installed. Select one explicitly with --package.')
    return candidates[0]



def package_version(text: str, package: str) -> dict:
    """Extract only two version fields from the explicitly selected package block."""
    current, versions = None, []
    for line in text.splitlines():
        header = re.match(r'\s*Package \[([^\]]+)\]\s*\(', line)
        if header:
            if current is not None:
                versions.append(current)
            current = {} if header[1] == package else None
            continue
        if current is None:
            continue
        code = re.fullmatch(r'\s*versionCode=(\d+)(?:\s+.*)?', line)
        name = re.fullmatch(r'\s*versionName=([0-9A-Za-z][0-9A-Za-z._+ -]{0,79})\s*', line)
        if code:
            current['versionCode'] = int(code[1])
        if name and name[1].strip() != 'null':
            current['versionName'] = name[1].strip()
    if current is not None:
        versions.append(current)
    nonempty = [v for v in versions if v]
    if len(nonempty) != 1:
        return {'status': 'unavailable', 'versionName': None, 'versionCode': None}
    return {'status': 'read', 'versionName': nonempty[0].get('versionName'), 'versionCode': nonempty[0].get('versionCode')}


def read_adb(adb: str, args: list[str], *, serial: str | None = None, timeout: int = 20) -> str:
    """Bounded memory-only command output; command errors never echo device identifiers."""
    command = [adb] + (['-s', serial] if serial is not None else []) + args
    try:
        process = subprocess.Popen(command, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=False)
    except OSError as exc:
        raise CaptureError('ADB could not be started. Supply an existing Platform Tools adb executable with --adb.') from exc
    timed_out = threading.Event()

    def stop():
        timed_out.set()
        try:
            process.kill()
        except OSError:
            pass

    timer = threading.Timer(timeout, stop)
    timer.daemon = True
    chunks, total = [], 0
    timer.start()
    try:
        while chunk := process.stdout.read(65536):
            total += len(chunk)
            if total > MAX_BYTES:
                process.kill()
                raise CaptureError('ADB output exceeded 8 MiB; no raw output was saved.')
            chunks.append(chunk)
        status = process.wait(timeout=2)
    finally:
        timer.cancel()
        process.stdout.close()
        if process.poll() is None:
            process.kill()
            process.wait(timeout=2)
    if timed_out.is_set():
        raise CaptureError('ADB did not finish within the local timeout; no raw output was saved.')
    if status != 0:
        raise CaptureError('ADB could not complete this read-only request. Check USB debugging and authorization; raw errors were not saved.')
    return b''.join(chunks).decode('utf-8', errors='replace')


def collect(adb: str, *, serial: str | None = None, package: str | None = None):
    selected_serial = select_device(read_adb(adb, ['devices']), serial)
    package_list = read_adb(adb, ['shell', 'pm', 'list', 'packages', 'com.fastemulator'], serial=selected_serial)
    selected_package = select_package(package_list, package)
    # The crash_dump/debuggerd emitter has a different PID from the crashed app.
    raw = read_adb(adb, ['logcat', '-b', 'crash', '-d', '-v', 'long'], serial=selected_serial)
    blocks, counts = filter_crashes(raw, selected_package)
    try:
        version_dump = read_adb(adb, ['shell', 'dumpsys', 'package', selected_package], serial=selected_serial)
        version = package_version(version_dump, selected_package)
    except (CaptureError, OSError, subprocess.TimeoutExpired):
        version = {'status': 'unavailable', 'versionName': None, 'versionCode': None}
    # No device serial/model and no raw system log or full package dump leave here.
    return blocks, {'package': selected_package, 'app_version': version, **counts, 'source_buffer': 'crash', 'device_writes': False, 'log_cleared': False, 'uploads': False}


def persist(blocks: list[dict], summary: dict, output: Path):
    output.mkdir(parents=True, exist_ok=False)
    document = '\n'.join(f'=== My Boy! crash block {i}: {block["kind"]} ===\n{block["text"]}' for i, block in enumerate(blocks, 1))
    if blocks:
        (output / 'myboy-crash-only.log').write_text(document, encoding='utf-8')
    result = {**summary, 'collected_at_utc': datetime.now(timezone.utc).isoformat(), 'sources': SOURCES}
    (output / 'summary.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--adb', default='adb', help='An existing Android SDK Platform Tools adb executable; nothing is downloaded.')
    ap.add_argument('--serial', help='Explicit device selector, never written into output.')
    ap.add_argument('--package', choices=PACKAGES, help='Required when both My Boy! GBA editions are installed.')
    ap.add_argument('--output', type=Path, default=Path('outputs/myboy') / ('android-crash-' + datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')))
    args = ap.parse_args(argv)
    adb = shutil.which(args.adb)
    if adb is None:
        print('ADB is not installed or not on PATH. Supply an existing adb executable with --adb; nothing was collected.', file=sys.stderr)
        return 2
    try:
        blocks, summary = collect(adb, serial=args.serial, package=args.package)
        persist(blocks, summary, args.output)
    except (CaptureError, FileExistsError) as exc:
        print(str(exc) if isinstance(exc, CaptureError) else 'The output directory already exists. Choose a new --output directory.', file=sys.stderr)
        return 2
    except (OSError, subprocess.TimeoutExpired):
        print('The local read or output operation failed; no raw device output is shown.', file=sys.stderr)
        return 2
    print(json.dumps({'matching_blocks': summary['matching_blocks'], 'matching_records': summary['matching_records'], 'raw_system_log_saved': False, 'device_identifiers_saved': False}, ensure_ascii=False))
    if not blocks:
        print('No exact-package crash was found in the crash buffer. This does not prove that the app did not crash.', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
