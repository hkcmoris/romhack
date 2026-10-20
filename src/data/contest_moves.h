#include "localization.h"
const struct ContestEffect gContestEffects[] =
{
    [CONTEST_EFFECT_HIGHLY_APPEALING] =
    {
        #if C_UPDATED_MOVE_EFFECTS >= GEN_6
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Quite the appealing move.", "Působivé vystoupení.")),
        #else
        .description = COMPOUND_STRING(LOCALIZED_TEXT("A highly appealing move.", "Velmi působivé vystoupení.")),
        #endif
        .effectType = CONTEST_EFFECT_TYPE_APPEAL,
        .appeal = 40,
        .jam = 0,
        .function = ContestEffect_HighlyAppealing,
    },
    [CONTEST_EFFECT_USER_MORE_EASILY_STARTLED] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("After this move, the user is\nmore easily startled.", "Po vystoupení se uživatel snáze nechá\nvyrušit.")),
        .effectType = CONTEST_EFFECT_TYPE_APPEAL,
        .appeal = 60,
        .jam = 0,
        .function = ContestEffect_UserMoreEasilyStartled,
    },
    [CONTEST_EFFECT_GREAT_APPEAL_BUT_NO_MORE_MOVES] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Makes a great appeal, but\nallows no more to the end.", "Skvělé vystoupení. Pak už do konce\nsoutěže nevystoupí.")),
        .effectType = CONTEST_EFFECT_TYPE_APPEAL,
        .appeal = 80,
        .jam = 0,
        .function = ContestEffect_GreatAppealButNoMoreMoves,
    },
    [CONTEST_EFFECT_REPETITION_NOT_BORING] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Can be repeatedly used\nwithout boring the JUDGE.", "Lze opakovat bez znudění poroty.")),
        .effectType = CONTEST_EFFECT_TYPE_APPEAL,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_RepetitionNotBoring,
    },
    [CONTEST_EFFECT_AVOID_STARTLE_ONCE] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Can avoid being startled\nby others once.", "Jednou zabrání vyrušení soupeřem.")),
        .effectType = CONTEST_EFFECT_TYPE_AVOID_STARTLE,
        .appeal = 20,
        .jam = 0,
        .function = ContestEffect_AvoidStartleOnce,
    },
    [CONTEST_EFFECT_AVOID_STARTLE] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Can avoid being startled\nby others.", "Zabrání vyrušení ostatními.")),
        .effectType = CONTEST_EFFECT_TYPE_AVOID_STARTLE,
        .appeal = 10,
        .jam = 0,
        .function = ContestEffect_AvoidStartle,
    },
    [CONTEST_EFFECT_AVOID_STARTLE_SLIGHTLY] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Can avoid being startled\nby others a little.", "Částečně zabrání vyrušení.")),
        .effectType = CONTEST_EFFECT_TYPE_AVOID_STARTLE,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_AvoidStartleSlightly,
    },
    [CONTEST_EFFECT_USER_LESS_EASILY_STARTLED] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("After this move, the user is\nless likely to be startled.", "Uživatel se pak hůře nechá vyrušit.")),
        .effectType = CONTEST_EFFECT_TYPE_AVOID_STARTLE,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_UserLessEasilyStartled,
    },
    [CONTEST_EFFECT_STARTLE_FRONT_MON] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Slightly startles the\nPOKéMON in front.", "Mírně vyruší předchozího Pokémona.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MON,
        .appeal = 30,
        .jam = 20,
        .function = ContestEffect_StartleFrontMon,
    },
    [CONTEST_EFFECT_SLIGHTLY_STARTLE_PREV_MONS] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Slightly startles those\nthat have made appeals.", "Mírně vyruší všechny, kdo už vystoupili.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 30,
        .jam = 10,
        .function = ContestEffect_StartlePrevMons,
    },
    [CONTEST_EFFECT_STARTLE_PREV_MON] =
    {
        #if C_UPDATED_MOVE_EFFECTS >= GEN_6
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Startles the last Pokémon\nto act before the user.", "Vyruší posledního Pokémona před\nuživatelem.")),
        #else
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Startles the POKéMON that\nappealed before the user.", "Vyruší předchozího Pokémona.")),
        #endif
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MON,
        .appeal = 20,
        .jam = 30,
        .function = ContestEffect_StartleFrontMon,
    },
    [CONTEST_EFFECT_STARTLE_PREV_MONS] =
    {
        #if C_UPDATED_MOVE_EFFECTS >= GEN_6
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Startles all of the Pokémon\nto act before the user.", "Vyruší všechny, kdo už vystoupili.")),
        #else
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Startles all POKéMON that\nhave done their appeals.", "Vyruší všechny, kdo už vystoupili.")),
        #endif
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 20,
        .jam = 20,
        .function = ContestEffect_StartlePrevMons,
    },
    [CONTEST_EFFECT_BADLY_STARTLE_FRONT_MON] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Badly startles the\nPOKéMON in front.", "Silně vyruší předchozího Pokémona.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MON,
        .appeal = 10,
        .jam = 40,
        .function = ContestEffect_StartleFrontMon,
    },
    [CONTEST_EFFECT_BADLY_STARTLE_PREV_MONS] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Badly startles those that\nhave made appeals.", "Silně vyruší všechny, kdo už vystoupili.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 10,
        .jam = 30,
        .function = ContestEffect_StartlePrevMons,
    },
    [CONTEST_EFFECT_STARTLE_PREV_MON_2] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Startles the POKéMON that\nappealed before the user.", "Vyruší předchozího Pokémona.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MON,
        .appeal = 30,
        .jam = 20,
        .function = ContestEffect_StartlePrevMon2,
    },
    [CONTEST_EFFECT_STARTLE_PREV_MONS_2] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Startles all POKéMON that\nhave done their appeals.", "Vyruší všechny, kdo už vystoupili.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 30,
        .jam = 10,
        .function = ContestEffect_StartlePrevMons2,
    },
    [CONTEST_EFFECT_SHIFT_JUDGE_ATTENTION] =
    {
        #if C_UPDATED_MOVE_EFFECTS >= GEN_6
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Makes audience expect\nlittle of other contestants.", "Sníží očekávání publika od soupeřů.")),
        #else
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Shifts the JUDGE's\nattention from others.", "Odvede pozornost poroty od soupeřů.")),
        #endif
        .effectType = CONTEST_EFFECT_TYPE_WORSEN,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_ShiftJudgeAttention,
    },
    [CONTEST_EFFECT_STARTLE_MON_WITH_JUDGES_ATTENTION] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Startles the POKéMON that\nhas the JUDGE's attention.", "Vyruší Pokémona, kterého porota\nsleduje.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 20,
        .jam = 10,
        .function = ContestEffect_StartleMonWithJudgesAttention,
    },
    [CONTEST_EFFECT_JAMS_OTHERS_BUT_MISS_ONE_TURN] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Jams the others, and misses\none turn of appeals.", "Vyruší ostatní, ale pak jedno kolo\nvynechá.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 40,
        .jam = 40,
        .function = ContestEffect_JamsOthersButMissOneTurn,
    },
    [CONTEST_EFFECT_STARTLE_MONS_SAME_TYPE_APPEAL] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Startles POKéMON that\nmade a same-type appeal.", "Vyruší Pokémony se stejným typem\nvystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 20,
        .jam = 10,
        .function = ContestEffect_StartleMonsSameTypeAppeal,
    },
    [CONTEST_EFFECT_STARTLE_MONS_COOL_APPEAL] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Badly startles POKéMON\nthat made COOL appeals.", "Silně vyruší stylová vystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 20,
        .jam = 10,
        .function = ContestEffect_StartleMonsCoolAppeal,
    },
    [CONTEST_EFFECT_STARTLE_MONS_BEAUTY_APPEAL] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Badly startles POKéMON\nthat made BEAUTY appeals.", "Silně vyruší krásná vystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 20,
        .jam = 10,
        .function = ContestEffect_StartleMonsBeautyAppeal,
    },
    [CONTEST_EFFECT_STARTLE_MONS_CUTE_APPEAL] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Badly startles POKéMON\nthat made CUTE appeals.", "Silně vyruší roztomilá vystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 20,
        .jam = 10,
        .function = ContestEffect_StartleMonsCuteAppeal,
    },
    [CONTEST_EFFECT_STARTLE_MONS_SMART_APPEAL] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Badly startles POKéMON\nthat made SMART appeals.", "Silně vyruší chytrá vystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 20,
        .jam = 10,
        .function = ContestEffect_StartleMonsSmartAppeal,
    },
    [CONTEST_EFFECT_STARTLE_MONS_TOUGH_APPEAL] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Badly startles POKéMON\nthat made TOUGH appeals.", "Silně vyruší drsná vystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 20,
        .jam = 10,
        .function = ContestEffect_StartleMonsToughAppeal,
    },
    [CONTEST_EFFECT_MAKE_FOLLOWING_MON_NERVOUS] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Makes one POKéMON after\nthe user nervous.", "Znervózní příštího Pokémona.")),
        .effectType = CONTEST_EFFECT_TYPE_WORSEN,
        .appeal = 20,
        .jam = 0,
        .function = ContestEffect_MakeFollowingMonNervous,
    },
    [CONTEST_EFFECT_MAKE_FOLLOWING_MONS_NERVOUS] =
    {
        #if C_UPDATED_MOVE_EFFECTS >= GEN_6
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Makes the remaining\nPokémon nervous.", "Znervózní zbývající Pokémony.")),
        #else
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Makes all POKéMON after\nthe user nervous.", "Znervózní všechny příští Pokémony.")),
        #endif
        .effectType = CONTEST_EFFECT_TYPE_WORSEN,
        .appeal = 20,
        .jam = 0,
        .function = ContestEffect_MakeFollowingMonsNervous,
    },
    [CONTEST_EFFECT_WORSEN_CONDITION_OF_PREV_MONS] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Worsens the condition of\nthose that made appeals.", "Zhorší kondici těch, kdo už vystoupili.")),
        .effectType = CONTEST_EFFECT_TYPE_WORSEN,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_WorsenConditionOfPrevMons,
    },
    [CONTEST_EFFECT_BADLY_STARTLES_MONS_IN_GOOD_CONDITION] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Badly startles POKéMON in\ngood condition.", "Silně vyruší Pokémony v dobré kondici.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 30,
        .jam = 10,
        .function = ContestEffect_BadlyStartlesMonsInGoodCondition,
    },
    [CONTEST_EFFECT_BETTER_IF_FIRST] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("The appeal works great if\nperformed first.", "Výborně působí při prvním vystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 20,
        .jam = 0,
        .function = ContestEffect_BetterIfFirst,
    },
    [CONTEST_EFFECT_BETTER_IF_LAST] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("The appeal works great if\nperformed last.", "Výborně působí při posledním\nvystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 20,
        .jam = 0,
        .function = ContestEffect_BetterIfLast,
    },
    [CONTEST_EFFECT_APPEAL_AS_GOOD_AS_PREV_ONES] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Makes the appeal as good\nas those before it.", "Dojem vyrovná předchozím vystoupením.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 10,
        .jam = 0,
        .function = ContestEffect_AppealAsGoodAsPrevOnes,
    },
    [CONTEST_EFFECT_APPEAL_AS_GOOD_AS_PREV_ONE] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Makes the appeal as good\nas the one before it.", "Dojem vyrovná předchozímu vystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 10,
        .jam = 0,
        .function = ContestEffect_AppealAsGoodAsPrevOne,
    },
    [CONTEST_EFFECT_BETTER_WHEN_LATER] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("The appeal works better\nthe later it is performed.", "Čím později vystoupí, tím lépe působí.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 10,
        .jam = 0,
        .function = ContestEffect_BetterWhenLater,
    },
    [CONTEST_EFFECT_QUALITY_DEPENDS_ON_TIMING] =
    {
        #if C_UPDATED_MOVE_EFFECTS >= GEN_6
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Effectiveness varies\ndepending on when it is used.", "Účinek závisí na pořadí vystoupení.")),
        #else
        .description = COMPOUND_STRING(LOCALIZED_TEXT("The appeal's quality varies\ndepending on its timing.", "Kvalita závisí na pořadí vystoupení.")),
        #endif
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 10,
        .jam = 0,
        .function = ContestEffect_QualityDependsOnTiming,
    },
    [CONTEST_EFFECT_BETTER_IF_SAME_TYPE] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Works well if it's the same\ntype as the one before.", "Dobře působí po vystoupení stejného\ntypu.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 20,
        .jam = 0,
        .function = ContestEffect_BetterIfSameType,
    },
    [CONTEST_EFFECT_BETTER_IF_DIFF_TYPE] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Works well if different in\ntype than the one before.", "Dobře působí po vystoupení jiného\ntypu.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 20,
        .jam = 0,
        .function = ContestEffect_BetterIfDiffType,
    },
    [CONTEST_EFFECT_AFFECTED_BY_PREV_APPEAL] =
    {
        #if C_UPDATED_MOVE_EFFECTS >= GEN_6
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Affected by how well the\nprevious Pokémon's move went.", "Účinek závisí na předchozím vystoupení.")),
        #else
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Affected by how well the\nappeal in front goes.", "Účinek závisí na předchozím vystoupení.")),
        #endif
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_AffectedByPrevAppeal,
    },
    [CONTEST_EFFECT_IMPROVE_CONDITION_PREVENT_NERVOUSNESS] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Ups the user's condition.\nHelps prevent nervousness.", "Zlepší kondici. Pomáhá proti nervozitě.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 10,
        .jam = 0,
        .function = ContestEffect_ImproveConditionPreventNervousness,
    },
    [CONTEST_EFFECT_BETTER_WITH_GOOD_CONDITION] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("The appeal works well if the\nuser's condition is good.", "Dobře působí při dobré kondici\nuživatele.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 10,
        .jam = 0,
        .function = ContestEffect_BetterWithGoodCondition,
    },
    [CONTEST_EFFECT_NEXT_APPEAL_EARLIER] =
    {
        #if C_UPDATED_MOVE_EFFECTS >= GEN_6
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Causes the user to move\nearlier on the next turn.", "Příště se uživatel dostane na řadu\ndříve.")),
        #else
        .description = COMPOUND_STRING(LOCALIZED_TEXT("The next appeal can be\nmade earlier next turn.", "Příště se dostane na řadu dříve.")),
        #endif
        .effectType = CONTEST_EFFECT_TYPE_TURN_ORDER,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_NextAppealEarlier,
    },
    [CONTEST_EFFECT_NEXT_APPEAL_LATER] =
    {
        #if C_UPDATED_MOVE_EFFECTS >= GEN_6
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Causes the user to move\nlater on the next turn.", "Příště se uživatel dostane na řadu\npozději.")),
        #else
        .description = COMPOUND_STRING(LOCALIZED_TEXT("The next appeal can be\nmade later next turn.", "Příště se dostane na řadu později.")),
        #endif
        .effectType = CONTEST_EFFECT_TYPE_TURN_ORDER,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_NextAppealLater,
    },
    [CONTEST_EFFECT_MAKE_SCRAMBLING_TURN_ORDER_EASIER] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Makes the next turn's order\nmore easily scrambled.", "Usnadní změnu pořadí v příštím kole.")),
        .effectType = CONTEST_EFFECT_TYPE_TURN_ORDER,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_MakeScramblingTurnOrderEasier,
    },
    [CONTEST_EFFECT_SCRAMBLE_NEXT_TURN_ORDER] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Scrambles the order of\nappeals on the next turn.", "Zamíchá pořadí vystoupení v příštím\nkole.")),
        .effectType = CONTEST_EFFECT_TYPE_TURN_ORDER,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_ScrambleNextTurnOrder,
    },
    [CONTEST_EFFECT_EXCITE_AUDIENCE_IN_ANY_CONTEST] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("An appeal that excites the\naudience in any CONTEST.", "Nadchne publikum v každé soutěži.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = C_UPDATED_MOVE_EFFECTS >= GEN_6 ? 20 : 10,
        .jam = 0,
        .function = ContestEffect_ExciteAudienceInAnyContest,
    },
    [CONTEST_EFFECT_BADLY_STARTLE_MONS_WITH_GOOD_APPEALS] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Badly startles all POKéMON\nthat made good appeals.", "Silně vyruší všechny po dobrém\nvystoupení.")),
        .effectType = CONTEST_EFFECT_TYPE_STARTLE_MONS,
        .appeal = 20,
        .jam = 10,
        .function = ContestEffect_BadlyStartleMonsWithGoodAppeals,
    },
    [CONTEST_EFFECT_BETTER_WHEN_AUDIENCE_EXCITED] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("The appeal works best the\nmore the crowd is excited.", "Čím nadšenější publikum, tím lepší\ndojem.")),
        .effectType = CONTEST_EFFECT_TYPE_SPECIAL_APPEAL,
        .appeal = 10,
        .jam = 0,
        .function = ContestEffect_BetterWhenAudienceExcited,
    },
    [CONTEST_EFFECT_DONT_EXCITE_AUDIENCE] =
    {
        .description = COMPOUND_STRING(LOCALIZED_TEXT("Temporarily stops the\ncrowd from growing excited.", "Dočasně zabrání dalšímu nadšení\npublika.")),
        .effectType = CONTEST_EFFECT_TYPE_WORSEN,
        .appeal = 30,
        .jam = 0,
        .function = ContestEffect_DontExciteAudience,
    },
};
