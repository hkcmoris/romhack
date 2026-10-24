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
from virelia_localization_coverage import derive_coverage

ROOT = Path(__file__).resolve().parents[1]
TOKEN = re.compile(r"\{[^}]+\}")


def validate_move_description_layout(fonts, chars, controller_source=None):
    """Check the assembled stat row, whose CLEAR_TO fields do not auto-wrap."""
    if controller_source is None:
        controller_source = (ROOT / "src/battle_controller_player.c").read_text(encoding="utf-8")
    controller = re.search(r"static void MoveSelectionDisplayMoveDescription\(enum BattlerId battler\)\n\{(.*?)\n\}", controller_source, re.S)
    errors = []
    result = {"cases": 0}
    if not controller:
        return result, ["Move description: missing native row renderer"]
    controller = controller[1]

    def c_string(name):
        matches = re.findall(r"\bu8\s+" + name + r"\[\d*\]\s*=\s*_\((.*?)\);", controller, re.S)
        if not matches:
            raise ValueError(f"missing {name} initializer")
        value = matches[-1]
        localized = re.fullmatch(r'\s*LOCALIZED_TEXT\(\s*"((?:\\.|[^"\\])*)"\s*,\s*"((?:\\.|[^"\\])*)"\s*\)\s*', value, re.S)
        return localized[2] if localized else "".join(re.findall(r'"((?:\\.|[^"\\])*)"', value))

    try:
        window_source = (ROOT / "src/battle_bg.c").read_text(encoding="utf-8")
        text_source = (ROOT / "src/battle_message.c").read_text(encoding="utf-8")
        windows = re.findall(r"\[B_WIN_MOVE_DESCRIPTION\]\s*=\s*\{(.*?)\n    \}", window_source, re.S)
        printers = re.findall(r"\[B_WIN_MOVE_DESCRIPTION\]\s*=\s*\{(.*?)\n    \}", text_source, re.S)
        geometry = {(int(re.search(r"\.tilemapLeft\s*=\s*(\d+)", block)[1]) * 8,
                     int(re.search(r"\.width\s*=\s*(\d+)", block)[1]) * 8) for block in windows}
        printer_geometry = {(re.search(r"\.fontId\s*=\s*FONT_(\w+)", block)[1],
                             int(re.search(r"\.x\s*=\s*(\d+)", block)[1]),
                             int(re.search(r"\.letterSpacing\s*=\s*(\d+)", block)[1])) for block in printers}
        if len(geometry) != 1 or len(printer_geometry) != 1:
            raise ValueError("window/printer variants need individual layout checks")
        window_x, window_width = next(iter(geometry))
        font_id, text_x, spacing = next(iter(printer_geometry))
        font = "".join(word.title() for word in font_id.split("_"))
        starts = {name: int(re.fullmatch(r"\{CLEAR_TO (\d+)\}", c_string(name + "_start"))[1]) + text_x
                  for name in ("cat", "pwr", "acc")}
        labels = {name: c_string(name + "_desc") for name in starts}
        for name in ("pwr", "acc"):
            size = re.search(r"\b" + name + r"_num\[(\d+)\]", controller)
            if not size or int(size[1]) < 4:
                errors.append(f"Move description: {name} buffer needs three digits and EOS")
        summary = (ROOT / "src/pokemon_summary_screen.c").read_text(encoding="utf-8")
        icon_oam = re.search(r"sOamData_CategoryIcons\s*=\s*\{(.*?)\n\};", summary, re.S)[1]
        icon_width = int(re.search(r"SPRITE_SIZE\((\d+)x\d+\)", icon_oam)[1])
        icon_center = int(re.search(r"CreateSprite\(&gSpriteTemplate_CategoryIcons,\s*(\d+),", controller)[1]) - window_x
        icon_left, icon_right = icon_center - icon_width // 2, icon_center + icon_width // 2

        def width(text, font_name):
            return sum(fonts[font_name][chars[c]] + spacing for c in text)

        # The sprite uses absolute screen coordinates, whereas CLEAR_TO is
        # relative to the window. Ignore trailing space when checking its ink.
        category_end = starts["cat"] + width(labels["cat"].rstrip(), font)
        if not (0 <= starts["cat"] <= category_end <= icon_left < icon_right <= starts["pwr"] <= window_width):
            errors.append("Move description: category label/icon overlaps a column or window edge")
        max_right = {}
        # Native Narrow is authoritative; retain a Normal-font safety margin
        # for the new Czech columns, without widening the legacy icon label.
        for checked_font in dict.fromkeys((font, "Normal")):
            max_right[checked_font] = 0
            for power in ("100", "999", "---"):
                for accuracy in ("100", "---"):
                    result["cases"] += 1
                    power_end = starts["pwr"] + width(labels["pwr"] + power, checked_font)
                    accuracy_end = starts["acc"] + width(labels["acc"] + accuracy, checked_font)
                    max_right[checked_font] = max(max_right[checked_font], accuracy_end)
                    if power_end + 2 > starts["acc"]:
                        errors.append(f"Move description ({checked_font}): power {power} leaves less than 2px before accuracy")
                    if accuracy_end > window_width:
                        errors.append(f"Move description ({checked_font}): accuracy {accuracy} ends at {accuracy_end}px > {window_width}px")
        result.update({"window_width": window_width, "native_font": font,
                       "columns": starts, "category_icon": [icon_left, icon_right],
                       "max_right": max_right})
    except (AttributeError, IndexError, KeyError, TypeError, ValueError) as error:
        errors.append(f"Move description: cannot validate native geometry: {error}")
    return result, errors


