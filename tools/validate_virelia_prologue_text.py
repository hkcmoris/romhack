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
    nature = widest("nature_name")
    generic = max([mon, item, move], key=lambda t: text_width(t, "Normal"))
    substitutions = {
        "{PLAYER}": player, "{STR_VAR_1}": generic,
        "{STR_VAR_2}": generic, "{STR_VAR_3}": "999999",
        "{DYNAMIC 0}": "", "{DYNAMIC 1}": "",
        "{DYNAMIC 2}": nature, "{DYNAMIC 3}": "100",
        "{DYNAMIC 4}": "Whisperwood Trail", "{DYNAMIC 5}": "",
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
        fixed_bytes = {"move_name": 17, "ability_name": 17,
                       "type_name": 9, "type_generic": 17,
                       "item_name": 21, "item_plural_name": 23,
                       "nature_name": 13, "dex_category": 13,
                       "trainer_class": 13}
        byte_limit = entry.get("max_bytes", fixed_bytes.get(entry["kind"]))
        if byte_limit and len(visible) + 1 > byte_limit:
            errors.append(f"{label}: {len(visible) + 1} encoded bytes > {byte_limit}")
        if entry.get("max_appended_bytes"):
            # Naming's native StringAppendN copies encoded bytes. This suffix
            # retains its two-byte placeholder even though the printer ignores
            # it; the species name has already been written separately.
            suffix_bytes = len(TOKEN.sub("", text)) + sum(2 for _ in TOKEN.finditer(text)) + 1
            if suffix_bytes > entry["max_appended_bytes"]:
                errors.append(f"{label}: nickname suffix exceeds StringAppendN byte limit")
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
    # These script files contain the playable prologue, including every text
    # label on all five maps. Keep this scan automatic as story text evolves.
    script_paths = sorted((ROOT / "data/maps").glob("Virelia_*/scripts.inc"))
    script_paths += [ROOT / "data/scripts/virelia_prologue.inc"]
    script_count = 0
    dynamic = substitutions
    for path in script_paths:
        source = path.read_text(encoding="utf-8")
        for match in re.finditer(r"(?m)^([A-Za-z0-9_]+)::?\n((?:[ \t]*\.string \"(?:\\.|[^\"\\])*\"\n)+)", source):
            text = "".join(re.findall(r'"((?:\\.|[^"\\])*)"', match[2])).removesuffix("$")
            label = f"{path.relative_to(ROOT)}:{match[1]}"
            script_count += 1
            visible = TOKEN.sub(replace_token, text)
            for line in re.split(r"\\[pnl]", visible):
                unsupported = sorted(set(line) - set(chars))
                if unsupported:
                    errors.append(f"{label}: unmapped glyphs {unsupported!r}")
                elif text_width(line, "Normal") > 208:
                    errors.append(f"{label}: {text_width(line, 'Normal')}px > 208px: {line}")
            lines = 1
            for control in re.findall(r"\\[pnl]", text):
                if control == r"\p":
                    lines = 1
                elif control == r"\l":
                    lines = 2
                else:
                    lines += 1
                    if lines > 2:
                        errors.append(f"{label}: newline below the two-line dialogue window")
    # New game uses three dedicated native strings before naming.
    main_source = (ROOT / "src/main_menu.c").read_text(encoding="utf-8")
    for match in re.finditer(r'sText_Virelia\w+\[\]\s*=\s*_\("((?:\\.|[^"\\])*)"\)', main_source):
        label = "src/main_menu.c:new game"
        visible = TOKEN.sub(replace_token, match[1])
        for line in re.split(r"\\[pnl]", visible):
            if any(c not in chars for c in line) or text_width(line, "Normal") > 208:
                errors.append("src/main_menu.c: new game text exceeds font/window constraints")
    regional = (ROOT / "src/data/pokemon/species_info/virelia_families.h").read_text(encoding="utf-8")
    regional_count = 0
    for match in re.finditer(r'\[(SPECIES_\w+)\]\s*=\s*\{(.*?)\n    \},', regional, re.S):
        regional_count += 1
        for field, limit, max_lines in [("categoryName", 96, 1), ("description", 208, 3)]:
            value = re.search(r'\.' + field + r'\s*=\s*(?:_|COMPOUND_STRING)\(\s*((?:"(?:\\.|[^"\\])*"\s*)+)\)', match[2])
            if not value:
                errors.append(f"{match[1]}: missing regional {field}")
                continue
            text = "".join(re.findall(r'"((?:\\.|[^"\\])*)"', value[1]))
            lines = text.split(r"\n")
            if len(lines) > max_lines or (field == "categoryName" and len(text) > 12):
                errors.append(f"{match[1]}: regional {field} exceeds fixed bounds")
            for line in lines:
                if any(c not in chars for c in line) or text_width(line, "Normal") > limit:
                    errors.append(f"{match[1]}: regional {field} exceeds native Normal {limit}px")
    if regional_count != 9:
        errors.append(f"Expected 9 regional Dex entries, found {regional_count}")
    result = {"passed": not errors, "entries": len(manifest["entries"]), "map_script_texts": script_count, "regional_dex_entries": regional_count, "expanded_cases": dynamic_cases, "max_expanded_width": max(widths, default=0), "level_bound": manifest["level_bound"], "errors": errors}
    if args.json:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    else:
        print(f"Czech prologue text: {'PASS' if not errors else 'FAIL'} ({result['entries']} entries, level bound {result['level_bound']})")
        for error in errors:
            print(error)
    return bool(errors)


if __name__ == "__main__":
    sys.exit(main())
