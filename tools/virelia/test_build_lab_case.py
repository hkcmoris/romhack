"""Regression checks for palette files normalized by Git on checkout."""
import unittest

from build_lab_case import asset_matches, build


class LabCaseCheckoutTests(unittest.TestCase):
    def test_palette_accepts_lf_and_crlf_checkout(self):
        expected = build()[0]["palettes/12.pal"]
        lf = expected.replace(b"\r\n", b"\n")
        self.assertTrue(asset_matches("palettes/12.pal", lf, expected))
        self.assertTrue(asset_matches("palettes/12.pal", lf.replace(b"\n", b"\r\n"), expected))

    def test_changed_palette_color_is_stale(self):
        expected = build()[0]["palettes/12.pal"]
        changed = expected.replace(b"65 65 82", b"64 65 82", 1)
        self.assertNotEqual(changed, expected)
        self.assertFalse(asset_matches("palettes/12.pal", changed, expected))

    def test_binary_assets_keep_exact_bytes(self):
        for name in ("tiles.png", "metatiles.bin", "metatile_attributes.bin"):
            with self.subTest(name=name):
                self.assertFalse(asset_matches(name, b"\r\n", b"\n"))
                self.assertTrue(asset_matches(name, b"\r\n", b"\r\n"))


if __name__ == "__main__":
    unittest.main()
