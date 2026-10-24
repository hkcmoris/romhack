"""Synthetic format tests only; these do not emulate My Boy or reproduce a crash."""

import gzip
import struct
import unittest
import zlib

from decode_state import (BUFFER_SIZE, CAPACITY, DiagnosticDecodeError, EVENT,
                          HEADER, HEADER_FIELDS, MAGIC, MAX_BYTES, decode_state)


def make_buffer(*, sequences=(1,), next_sequence=2, count=None, **overrides):
    header = dict.fromkeys(HEADER_FIELDS, 0)
    header.update(magic0=0x4456424D, magic1=0x31474149, version=1,
                  mode=1, capacity=CAPACITY, eventSize=EVENT.size,
                  nextSequence=next_sequence,
                  count=len(sequences) if count is None else count)
    header.update(overrides)
    result = bytearray(BUFFER_SIZE)
    HEADER.pack_into(result, 0, *(header[field] for field in HEADER_FIELDS))
    for sequence in sequences:
        slot = (sequence - 1) & (CAPACITY - 1)
        EVENT.pack_into(result, HEADER.size + slot * EVENT.size,
                        sequence, sequence, 3, 0x083BFC98,
                        0x80000019, 0x4C00)
    return bytes(result)


class DiagnosticDecoderTests(unittest.TestCase):
    def test_raw_state_offsets_do_not_imply_ram_addresses(self):
        result = decode_state(b"wrapper" + make_buffer())
        self.assertEqual(result["container"], "raw")
        ring = result["buffers"][0]
        self.assertEqual(ring["decoded_offset"], 7)
        self.assertNotIn("ewram_address", ring)
        event = ring["events"][0]
        self.assertTrue(event["effects_lookahead"])
        self.assertEqual(event["opcode"], 0x19)
        self.assertEqual((event["map_group"], event["map_number"]), (76, 0))

    def test_raw_ewram_addresses_require_explicit_size_checked_option(self):
        memory = bytearray(0x40000)
        memory[0x120:0x120 + BUFFER_SIZE] = make_buffer()
        ring = decode_state(bytes(memory), ewram_raw=True)["buffers"][0]
        self.assertEqual(ring["ewram_address"], 0x02000120)
        with self.assertRaises(DiagnosticDecodeError):
            decode_state(make_buffer(), ewram_raw=True)
        with self.assertRaises(DiagnosticDecodeError):
            decode_state(gzip.compress(bytes(memory)), ewram_raw=True)

    def test_complete_gzip_and_zlib_containers(self):
        raw = b"prefix" + make_buffer() + b"suffix"
        for name, packed in (("gzip", gzip.compress(raw)), ("zlib", zlib.compress(raw))):
            with self.subTest(name=name):
                result = decode_state(packed)
                self.assertEqual(result["container"], name)
                self.assertEqual(result["decoded_size"], len(raw))
                self.assertEqual(result["buffers"][0]["decoded_offset"], 6)

    def test_unknown_container_and_corrupt_or_incomplete_compression(self):
        for data in (b"unknown-state-format", gzip.compress(make_buffer())[:-4],
                     zlib.compress(make_buffer())[:-1],
                     zlib.compress(make_buffer())[:-1] + b"\x00",
                     gzip.compress(make_buffer()) + b"extra"):
            with self.subTest(data=data[:10]):
                with self.assertRaises(DiagnosticDecodeError):
                    decode_state(data)

    def test_decompression_is_bounded(self):
        with self.assertRaisesRegex(DiagnosticDecodeError, "exceeds"):
            decode_state(zlib.compress(b"\x00" * (MAX_BYTES + 1)))
        with self.assertRaisesRegex(DiagnosticDecodeError, "exceeds"):
            decode_state(b"\x00" * (MAX_BYTES + 1))

    def test_reject_invalid_abi_and_truncated_buffers(self):
        for overrides in ({"version": 2}, {"mode": 0}, {"mode": 3},
                          {"capacity": 65}, {"eventSize": 20}, {"count": 65}):
            with self.subTest(overrides=overrides):
                with self.assertRaises(DiagnosticDecodeError):
                    decode_state(make_buffer(**overrides))
        with self.assertRaises(DiagnosticDecodeError):
            decode_state(make_buffer()[:-1])

    def test_full_ring_order_and_sticky_header(self):
        sequences = tuple(range(97, 161))
        raw = make_buffer(sequences=sequences, next_sequence=161,
                          faultCode=3, faultArg0=0x23682368,
                          lastScript=0x083BFC98, lastSp=0x03007FF0)
        ring = decode_state(raw)["buffers"][0]
        self.assertEqual([event["sequence"] for event in ring["events"]], list(sequences))
        self.assertEqual(ring["header"]["faultArg0"], 0x23682368)
        self.assertEqual(ring["header"]["lastSp"], 0x03007FF0)

    def test_sequence_wrap_skips_reserved_zero_and_rejects_stale_slot(self):
        # At wrap, the C writer skips sequence zero and physical slot 63.
        raw = make_buffer(sequences=tuple(range(0xFFFFFFC0, 0x100000000)) + (1,),
                          next_sequence=2, count=64)
        ring = decode_state(raw)["buffers"][0]
        sequences = [event["sequence"] for event in ring["events"]]
        self.assertEqual(sequences, list(range(0xFFFFFFC2, 0x100000000)) + [1])
        self.assertEqual(len(sequences), 63)
        self.assertEqual(ring["rejected_events"][0]["sequence"], 0xFFFFFFC0)
        ring = decode_state(make_buffer(sequences=(0xFFFFFFFE, 0xFFFFFFFF),
                                        next_sequence=0))["buffers"][0]
        self.assertEqual([event["sequence"] for event in ring["events"]],
                         [0xFFFFFFFE, 0xFFFFFFFF])

    def test_incomplete_entry_is_ignored_and_header_pending_commit_retained(self):
        raw = bytearray(make_buffer(sequences=(1, 2, 3), next_sequence=4))
        struct.pack_into("<I", raw, HEADER.size + 2 * EVENT.size, 0)
        ring = decode_state(bytes(raw))["buffers"][0]
        self.assertEqual([event["sequence"] for event in ring["events"]], [1, 2])
        ring = decode_state(make_buffer(sequences=(1, 2), next_sequence=2, count=1))["buffers"][0]
        self.assertEqual([event["sequence"] for event in ring["events"]], [1, 2])
        self.assertTrue(ring["events"][-1]["header_pending"])

    def test_false_magic_does_not_hide_valid_ring_and_multiple_rings_are_reported(self):
        bad = bytearray(make_buffer(version=20))
        raw = bytes(bad) + make_buffer() + make_buffer(mode=2)
        result = decode_state(raw)
        self.assertEqual(len(result["buffers"]), 2)
        self.assertEqual(result["rejected_candidates"][0]["decoded_offset"], 0)
        self.assertEqual(result["buffers"][0]["decoded_offset"], BUFFER_SIZE)
        self.assertEqual(MAGIC, make_buffer()[:8])

    def test_corrupt_event_slot_is_reported_without_inventing_order(self):
        raw = bytearray(make_buffer())
        struct.pack_into("<I", raw, HEADER.size, 2)
        ring = decode_state(bytes(raw))["buffers"][0]
        self.assertEqual(ring["events"], [])
        self.assertIn("physical slot", ring["rejected_events"][0]["reason"])


if __name__ == "__main__":
    unittest.main()
