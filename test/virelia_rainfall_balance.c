#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "graphics.h"
#include "pokemon.h"
#include "string_util.h"
#include "virelia_prologue.h"
#include "test/test.h"
#include "test/battle.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/move_relearner.h"
#include "constants/opponents.h"
#include "constants/trainers.h"
#include "constants/virelia_prologue.h"

#if !IS_FRLG && P_VIRELIA_STARTERS

// TESTING uses synthetic trainers; retain the actual authored Chapter 2 teams.
static const struct Trainer sRainfallTrainers[DIFFICULTY_COUNT][TRAINERS_COUNT] =
{
#include "../src/data/trainers.h"
};

TEST("Virelia Rainfall rival teams retain opposite gender and evolved counter starters")
{
    u32 choice = 0, gender = MALE;
    for (u32 c = 0; c < VIRELIA_STARTER_COUNT; c++)
        for (u32 g = MALE; g <= FEMALE; g++)
            PARAMETRIZE { choice = c; gender = g; }
    static const u16 ids[2][3] =
    {
        { TRAINER_VIRELIA_VIKTOR_RAINFALL_GRASS, TRAINER_VIRELIA_VIKTOR_RAINFALL_WATER, TRAINER_VIRELIA_VIKTOR_RAINFALL_FIRE },
        { TRAINER_VIRELIA_VIKTORIE_RAINFALL_GRASS, TRAINER_VIRELIA_VIKTORIE_RAINFALL_WATER, TRAINER_VIRELIA_VIKTORIE_RAINFALL_FIRE },
    };
    static const u16 evolved[] = { SPECIES_ARBOK_VIRELIA, SPECIES_MIGHTYENA_VIRELIA, SPECIES_XATU_VIRELIA };
    u32 previousGender = gSaveBlock2Ptr->playerGender;
    gSaveBlock2Ptr->playerGender = gender;
    const struct Trainer *trainer = &sRainfallTrainers[DIFFICULTY_NORMAL][ids[GetVireliaRivalGender()][choice]];
    EXPECT_EQ((u32)trainer->partySize, 3);
    EXPECT_EQ((u32)trainer->gender, gender == MALE ? FEMALE : MALE);
    EXPECT_EQ((u32)trainer->trainerPic, gender == MALE ? TRAINER_PIC_RS_MAY : TRAINER_PIC_RS_BRENDAN);
    EXPECT_EQ(StringCompare(trainer->trainerName, GetVireliaRivalName()), 0);
    gSaveBlock2Ptr->playerGender = previousGender;
    for (u32 slot = 0; slot < 3; slot++)
    {
        const struct TrainerMon *mon = &trainer->party[slot];
        EXPECT_EQ(mon->lvl, 17 + slot);
        EXPECT_EQ(mon->iv, TRAINER_PARTY_IVS(0, 0, 0, 0, 0, 0));
        EXPECT_EQ(mon->heldItem, ITEM_NONE);
        for (u32 move = 0; move < MAX_MON_MOVES; move++)
            EXPECT_LE(gMovesInfo[mon->moves[move]].power, 65);
    }
    EXPECT_EQ(trainer->party[2].species, evolved[choice]);
}

TEST("Virelia Rainfall ordinary trainers use distinct manageable level sixteen to twenty parties")
{
    u32 trainerId = TRAINER_VIRELIA_RAINFALL_HIKER;
    for (u32 id = TRAINER_VIRELIA_RAINFALL_HIKER; id <= TRAINER_VIRELIA_RAINFALL_WEATHER; id++)
        PARAMETRIZE { trainerId = id; }
    const struct Trainer *trainer = &sRainfallTrainers[DIFFICULTY_NORMAL][trainerId];
    EXPECT_EQ((u32)trainer->partySize, 2);
    EXPECT_NE(trainer->party[0].species, trainer->party[1].species);
    for (u32 slot = 0; slot < 2; slot++)
    {
        const struct TrainerMon *mon = &trainer->party[slot];
        EXPECT_GE(mon->lvl, 16);
        EXPECT_LE(mon->lvl, 20);
        EXPECT_EQ(mon->heldItem, ITEM_NONE);
        for (u32 move = 0; move < MAX_MON_MOVES; move++)
            EXPECT_LE(gMovesInfo[mon->moves[move]].power, 65);
    }
}

TEST("Virelia Rainfall Natu family learns Rain Dance at fourteen while keeping evolution and abilities")
{
    static const u16 species[] = { SPECIES_NATU_VIRELIA, SPECIES_XATU_VIRELIA, SPECIES_NIMBATU };
    for (u32 s = 0; s < ARRAY_COUNT(species); s++)
    {
        bool32 rain = FALSE;
        const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species[s]);
        for (u32 i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
            if (learnset[i].move == MOVE_RAIN_DANCE)
            {
                EXPECT_EQ(learnset[i].level, 14);
                rain = TRUE;
            }
        EXPECT(rain);
        EXPECT_EQ(gSpeciesInfo[species[s]].abilities[0], ABILITY_TORRENT);
        EXPECT_EQ(gSpeciesInfo[species[s]].abilities[2], ABILITY_WATER_ABSORB);
    }
}

