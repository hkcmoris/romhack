#include "global.h"
#include "test/battle.h"

#if P_VIRELIA_STARTERS

// Exercise the real battle initialization and its ordinary front/back graphics
// loading. This is independent of CreateMonPicSprite and the overworld picbox.
SINGLE_BATTLE_TEST("Virelia starter and evolution graphics load on both sides of a normal battle")
{
    enum Species species = SPECIES_NONE;
    s16 playerDamage, opponentDamage;
    PARAMETRIZE { species = SPECIES_POOCHYENA_VIRELIA; }
    PARAMETRIZE { species = SPECIES_MIGHTYENA_VIRELIA; }
    PARAMETRIZE { species = SPECIES_NATU_VIRELIA; }
    PARAMETRIZE { species = SPECIES_EKANS_VIRELIA; }

    // Peck also reaches the Ghost-type evolution.
    GIVEN {
        PLAYER(species) { Level(5); Speed(10); Moves(MOVE_PECK); }
        OPPONENT(species) { Level(5); Speed(9); Moves(MOVE_PECK); }
    } WHEN {
        TURN { MOVE(player, MOVE_PECK); MOVE(opponent, MOVE_PECK); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PECK, player);
        HP_BAR(opponent, captureDamage: &opponentDamage);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PECK, opponent);
        HP_BAR(player, captureDamage: &playerDamage);
    } THEN {
        EXPECT_GT(playerDamage, 0);
        EXPECT_GT(opponentDamage, 0);
        EXPECT(player->hp > 0);
        EXPECT(opponent->hp > 0);
    }
}

#endif // P_VIRELIA_STARTERS