def validate_english_names(source, macro, label, allowed_identifiers=()):
    """Keep shared display names independent of the selected game language."""
    # Ignore C comments without stripping comment-like text inside a string.
    source = re.sub(r'"(?:\\.|[^"\\])*"|/\*.*?\*/|//[^\n]*',
                    lambda m: m[0] if m[0].startswith('"') else ' ' + '\n' * m[0].count('\n'),
                    source, flags=re.S)
    names = list(re.finditer(r'^[ \t]*\.name\s*=\s*', source, re.M))
    expression = re.escape(macro) + r'\(\s*(?:"(?:\\.|[^"\\])*"\s*)+\)'
    alternatives = [expression, *(re.escape(name) for name in allowed_identifiers)]
    literal = re.compile(r'(?:' + '|'.join(alternatives) + r')\s*,')
    errors = [f"{label} names must remain English literals (line {source.count(chr(10), 0, name.start()) + 1})"
              for name in names if not literal.match(source, name.end())]
    if not names:
        errors.append(f"{label} names: missing shared name table")
    return len(names), errors


def validate_english_move_names(source):
    return validate_english_names(source, "COMPOUND_STRING", "Move")


def validate_english_item_names(source):
    return validate_english_names(source, "ITEM_NAME", "Item", ("gQuestionMarksItemName",))


