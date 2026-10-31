"""Derive bounded Virelia localization coverage from actual species data."""
from pathlib import Path
import re
import json
ROOT = Path(__file__).resolve().parents[1]
BASE_SPECIES = {'WURMPLE','ZIGZAGOON','POOCHYENA','SEEDOT','TAILLOW','SHROOMISH','RALTS','RATTATA','ZUBAT','POOCHYENA_VIRELIA','NATU_VIRELIA','EKANS_VIRELIA','GEODUDE','ROGGENROLA','SANDSHREW','MAKUHITA','ARON','NOSEPASS'}
CHAPTER_MOVES = {'TACKLE','SAND_ATTACK','HARDEN','HEADBUTT','ROCK_THROW','MUD_SLAP','DEFENSE_CURL','METAL_CLAW','LOW_KICK','FOCUS_ENERGY','ROCK_TOMB','THUNDER_WAVE','BLOCK'}
CHAPTER_ITEMS = {'POTION','SUPER_POTION','POKE_BALL','ANTIDOTE','PARALYZE_HEAL','ESCAPE_ROPE','REPEL','TM_ROCK_TOMB','GREAT_BALL','NET_BALL','MYSTIC_WATER'}
def records():
    result = {}
    for path in sorted((ROOT / 'src/data/pokemon/species_info').glob('*_families.h')):
        for match in re.finditer(r'^    \[(SPECIES_\w+)\]\s*=\s*\{(.*?)^    \},', path.read_text(encoding='utf-8'), re.M | re.S):
            result[match[1]] = (path.relative_to(ROOT).as_posix(), match[2])
    return result

def derive_coverage(level_bound=24):
    species_records = records()
    learnsets = {}
    for path in sorted((ROOT / 'src/data/pokemon/level_up_learnsets').glob('*.h')):
        for match in re.finditer(r'struct LevelUpMove (\w+)\[\]\s*=\s*\{(.*?)\n\};', path.read_text(encoding='utf-8'), re.S):
            learnsets.setdefault(match[1], set()).update((int(level), move) for level, move in re.findall(r'LEVEL_UP_MOVE\(\s*(\d+),\s*(MOVE_\w+)\)', match[2]))
    species = {'SPECIES_' + name for name in BASE_SPECIES}
    wild = json.loads((ROOT / 'src/data/wild_encounters.json').read_text(encoding='utf8'))
    for group in wild['wild_encounter_groups']:
        for encounter in group['encounters']:
            if not encounter.get('map', '').startswith('MAP_VIRELIA_'):
                continue
            for field in ('land_mons', 'water_mons', 'fishing_mons', 'rock_smash_mons'):
                species.update(mon['species'] for mon in encounter.get(field, {}).get('mons', [])
                               if mon['species'] != 'SPECIES_NONE')
    moves = {'MOVE_' + name for name in CHAPTER_MOVES} | {'MOVE_STRUGGLE'}
    abilities = set()
    items = {'ITEM_' + name for name in CHAPTER_ITEMS}
    trainers = (ROOT / 'src/data/trainers.party').read_text(encoding='utf8')
    for trainer in re.finditer(r'^=== TRAINER_VIRELIA_\w+ ===\n(.*?)(?=^=== |\Z)', trainers, re.M | re.S):
        for name in re.findall(r'^([^:\n]+)\nLevel:', trainer[1], re.M):
            species.add(name if name.startswith('SPECIES_') else 'SPECIES_' + re.sub(r'[^A-Za-z0-9]+', '_', name).upper())
        moves.update('MOVE_' + re.sub(r'[^A-Za-z0-9]+', '_', name).upper() for name in re.findall(r'^- (.+)', trainer[1], re.M))
    scripts = list((ROOT / 'data/maps').glob('Virelia_*/scripts.inc'))
    scripts += list((ROOT / 'data/scripts').glob('virelia*.inc'))
    for path in scripts:
        items.update(re.findall(r'\bITEM_[A-Z0-9_]+\b', path.read_text(encoding='utf8')))
    for path in (ROOT / 'data/maps').glob('Virelia_*/map.json'):
        entry = json.loads(path.read_text(encoding='utf8'))
        items.update(event['item'] for event in entry['bg_events'] if event['type'] == 'hidden_item')
    for name in ('virelia_first_gym.c', 'virelia_rainfall.c'):
        items.update(re.findall(r'\bITEM_[A-Z0-9_]+\b', (ROOT / 'src' / name).read_text(encoding='utf8')))
    if 'ITEM_HM_CUT' in items:
        moves.add('MOVE_CUT')
    # Pickup grants Moon/Sun Stones at levels 11-20; include supported stone evolutions.
    pickup = (ROOT / 'src/battle_script_commands.c').read_text(encoding='utf-8')
    table = re.search(r'sPickupTable\[\]\s*=\s*\{(.*?)\n\};', pickup, re.S)[1]
    for item, weights in re.findall(r'\{\s*(ITEM_\w+),\s*\{([^}]+)\}', table):
        if any(value.strip() not in ('', '_', '0') for value in weights.split(',')[:(level_bound - 1) // 10 + 1]):
            items.add(item)
    pending = list(species)
    while pending:
        name = pending.pop()
        if name not in species_records:
            raise ValueError(f'Missing species record: {name}')
        _, block = species_records[name]
        for field in re.findall(r'\.abilities\s*=\s*\{([^}]+)\}', block):
            abilities.update(re.findall(r'ABILITY_\w+', field))
        items.update(re.findall(r'\.item(?:Common|Rare)\s*=\s*(ITEM_\w+)', block))
        for learnset in re.findall(r'\.levelUpLearnset\s*=\s*(\w+)', block):
            if learnset not in learnsets:
                raise ValueError(f'Missing learnset: {learnset}')
            moves.update(move for level, move in learnsets[learnset] if level <= level_bound)
        for method, parameter, target, tail in re.findall(r'\{(EVO_\w+),\s*(\w+),\s*(SPECIES_\w+)([^}]*?)(?=\})', block):
            reachable = method.startswith('EVO_LEVEL') and parameter.isdigit() and int(parameter) <= level_bound
            reachable |= method == 'EVO_ITEM' and parameter in items
            if 'IF_IN_MAPSEC' in tail:
                reachable = False
            if reachable and target not in species:
                species.add(target)
                pending.append(target)
    abilities.discard('ABILITY_NONE')
    items.discard('ITEM_NONE')
    return {'species': sorted(species), 'moves': sorted(moves), 'abilities': sorted(abilities), 'items': sorted(items)}

if __name__ == '__main__':
    import json
    print(json.dumps(derive_coverage(), indent=2))