// Ordinary early-game IVs, no held items or EVs. All three evolved player
// starters, plus an unevolved level-17 Poochyena, survive one counter-starter
// STAB hit; this is not a full AI run.
SINGLE_BATTLE_TEST("Virelia Rainfall all starter matchups survive one counter hit without one hit wins")
{
    enum Species species = SPECIES_MIGHTYENA_VIRELIA, counter = SPECIES_ARBOK_VIRELIA;
    enum Move move = MOVE_BITE, counterMove = MOVE_EMBER;
    u32 level = 18;
    enum Ability ability = ABILITY_OVERGROW, counterAbility = ABILITY_BLAZE;
    PARAMETRIZE { species = SPECIES_MIGHTYENA_VIRELIA; counter = SPECIES_ARBOK_VIRELIA; move = MOVE_RAZOR_LEAF; counterMove = MOVE_EMBER; ability = ABILITY_OVERGROW; counterAbility = ABILITY_BLAZE; }
    PARAMETRIZE { species = SPECIES_XATU_VIRELIA; counter = SPECIES_MIGHTYENA_VIRELIA; move = MOVE_PSYBEAM; counterMove = MOVE_RAZOR_LEAF; ability = ABILITY_TORRENT; counterAbility = ABILITY_OVERGROW; }
    PARAMETRIZE { species = SPECIES_ARBOK_VIRELIA; counter = SPECIES_XATU_VIRELIA; move = MOVE_ACID_SPRAY; counterMove = MOVE_WATER_PULSE; ability = ABILITY_BLAZE; counterAbility = ABILITY_TORRENT; }
    PARAMETRIZE { species = SPECIES_POOCHYENA_VIRELIA; counter = SPECIES_ARBOK_VIRELIA; move = MOVE_BITE; counterMove = MOVE_EMBER; ability = ABILITY_OVERGROW; counterAbility = ABILITY_BLAZE; level = 17; }
    s16 dealt, received;
    GIVEN {
        PLAYER(species) {
            Nature(NATURE_HARDY); Ability(ability); Moves(move); Level(level);
            HPIV(15); AttackIV(15); DefenseIV(15); SpAttackIV(15); SpDefenseIV(15); SpeedIV(15);
        }
        OPPONENT(counter) {
            Nature(NATURE_HARDY); Ability(counterAbility); Moves(counterMove); Level(19);
            HPIV(0); AttackIV(0); DefenseIV(0); SpAttackIV(0); SpDefenseIV(0); SpeedIV(0);
        }
    } WHEN {
        TURN {
            MOVE(player, move);
            if (counterMove == MOVE_WATER_PULSE)
                MOVE(opponent, counterMove, WITH_RNG(RNG_SECONDARY_EFFECT, FALSE));
            else
                MOVE(opponent, counterMove);
        }
    } SCENE {
        if (species == SPECIES_XATU_VIRELIA || species == SPECIES_POOCHYENA_VIRELIA)
        {
            HP_BAR(player, captureDamage: &received);
            HP_BAR(opponent, captureDamage: &dealt);
        }
        else
        {
            HP_BAR(opponent, captureDamage: &dealt);
            HP_BAR(player, captureDamage: &received);
        }
    } THEN {
        EXPECT_GT(received, 0);
        EXPECT_LT(received, player->maxHP);
        EXPECT_GT(dealt, 0);
        EXPECT_LT(dealt, opponent->maxHP);
        EXPECT_GT(player->hp, 0);
        EXPECT_GT(opponent->hp, 0);
    }
}

SINGLE_BATTLE_TEST("Virelia Rainfall Xatu Rain Dance strengthens its obtainable Water Pulse", s16 damage)
{
    u32 setupMove;
    PARAMETRIZE { setupMove = MOVE_GROWL; }
    PARAMETRIZE { setupMove = MOVE_RAIN_DANCE; }
    GIVEN {
        PLAYER(SPECIES_XATU_VIRELIA) { Level(18); Ability(ABILITY_TORRENT); }
        OPPONENT(SPECIES_MARILL) { Level(20); Ability(ABILITY_THICK_FAT); }
    } WHEN {
        TURN { MOVE(player, setupMove); }
        TURN { MOVE(player, MOVE_WATER_PULSE, WITH_RNG(RNG_SECONDARY_EFFECT, FALSE)); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

#endif