def validate_coverage(manifest):
    """Reject gaps when wild data, teams, pickups or learnsets change."""
    errors = []
    if manifest["level_bound"] < 20:
        errors.append("First Gym localization must cover at least level 20")
    coverage = derive_coverage(manifest["level_bound"])
    if manifest.get("coverage") != coverage:
        errors.append("Localization coverage differs from actual Virelia data; update the manifest and translations")
    by_kind = collections.defaultdict(set)
    for entry in manifest["entries"]:
        by_kind[entry["kind"]].add(entry["key"].split(".")[0])
    for group, kinds in (("moves", ("move_name", "move_description")),
                         ("abilities", ("ability_name", "ability_description")),
                         ("items", ("item_name", "item_description")),
                         ("species", ("dex_category", "dex_description"))):
        for record in coverage[group]:
            # Regional records use direct Czech strings and have their own
            # native-font checks below; every other record needs a manifest.
            if group == "species" and record.endswith("_VIRELIA"):
                continue
            for kind in kinds:
                if record not in by_kind[kind]:
                    errors.append(f"{record}: missing {kind} in bounded localization coverage")
    # Direct calls to shared messages must have an explicit translation entry.
    # Map-local text scanning alone cannot detect English shared shop greetings.
    declared_labels = {entry["key"] for entry in manifest["entries"]}
    for path in sorted((ROOT / "data/maps").glob("Virelia_*/scripts.inc")):
        for label in re.findall(r"\b(?:message|msgbox)\s+(gText_\w+)", path.read_text(encoding="utf8")):
            if label not in declared_labels:
                errors.append(f"{path.relative_to(ROOT)}:{label}: shared message missing from localization manifest")
    return {key: len(values) for key, values in coverage.items()}, errors


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
        "{PLAYER}": player, "{RIVAL}": "VIKTORIE", "{STR_VAR_1}": generic,
        "{STR_VAR_2}": generic, "{STR_VAR_3}": "999999",
        "{DYNAMIC 0}": "", "{DYNAMIC 1}": "",
        "{DYNAMIC 2}": nature, "{DYNAMIC 3}": "100",
        "{DYNAMIC 4}": "Whisperwood Trail", "{DYNAMIC 5}": "",
        "{PKMN}": "PKMN", "{POKEBLOCK}": "POKéBLOCK",
        "{LV_2}": "úr.", "{A_BUTTON}": "WW",
        "{B_BUTTON}": "WW", "{START_BUTTON}": "WW",
        "{DPAD_UPDOWN}": "WW", "{DPAD_NONE}": "WW",
    }
    item_base_names = {e["key"].split(".")[0]: e["english"] for e in manifest["entries"] if e["kind"] == "item_name"}
    for entry in manifest["entries"]:
        path, key, text = entry["path"], entry["key"], entry["czech"]
        source = sources.setdefault(path, (ROOT / path).read_text(encoding="utf-8"))
        label = f"{path}:{key}"
        if entry["kind"] in ("move_name", "item_name"):
            # Match the named record so another entry cannot mask a mismatch.
            record_id = entry["key"].split(".")[0]
            block = re.search(r"^    \[" + re.escape(record_id) + r"\]\s*=\s*\{(.*?)^    \},", source, re.M | re.S)
            macro = "COMPOUND_STRING" if entry["kind"] == "move_name" else "ITEM_NAME"
            name = re.compile(r'\.name\s*=\s*' + macro + r'\("' + re.escape(entry["english"]) + r'"\)')
            if text != entry["english"] or not block or not name.search(block[1]):
                errors.append(f"{label}: name must match its English name")
        elif entry["kind"] == "item_plural_name":
            # Czech quantity messages already supply Nx, so use the base name.
            item_id = entry["key"].split(".")[0]
            block = re.search(r"^    \[" + re.escape(item_id) + r"\]\s*=\s*\{(.*?)^    \},", source, re.M | re.S)
            pair = 'LOCALIZED_TEXT("' + entry["english"] + '", "' + text + '")'
            plural = re.compile(r'\.pluralName\s*=\s*ITEM_PLURAL_NAME\(\s*' + re.escape(pair) + r'\s*\)')
            if text != item_base_names.get(item_id) or not block or not plural.search(block[1]):
                errors.append(f"{label}: quantity display must preserve its English base name")
        elif not entry.get("conditional"):
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
    # Scan every Virelia map, including service interiors, quarry and Gym.
    # New maps inherit the same glyph, width and dialogue-page validation.
    script_paths = sorted((ROOT / "data/maps").glob("Virelia_*/scripts.inc"))
    script_paths += sorted((ROOT / "data/scripts").glob("virelia*.inc"))
    script_count = 0
    dynamic = substitutions
    for path in script_paths:
        source = path.read_text(encoding="utf-8")
        for match in re.finditer(r"(?m)^([A-Za-z0-9_]+)::?\n((?:[ \t]*\.string \"(?:\\.|[^\"\\])*\"\n)+)", source):
            text = "".join(re.findall(r'"((?:\\.|[^"\\])*)"', match[2])).removesuffix("$")
            label = f"{path.relative_to(ROOT)}:{match[1]}"
            script_count += 1
            dynamic = dict(substitutions)
            if match[1] == "VireliaStarterTest_Text_Given":
                dynamic["{STR_VAR_2}"] = "100"
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
    # The custom case uses a 24-tile prompt and a full-width type label.
    case_source = (ROOT / "src/starter_choose.c").read_text(encoding="utf-8")
    case_texts = re.findall(r'static const u8 (sVirelia\w+)\[\] = _\("((?:\\.|[^"\\])*)"\);', case_source)
    if len(case_texts) != 5:
        errors.append("Briefcase: expected prompt, confirmation and three type labels")
    for name, value in case_texts:
        font, limit = ("Narrow", 240) if name.endswith("Type") else ("Normal", 192)
        for line in value.split(r"\n"):
            if any(c not in chars for c in line) or text_width(line, font) > limit:
                errors.append(f"Briefcase {name}: text exceeds {font} {limit}px window")
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
    move_names, name_errors = validate_english_move_names(sources["src/data/moves_info.h"])
    errors.extend(name_errors)
    item_names, item_name_errors = validate_english_item_names(sources["src/data/items.h"])
    errors.extend(item_name_errors)
    move_layout, layout_errors = validate_move_description_layout(fonts, chars)
    errors.extend(layout_errors)
    coverage_counts, coverage_errors = validate_coverage(manifest)
    errors.extend(coverage_errors)
    result = {"passed": not errors, "entries": len(manifest["entries"]), "map_script_texts": script_count, "regional_dex_entries": regional_count, "expanded_cases": dynamic_cases, "max_expanded_width": max(widths, default=0), "level_bound": manifest["level_bound"], "bounded_coverage": coverage_counts, "move_description_layout": move_layout, "english_move_names": move_names, "english_item_names": item_names, "errors": errors}
    if args.json:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    else:
        print(f"Czech prologue text: {'PASS' if not errors else 'FAIL'} ({result['entries']} entries, level bound {result['level_bound']})")
        for error in errors:
            print(error)
    return bool(errors)


if __name__ == "__main__":
    sys.exit(main())
