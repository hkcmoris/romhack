import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import capture_android_crash as capture

PKG = 'com.fastemulator.gba'
FOREIGN = 'com.example.foreign'
FIXTURE = Path(__file__).parent / 'fixtures/android_crash_interleaved.txt'


def record(body, *, pid=900, tid=900, tag='DEBUG', tick=0):
    return f'[ 10-22 10:00:{tick:02d}.000  {pid}: {tid} F/{tag} ]\n{body}\n\n'


def native(owner, token, *, pid=900, tick=0):
    return record('*** *** *** *** *** *** *** ***\n' + f'pid: 2345, tid: 2350, name: engine >>> {owner} <<<\n' + f'backtrace:\n    #00 pc 01234567 /lib/{token}.so', pid=pid, tid=pid, tick=tick)


def saved_text(blocks):
    return '\n'.join(block['text'] for block in blocks)


class FilterTests(unittest.TestCase):
    def test_interleaved_native_keeps_header_registers_and_complete_backtrace(self):
        blocks, counts = capture.filter_crashes(FIXTURE.read_text(encoding='utf-8'), PKG)
        native_blocks = [b for b in blocks if b['kind'] == 'native']
        self.assertEqual(len(native_blocks), 1)
        text = native_blocks[0]['text']
        self.assertIn('*** *** ***', text)
        self.assertIn("ABI: 'arm'", text)
        self.assertIn('r0 00000000', text)
        self.assertIn('#00 pc 00001234', text)
        self.assertIn('#01 pc 00005678', text)
        self.assertIn('900: 900', text)  # debuggerd emitter, not the crashed app PID.
        self.assertNotIn('FOREIGN_', saved_text(blocks))
        self.assertNotIn('GBC_SECRET', saved_text(blocks))
        self.assertEqual(counts['matching_blocks'], 2)

    def test_crash_dump_emitter_tag_with_different_pid_is_retained(self):
        raw = record('*** *** *** *** *** ***\npid: 2345, tid: 2350, name: engine >>> ' + PKG + ' <<<\nbacktrace:\n#00 pc 12345678 libnative.so', pid=921, tid=921, tag='crash_dump32')
        blocks, _ = capture.filter_crashes(raw, PKG)
        self.assertEqual(len(blocks), 1)
        self.assertIn('921: 921', blocks[0]['text'])
        self.assertIn('libnative.so', blocks[0]['text'])

    def test_java_keeps_fatal_header_and_matching_stack(self):
        blocks, _ = capture.filter_crashes(FIXTURE.read_text(encoding='utf-8'), PKG)
        java = [b for b in blocks if b['kind'] == 'java']
        self.assertEqual(len(java), 1)
        self.assertIn('FATAL EXCEPTION: main', java[0]['text'])
        self.assertIn('MYBOY_JAVA_MARKER', java[0]['text'])
        self.assertIn('Emulator.java:42', java[0]['text'])
        self.assertNotIn('FOREIGN_', java[0]['text'])

    def test_device_metadata_is_removed_before_persistence(self):
        blocks, counts = capture.filter_crashes(FIXTURE.read_text(encoding='utf-8'), PKG)
        self.assertNotIn('FAKE_DEVICE_MODEL_SERIAL_SECRET', saved_text(blocks))
        self.assertNotIn('Build fingerprint', saved_text(blocks))
        self.assertEqual(counts['removed_device_metadata_lines'], 1)

    def test_multiple_crashes_with_reused_emitter_do_not_share_ownership(self):
        raw = native(PKG, 'OWN_FIRST', tick=0) + native(FOREIGN, 'FOREIGN_MIDDLE', tick=1) + native(PKG, 'OWN_LAST', tick=2)
        blocks, _ = capture.filter_crashes(raw, PKG)
        self.assertEqual(len(blocks), 2)
        self.assertIn('OWN_FIRST', saved_text(blocks))
        self.assertIn('OWN_LAST', saved_text(blocks))
        self.assertNotIn('FOREIGN_MIDDLE', saved_text(blocks))

    def test_changed_identity_without_separator_ends_previous_group(self):
        raw = native(PKG, 'OWN') + record(f'pid: 8888, tid: 8888, name: other >>> {FOREIGN} <<<\nFOREIGN_NO_SEPARATOR')
        blocks, _ = capture.filter_crashes(raw, PKG)
        self.assertIn('OWN', saved_text(blocks))
        self.assertNotIn('FOREIGN_NO_SEPARATOR', saved_text(blocks))

    def test_package_prefixes_subprocesses_and_gbc_are_excluded(self):
        raw = ''.join(native(pkg, 'SECRET', pid=900+i) for i, pkg in enumerate((PKG + '.foreign', PKG + ':worker', 'com.fastemulator.gbc', 'com.fastemulator.gbafree')))
        self.assertEqual(capture.filter_crashes(raw, PKG)[0], [])

    def test_unowned_mentions_cannot_establish_package_identity(self):
        raw = record(f'Foreign exception mentions "Process: {PKG}, PID: 2345"', tag='AndroidRuntime')
        raw += record(f'Fatal signal in an unrelated app; package string {PKG}', tag='libc')
        self.assertEqual(capture.filter_crashes(raw, PKG)[0], [])

    def test_ambiguous_record_does_not_leak_foreign_trace(self):
        raw = native(PKG, 'OWN')
        raw += record(f'pid: 2345, tid: 2345, name: a >>> {PKG} <<<\npid: 9999, tid: 9999, name: b >>> {FOREIGN} <<<\nFOREIGN_AMBIGUOUS')
        blocks, _ = capture.filter_crashes(raw, PKG)
        self.assertNotIn('FOREIGN_AMBIGUOUS', saved_text(blocks))

    def test_unknown_header_does_not_become_an_owned_body(self):
        raw = native(PKG, 'OWN') + '[ unsupported-format foreign-header ]\nFOREIGN_BAD_HEADER\n'
        blocks, _ = capture.filter_crashes(raw, PKG)
        self.assertNotIn('FOREIGN_BAD_HEADER', saved_text(blocks))

    def test_no_explicit_identity_means_no_saved_block(self):
        raw = record('*** *** *** *** *** ***\nABI: arm\nbacktrace:\n#00 pc deadbeef libunknown.so')
        self.assertEqual(capture.filter_crashes(raw, PKG)[0], [])

    def test_persistence_contains_only_filtered_blocks_and_summary(self):
        blocks, counts = capture.filter_crashes(FIXTURE.read_text(encoding='utf-8'), PKG)
        with tempfile.TemporaryDirectory() as temp:
            output = Path(temp) / 'filtered'
            capture.persist(blocks, {'package': PKG, **counts}, output)
            self.assertEqual({p.name for p in output.iterdir()}, {'myboy-crash-only.log', 'summary.json'})
            written = '\n'.join(p.read_text(encoding='utf-8') for p in output.iterdir())
            for forbidden in ('FOREIGN_', 'GBC_SECRET', 'FAKE_DEVICE_MODEL_SERIAL_SECRET'):
                self.assertNotIn(forbidden, written)
            self.assertIn('MYBOY_JAVA_MARKER', written)
            self.assertIn('#01 pc 00005678', written)


