#include "global.h"
#include "test/battle.h"

#if !IS_FRLG && P_VIRELIA_STARTERS

// Deterministic mechanics checks use ordinary level-16 starter stages and
// legal moves. They measure damage; they do not substitute for a full AI run.
SINGLE_BATTLE_TEST("Virelia first Gym starter stages survive one Rock Tomb without one hit victories")
{
    enum Species species = SPECIES_POOCHYENA_VIRELIA;
    enum Move move = MOVE_RAZOR_LEAF;
    enum Ability ability = ABILITY_OVERGROW;
    PARAMETRIZE { species = SPECIES_POOCHYENA_VIRELIA; move = MOVE_RAZOR_LEAF; ability = ABILITY_OVERGROW; }
    PARAMETRIZE { species = SPECIES_ARIADOS_VIRELIA; move = MOVE_AQUA_JET; ability = ABILITY_TORRENT; }
    PARAMETRIZE { species = SPECIES_ARBOK_VIRELIA; move = MOVE_ACID_SPRAY; ability = ABILITY_BLAZE; }
    s16 dealt, received;
    GIVEN {
        PLAYER(species) {
            Nature(NATURE_HARDY); Ability(ability); Moves(move);
            HPIV(15); AttackIV(15); DefenseIV(15); SpAttackIV(15); SpDefenseIV(15); SpeedIV(15);
            Level(16);
        }
        OPPONENT(SPECIES_NOSEPASS) {
            Nature(NATURE_HARDY); Ability(ABILITY_STURDY); Moves(MOVE_ROCK_TOMB);
            HPIV(0); AttackIV(0); DefenseIV(0); SpAttackIV(0); SpDefenseIV(0); SpeedIV(0);
            Level(16);
        }
    } WHEN {
        TURN { MOVE(player, move); MOVE(opponent, MOVE_ROCK_TOMB); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &dealt);
        HP_BAR(player, captureDamage: &received);
    } THEN {
        EXPECT_GT(dealt, 0);
        EXPECT_LE(dealt, opponent->maxHP * 2 / 3);
        EXPECT_GT(received, 0);
        EXPECT_LT(received, player->maxHP);
        EXPECT_GT(player->hp, 0);
        EXPECT_GT(opponent->hp, 0);
        if (move == MOVE_ACID_SPRAY)
            EXPECT_EQ(opponent->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE - 2);
    }
}

// The harness defaults to the lowest damage roll (85 percent).
SINGLE_BATTLE_TEST("Virelia quarry Makuhita at level fifteen can defeat the leader ace with obtainable Vital Throw")
{
    GIVEN {
        PLAYER(SPECIES_MAKUHITA) {
            Nature(NATURE_HARDY); Ability(ABILITY_THICK_FAT); Moves(MOVE_VITAL_THROW);
            HPIV(15); AttackIV(15); DefenseIV(15); SpAttackIV(15); SpDefenseIV(15); SpeedIV(15);
            Level(15);
        }
        OPPONENT(SPECIES_NOSEPASS) {
            Nature(NATURE_HARDY); Ability(ABILITY_STURDY); Moves(MOVE_ROCK_TOMB);
            HPIV(0); AttackIV(0); DefenseIV(0); SpAttackIV(0); SpDefenseIV(0); SpeedIV(0);
            Level(16);
        }
    } WHEN {
        TURN { MOVE(player, MOVE_VITAL_THROW); MOVE(opponent, MOVE_ROCK_TOMB); }
        TURN { MOVE(player, MOVE_VITAL_THROW); MOVE(opponent, MOVE_ROCK_TOMB); }
        TURN { MOVE(player, MOVE_VITAL_THROW); MOVE(opponent, MOVE_ROCK_TOMB); }
    } SCENE {
        HP_BAR(player);
        HP_BAR(opponent);
        HP_BAR(player);
        HP_BAR(opponent);
        HP_BAR(player);
        HP_BAR(opponent);
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
        EXPECT_GT(player->hp, 0);
    }
}

#endif
