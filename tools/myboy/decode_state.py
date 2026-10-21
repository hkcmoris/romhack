#!/usr/bin/env python3
"""Recover the opt-in My Boy diagnostic ring; this is not a CPU trace parser.

Accepts raw RAM/full state images and complete gzip or zlib streams. The state
container is deliberately not assigned a RAM address without --ewram-raw.
"""

import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys
import zlib


MAGIC = b"MBVDIAG1"
MAX_BYTES = 16 * 1024 * 1024
CAPACITY = 64
HEADER_FIELDS = (
    "magic0", "magic1", "version", "mode", "capacity", "eventSize",
    "nextSequence", "count", "faultCode", "faultArg0", "faultArg1",
    "lastScript", "lastOpcode", "lastCallback", "lastPreview",
    "lastBattleFlags", "lastSp", "lowestSp", "faultIme", "faultIe",
    "faultIf", "faultWaitcnt",
)
HEADER = struct.Struct("<22I")
EVENT = struct.Struct("<6I")
BUFFER_SIZE = HEADER.size + CAPACITY * EVENT.size
EVENT_NAMES = {
    1: "BOOT", 2: "SCRIPT_START", 3: "OPCODE", 4: "NATIVE",
    5: "CALLBACK", 6: "PREVIEW_REQUEST", 7: "PREVIEW_CREATED",
    8: "PREVIEW_FAILED", 9: "PREVIEW_CLOSED", 10: "PREVIEW_SKIPPED",
    11: "BATTLE_CREATE", 12: "BATTLE_TASK", 13: "FAULT", 14: "RESOURCE",
    15: "OPCODE_RETURN", 16: "SPECIAL",
}


class DiagnosticDecodeError(ValueError):
    """The input is unsupported, incomplete, or has no valid diagnostic ABI."""


def _unpack_container(data):
    if len(data) > MAX_BYTES:
        raise DiagnosticDecodeError("Input exceeds the 16 MiB limit")
    if data.startswith(b"\x1f\x8b"):
        container, window = "gzip", zlib.MAX_WBITS + 16
    elif (len(data) >= 2 and data[0] & 15 == 8 and data[0] >> 4 <= 7
          and int.from_bytes(data[:2], "big") % 31 == 0):
        container, window = "zlib", zlib.MAX_WBITS
    else:
        return data, "raw"
    stream = zlib.decompressobj(window)
    try:
        decoded = stream.decompress(data, MAX_BYTES + 1)
    except zlib.error as error:
        raise DiagnosticDecodeError(f"Invalid {container} stream: {error}") from error
    if len(decoded) > MAX_BYTES or stream.unconsumed_tail:
        raise DiagnosticDecodeError("Decompressed input exceeds the 16 MiB limit")
    if not stream.eof:
        raise DiagnosticDecodeError(f"Incomplete {container} stream")
    if stream.unused_data:
        raise DiagnosticDecodeError(f"Unsupported trailing data after {container} stream")
    return decoded, container


def _decode_buffer(data, offset, ewram_raw):
    if len(data) - offset < BUFFER_SIZE:
        raise DiagnosticDecodeError("Truncated diagnostic buffer")
    header = dict(zip(HEADER_FIELDS, HEADER.unpack_from(data, offset)))
    if header["version"] != 1:
        raise DiagnosticDecodeError("Unsupported diagnostic version")
    if header["mode"] not in (1, 2):
        raise DiagnosticDecodeError("Invalid diagnostic build mode")
    if header["capacity"] != CAPACITY or header["eventSize"] != EVENT.size:
        raise DiagnosticDecodeError("Invalid ring capacity or event size")
    if header["count"] > CAPACITY:
        raise DiagnosticDecodeError("Invalid ring count")
    events, rejected_events = [], []
    next_sequence = header["nextSequence"] or 1
    for slot in range(CAPACITY):
        words = EVENT.unpack_from(data, offset + HEADER.size + slot * EVENT.size)
        sequence, frame, kind, arg0, arg1, map_id = words
        if sequence == 0:
            continue  # Unused or being written: never manufacture its contents.
        # Zero is reserved as the incomplete-entry marker, including at wrap.
        age = (next_sequence - sequence) % 0xFFFFFFFF
        if (sequence - 1) & (CAPACITY - 1) != slot:
            rejected_events.append({"slot": slot, "sequence": sequence,
                                    "reason": "sequence does not match physical slot"})
            continue
        if age > header["count"]:
            rejected_events.append({"slot": slot, "sequence": sequence,
                                    "reason": "stale or inconsistent sequence"})
            continue
        event = dict(zip(("sequence", "frame", "kind", "arg0", "arg1", "map"), words))
        event.update(slot=slot, kind_name=EVENT_NAMES.get(kind, "UNKNOWN"),
                     age=age, header_pending=(age == 0))
        event["map_group"] = None if map_id == 0xFFFFFFFF else (map_id >> 8) & 255
        event["map_number"] = None if map_id == 0xFFFFFFFF else map_id & 255
        if kind == 3:
            event["effects_lookahead"] = bool(arg1 & 0x80000000)
            event["opcode"] = arg1 & 0x7FFFFFFF
        events.append(event)
    events.sort(key=lambda event: event["age"], reverse=True)
    result = {"decoded_offset": offset, "header": header, "events": events,
              "rejected_events": rejected_events,
              "note": "Logged checkpoints only; no instruction history or proven cause is inferred."}
    if ewram_raw:
        result["ewram_address"] = 0x02000000 + offset
    return result


def decode_state(data, *, ewram_raw=False):
    """Decode all valid rings, retaining evidence of rejected magic matches."""
    decoded, container = _unpack_container(data)
    if ewram_raw and (container != "raw" or len(decoded) != 0x40000):
        raise DiagnosticDecodeError("--ewram-raw requires exactly 256 KiB of uncompressed EWRAM")
    buffers, rejected_candidates = [], []
    start = 0
    while True:
        offset = decoded.find(MAGIC, start)
        if offset < 0:
            break
        start = offset + len(MAGIC)
        try:
            buffers.append(_decode_buffer(decoded, offset, ewram_raw))
        except DiagnosticDecodeError as error:
            rejected_candidates.append({"decoded_offset": offset, "reason": str(error)})
    if not buffers:
        detail = "; ".join(item["reason"] for item in rejected_candidates[:3])
        raise DiagnosticDecodeError("No valid diagnostic buffer found" + (f": {detail}" if detail else ""))
    return {"container": container, "input_sha256": hashlib.sha256(data).hexdigest(),
            "decoded_sha256": hashlib.sha256(decoded).hexdigest(),
            "decoded_size": len(decoded), "buffers": buffers,
            "rejected_candidates": rejected_candidates}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("state", type=Path, help="A copied emulator state or raw memory image")
    parser.add_argument("--ewram-raw", action="store_true",
                        help="Assert input is a raw 256 KiB EWRAM image; report RAM addresses")
    args = parser.parse_args(argv)
    try:
        if args.state.stat().st_size > MAX_BYTES:
            raise DiagnosticDecodeError("Input exceeds the 16 MiB limit")
        result = decode_state(args.state.read_bytes(), ewram_raw=args.ewram_raw)
    except (DiagnosticDecodeError, OSError) as error:
        print(f"Cannot decode diagnostics: {error}", file=sys.stderr)
        return 2
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