class DiscoveryTests(unittest.TestCase):
    def test_single_device_and_explicit_selection(self):
        self.assertEqual(capture.select_device('List of devices attached\nFAKE_A\tdevice\n', None), 'FAKE_A')
        raw = 'List of devices attached\nFAKE_A\tdevice\nFAKE_B\tdevice\n'
        with self.assertRaises(capture.CaptureError):
            capture.select_device(raw, None)
        self.assertEqual(capture.select_device(raw, 'FAKE_B'), 'FAKE_B')
        for raw in ('List of devices attached\n', 'FAKE_A unauthorized\n', 'FAKE_A offline\n'):
            with self.assertRaises(capture.CaptureError):
                capture.select_device(raw, None)

    def test_package_discovery_never_auto_selects_gbc_or_ambiguous_gba(self):
        self.assertEqual(capture.select_package('package:com.fastemulator.gbc\npackage:' + PKG, None), PKG)
        for raw in ('package:com.fastemulator.gbc\n', 'package:com.fastemulator.gbcfree\n', 'package:' + PKG + '\npackage:com.fastemulator.gbafree\n'):
            with self.assertRaises(capture.CaptureError):
                capture.select_package(raw, None)
        self.assertEqual(capture.select_package('package:' + PKG + '\npackage:com.fastemulator.gbafree', PKG), PKG)

    def test_collect_executes_only_four_read_commands_via_mock_no_adb(self):
        fixture = FIXTURE.read_text(encoding='utf-8')
        replies = ['List of devices attached\nTEST_SERIAL_DO_NOT_PERSIST\tdevice\n', 'package:com.fastemulator.gbc\npackage:' + PKG, fixture, 'Packages:\n  Package [' + PKG + '] (1234):\n    versionCode=99 minSdk=16\n    versionName=2.1.1\n    userId=DO_NOT_PERSIST']
        with patch.object(capture, 'read_adb', side_effect=replies) as mocked:
            blocks, summary = capture.collect('FAKE_ADB')
        self.assertEqual([call.args[1] for call in mocked.call_args_list], [
            ['devices'], ['shell', 'pm', 'list', 'packages', 'com.fastemulator'],
            ['logcat', '-b', 'crash', '-d', '-v', 'long'], ['shell', 'dumpsys', 'package', PKG]])
        self.assertNotIn('TEST_SERIAL_DO_NOT_PERSIST', json.dumps(summary))
        self.assertNotIn('TEST_SERIAL_DO_NOT_PERSIST', saved_text(blocks))
        self.assertEqual(summary['app_version'], {'status': 'read', 'versionName': '2.1.1', 'versionCode': 99})
        self.assertNotIn('DO_NOT_PERSIST', json.dumps(summary))
        self.assertFalse(summary['device_writes'])
        self.assertFalse(summary['log_cleared'])
        self.assertFalse(summary['uploads'])

    def test_version_fields_are_taken_only_from_exact_package_block(self):
        raw = 'Packages:\n Package [com.example.foreign] (111):\n  versionCode=111\n  versionName=FOREIGN_VERSION\n'
        raw += ' Package [' + PKG + '] (222):\n  versionCode=999 minSdk=16 targetSdk=35\n  versionName=2.1.1\n  dataDir=PRIVATE_NOT_SAVED\n'
        raw += ' Package [' + PKG + '.foreign] (333):\n  versionCode=333\n  versionName=FOREIGN_SUFFIX\n'
        result = capture.package_version(raw, PKG)
        self.assertEqual(result, {'status': 'read', 'versionName': '2.1.1', 'versionCode': 999})
        self.assertNotIn('PRIVATE', json.dumps(result))
        self.assertNotIn('FOREIGN', json.dumps(result))

    def test_missing_or_ambiguous_package_version_is_unknown(self):
        raw = ('Package [' + PKG + '] (1):\n versionCode=1\n') * 2
        for text in ('', 'versionCode=1\nversionName=2.1.1', raw):
            self.assertEqual(capture.package_version(text, PKG)['status'], 'unavailable')

    def test_multiple_devices_abort_before_package_or_log_reads(self):
        with patch.object(capture, 'read_adb', return_value='FAKE_A device\nFAKE_B device\n') as mocked:
            with self.assertRaises(capture.CaptureError):
                capture.collect('FAKE_ADB')
        self.assertEqual(mocked.call_count, 1)

    def test_main_without_adb_does_not_run_or_collect(self):
        with patch.object(capture.shutil, 'which', return_value=None), patch.object(capture, 'collect') as mocked, contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(capture.main([]), 2)
        mocked.assert_not_called()


if __name__ == '__main__':
    unittest.main(verbosity=2)
