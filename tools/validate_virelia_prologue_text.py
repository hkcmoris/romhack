#!/usr/bin/env python3
"""Validate the declared Czech prologue text against the native font metrics.

The JSON manifest is the auditable coverage boundary. This check does not assert
that arbitrary grinding or unlisted game content has been localized.
"""
import argparse
import collections
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
TOKEN = re.compile(r"\{[^}]+\}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true", help="print machine readable results")
    args = parser.parse_args()
    manifest = json.loads((ROOT / "tools/virelia_prologue_localization.json").read_text(encoding="utf-8"))
    charmap = (ROOT / "charmap.txt").read_text(encoding="utf-8").split("@ Hiragana")[0]
    chars = {m[1]: int(m[2], 16) for m in re.finditer(r"^'(.)'\s*=\s*([0-9A-F]{2})\b", charmap, re.M)}
    chars["'"] = 0xB4
    fonts = {m[1]: [int(x) for x in re.findall(r"\b\d+\b", m[2])] for m in re.finditer(r"gFont(\w+)LatinGlyphWidths\[\]\s*=\s*\{([^}]+)", (ROOT / "src/fonts.c").read_text(encoding="utf-8"))}
    sources = {}
    errors = []
    widths = []
    dynamic_cases = 0
    def text_width(text, font):
        return sum(fonts[font][chars[c]] for c in text)

    def widest(kind, field="name"):
        values = [e["czech"] for e in manifest["entries"] if e["kind"] == kind and "." + field + "." in e["key"]]
        return max(values, key=lambda t: text_width(t, "Normal"), default="WWWWWWWWWWWW")

    # The nickname limit is 12 in this repository, not vanilla's 10. Use the
    # widest glyph allowed by the name screen, rather than a typical nickname.
    mon = "W" * 12
    player = "W" * 7
    item = widest("item_name")
    move = widest("move_name")
    nature = widest("nature")
    generic = max([mon, item, move], key=lambda t: text_width(t, "Normal"))
    substitutions = {
        "{PLAYER}": player, "{STR_VAR_1}": generic,
        "{STR_VAR_2}": generic, "{STR_VAR_3}": "999999",
        "{DYNAMIC 0}": "", "{DYNAMIC 1}": "",
        "{DYNAMIC 2}": nature, "{DYNAMIC 3}": "100",
        "{DYNAMIC 4}": "Maplebrook", "{DYNAMIC 5}": "",
        "{PKMN}": "PKMN", "{POKEBLOCK}": "POKéBLOCK",
        "{LV_2}": "úr.", "{A_BUTTON}": "WW",
        "{B_BUTTON}": "WW", "{START_BUTTON}": "WW",
        "{DPAD_UPDOWN}": "WW", "{DPAD_NONE}": "WW",
    }
    for entry in manifest["entries"]:
        path, key, text = entry["path"], entry["key"], entry["czech"]
        source = sources.setdefault(path, (ROOT / path).read_text(encoding="utf-8"))
        label = f"{path}:{key}"
        if not entry.get("conditional"):
            pair = 'LOCALIZED_TEXT("' + entry["english"] + '", "' + text + '")'
            if pair not in source:
                errors.append(f"{label}: manifest/source mismatch")
        elif text not in source or "#if TESTING" not in source:
            errors.append(f"{label}: missing conditional translation")
        if not entry.get("conditional") and collections.Counter(TOKEN.findall(entry["english"])) != collections.Counter(TOKEN.findall(text)):
            errors.append(f"{label}: changed placeholders/control tokens")
        dynamic = {**substitutions, **entry.get("replacements", {})}
        if TOKEN.search(text):
            dynamic_cases += 1
        def replace_token(match):
            token = match[0]
            if token in dynamic:
                return dynamic[token]
            if token.startswith(("{COLOR ", "{SHADOW ", "{BACKGROUND ", "{HIGHLIGHT ", "{PAUSE ", "{PLAY_SE ")) or token == "{PAUSE_UNTIL_PRESS}":
                return ""
            errors.append(f"{label}: unbound token {token}")
            return ""
        visible = TOKEN.sub(replace_token, text)
        visible = re.sub(r"\\[pnl]", "\n", visible)
        visible = visible.replace(r'\"', '"').replace(r"\\", "\\")
        for line in visible.split("\n"):
            unsupported = sorted(set(line) - set(chars))
            if unsupported:
                errors.append(f"{label}: unmapped glyphs {unsupported!r}")
                continue
            width = sum(fonts[entry["font"]][chars[c]] for c in line)
            widths.append(width)
            if width > entry["width"]:
                errors.append(f"{label}: {width}px > {entry['width']}px: {line}")
        if entry.get("lines") and len(visible.split("\n")) > entry["lines"]:
            errors.append(f"{label}: too many lines")
    result = {"passed": not errors, "entries": len(manifest["entries"]), "expanded_cases": dynamic_cases, "max_expanded_width": max(widths, default=0), "level_bound": manifest["level_bound"], "errors": errors}
    if args.json:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    else:
        print(f"Czech prologue text: {'PASS' if not errors else 'FAIL'} ({result['entries']} entries, level bound {result['level_bound']})")
        for error in errors:
            print(error)
    return bool(errors)


if __name__ == "__main__":
    sys.exit(main())
