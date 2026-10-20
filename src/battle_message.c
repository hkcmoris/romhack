#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_ai_record.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_setup.h"
#include "battle_special.h"
#include "battle_z_move.h"
#include "data.h"
#include "event_data.h"
#include "frontier_util.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item.h"
#include "link.h"
#include "menu.h"
#include "palette.h"
#include "recorded_battle.h"
#include "string_util.h"
#include "strings.h"
#include "test_runner.h"
#include "text.h"
#include "trainer_hill.h"
#include "trainer_slide.h"
#include "trainer_tower.h"
#include "window.h"
#include "line_break.h"
#include "constants/abilities.h"
#include "constants/battle_dome.h"
#include "constants/battle_string_ids.h"
#include "constants/frontier_util.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "constants/trainer_hill.h"
#include "constants/weather.h"

// Upstream gameplay tests compare canonical English MESSAGE fixtures.
// Select before charmap preprocessing so normal/release ROMs contain only Czech.
#if TESTING
#define BATTLE_TEXT(english, czech) english
#else
#define BATTLE_TEXT(english, czech) czech
#endif

struct BattleWindowText
{
    u8 fillValue;
    u8 fontId;
    u8 x;
    u8 y;
    union {
        struct {
            DEPRECATED("Use color.background instead") u8 bgColor;
            DEPRECATED("Use color.foreground instead") u8 fgColor;
            DEPRECATED("Use color.shadow instead") u8 shadowColor;
            DEPRECATED("Use color.accent instead") u8 accentColor;
        };
        union TextColor color;
    };
    u8 letterSpacing;
    u8 lineSpacing;
    u8 speed;
};

#if TESTING
EWRAM_DATA u16 sBattlerAbilities[MAX_BATTLERS_COUNT] = {0};
#else
static EWRAM_DATA u16 sBattlerAbilities[MAX_BATTLERS_COUNT] = {0};
#endif
EWRAM_DATA struct BattleMsgData *gBattleMsgDataPtr = NULL;

// todo: make some of those names less vague: attacker/target vs pkmn, etc.

static const u8 sText_EmptyString4[] = _("");

const u8 gText_PkmnShroudedInMist[] = _(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} surrounds itself with a protective mist!", "{B_ATK_NAME_WITH_PREFIX} se zahalil do ochranné mlhy!"));
const u8 gText_PkmnGettingPumped[] = _(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} is getting pumped!", "{B_DEF_NAME_WITH_PREFIX} se soustředí!"));
const u8 gText_PkmnsXPreventsSwitching[] = _(BATTLE_TEXT("{B_BUFF1} is preventing switching out with its {B_LAST_ABILITY} Ability!\p", "{B_BUFF1} brání výměně díky schopnosti {B_LAST_ABILITY}!\p"));
const u8 gText_StatSharply[] = _(BATTLE_TEXT(" sharply", " výrazně"));
const u8 gText_StatRose[] = _(BATTLE_TEXT("rose!", "vzrostla!"));
const u8 gText_StatFell[] = _(BATTLE_TEXT("fell!", "klesla!"));
const u8 gText_DefendersStatRose[] = _(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1} rose{B_BUFF2}!", "{B_DEF_NAME_WITH_PREFIX}: statistika {B_BUFF1} vzrostla{B_BUFF2}!"));
static const u8 sText_GotAwaySafely[] = _(BATTLE_TEXT("{PLAY_SE SE_FLEE}You got away safely!\p", "{PLAY_SE SE_FLEE}Podařilo se ti utéct!\p"));
static const u8 sText_PlayerDefeatedLinkTrainer[] = _(BATTLE_TEXT("You defeated {B_LINK_OPPONENT1_NAME}!", "Porážíš soupeře {B_LINK_OPPONENT1_NAME}!"));
static const u8 sText_TwoLinkTrainersDefeated[] = _(BATTLE_TEXT("You defeated {B_LINK_OPPONENT1_NAME} and {B_LINK_OPPONENT2_NAME}!", "Porážíš soupeře {B_LINK_OPPONENT1_NAME} a {B_LINK_OPPONENT2_NAME}!"));
static const u8 sText_PlayerLostAgainstLinkTrainer[] = _(BATTLE_TEXT("You lost against {B_LINK_OPPONENT1_NAME}!", "Prohráváš proti soupeři {B_LINK_OPPONENT1_NAME}!"));
static const u8 sText_PlayerLostToTwo[] = _(BATTLE_TEXT("You lost to {B_LINK_OPPONENT1_NAME} and {B_LINK_OPPONENT2_NAME}!", "Prohráváš proti soupeřům {B_LINK_OPPONENT1_NAME} a {B_LINK_OPPONENT2_NAME}!"));
static const u8 sText_PlayerBattledToDrawLinkTrainer[] = _(BATTLE_TEXT("You battled to a draw against {B_LINK_OPPONENT1_NAME}!", "Remíza se soupeřem {B_LINK_OPPONENT1_NAME}!"));
static const u8 sText_PlayerBattledToDrawVsTwo[] = _(BATTLE_TEXT("You battled to a draw against {B_LINK_OPPONENT1_NAME} and {B_LINK_OPPONENT2_NAME}!", "Remíza se soupeři {B_LINK_OPPONENT1_NAME} a {B_LINK_OPPONENT2_NAME}!"));
static const u8 sText_WildFled[] = _(BATTLE_TEXT("{PLAY_SE SE_FLEE}{B_LINK_OPPONENT1_NAME} fled!", "{PLAY_SE SE_FLEE}{B_LINK_OPPONENT1_NAME} utekl!")); //not in gen 5+, replaced with match was forfeited text
static const u8 sText_TwoWildFled[] = _(BATTLE_TEXT("{PLAY_SE SE_FLEE}{B_LINK_OPPONENT1_NAME} and {B_LINK_OPPONENT2_NAME} fled!", "{PLAY_SE SE_FLEE}{B_LINK_OPPONENT1_NAME} a {B_LINK_OPPONENT2_NAME} utekli!")); //not in gen 5+, replaced with match was forfeited text
static const u8 sText_PlayerDefeatedLinkTrainerTrainer1[] = _(BATTLE_TEXT("You defeated {B_TRAINER1_NAME_WITH_CLASS}!\p", "Porážíš soupeře {B_TRAINER1_NAME_WITH_CLASS}!\p"));
static const u8 sText_OpponentMon1Appeared[] = _(BATTLE_TEXT("{B_OPPONENT_MON1_NAME} appeared!\p", "Objevil se {B_OPPONENT_MON1_NAME}!\p"));
static const u8 sText_WildPkmnAppeared[] = _(BATTLE_TEXT("You encountered a wild {B_OPPONENT_MON1_NAME}!\p", "Objevil se divoký {B_OPPONENT_MON1_NAME}!\p"));
static const u8 sText_LegendaryPkmnAppeared[] = _(BATTLE_TEXT("You encountered a wild {B_OPPONENT_MON1_NAME}!\p", "Objevil se divoký {B_OPPONENT_MON1_NAME}!\p"));
static const u8 sText_WildPkmnAppearedPause[] = _(BATTLE_TEXT("You encountered a wild {B_OPPONENT_MON1_NAME}!{PAUSE 127}", "Objevil se divoký {B_OPPONENT_MON1_NAME}!{PAUSE 127}"));
static const u8 sText_TwoWildPkmnAppeared[] = _(BATTLE_TEXT("Oh! A wild {B_OPPONENT_MON1_NAME} and {B_OPPONENT_MON2_NAME} appeared!\p", "Objevili se divocí {B_OPPONENT_MON1_NAME} a {B_OPPONENT_MON2_NAME}!\p"));
static const u8 sText_GhostAppearedCantId[] = _(BATTLE_TEXT("The GHOST appeared!\pDarn!\nThe GHOST can't be ID'd!\p", "Objevil se DUCH!\pSakra!\nDUCHA nelze určit!\p"));
static const u8 sText_TheGhostAppeared[] = _(BATTLE_TEXT("The GHOST appeared!\p", "Objevil se DUCH!\p"));
static const u8 sText_Trainer1WantsToBattle[] = _(BATTLE_TEXT("You are challenged by {B_TRAINER1_NAME_WITH_CLASS}!\p", "Vyzývá tě {B_TRAINER1_NAME_WITH_CLASS}!\p"));
static const u8 sText_LinkTrainerWantsToBattle[] = _(BATTLE_TEXT("You are challenged by {B_LINK_OPPONENT1_NAME}!", "Vyzývá tě {B_LINK_OPPONENT1_NAME}!"));
static const u8 sText_TwoLinkTrainersWantToBattle[] = _(BATTLE_TEXT("You are challenged by {B_LINK_OPPONENT1_NAME} and {B_LINK_OPPONENT2_NAME}!", "Vyzývají tě {B_LINK_OPPONENT1_NAME} a {B_LINK_OPPONENT2_NAME}!"));
static const u8 sText_Trainer1SentOutPkmn[] = _(BATTLE_TEXT("{B_TRAINER1_NAME_WITH_CLASS} sent out {B_OPPONENT_MON1_NAME}!", "{B_TRAINER1_NAME_WITH_CLASS} posílá do boje {B_OPPONENT_MON1_NAME}!"));
static const u8 sText_Trainer1SentOutTwoPkmn[] = _(BATTLE_TEXT("{B_TRAINER1_NAME_WITH_CLASS} sent out {B_OPPONENT_MON1_NAME} and {B_OPPONENT_MON2_NAME}!", "{B_TRAINER1_NAME_WITH_CLASS} posílá do boje {B_OPPONENT_MON1_NAME} a {B_OPPONENT_MON2_NAME}!"));
static const u8 sText_Trainer1SentOutPkmn2[] = _(BATTLE_TEXT("{B_TRAINER1_NAME_WITH_CLASS} sent out {B_BUFF1}!", "{B_TRAINER1_NAME_WITH_CLASS} posílá do boje {B_BUFF1}!"));
static const u8 sText_LinkTrainerIntroSendOutPkmn[] = _(BATTLE_TEXT("{B_LINK_OPPONENT1_NAME} sent out {B_LINK_OPPONENT_MON1_NAME}!", "{B_LINK_OPPONENT1_NAME} posílá do boje {B_LINK_OPPONENT_MON1_NAME}!"));
static const u8 sText_LinkTrainerSentOutPkmn[] = _(BATTLE_TEXT("{B_LINK_OPPONENT1_NAME} sent out {B_BUFF1}!", "{B_LINK_OPPONENT1_NAME} posílá do boje {B_BUFF1}!"));
static const u8 sText_LinkTrainer2SentOutPkmn2[] = _(BATTLE_TEXT("{B_LINK_OPPONENT2_NAME} sent out {B_BUFF1}!", "{B_LINK_OPPONENT2_NAME} posílá do boje {B_BUFF1}!"));
static const u8 sText_LinkTrainerSentOutTwoPkmn[] = _(BATTLE_TEXT("{B_LINK_OPPONENT1_NAME} sent out {B_OPPONENT_MON1_NAME} and {B_OPPONENT_MON2_NAME}!", "{B_LINK_OPPONENT1_NAME} posílá do boje {B_OPPONENT_MON1_NAME} a {B_OPPONENT_MON2_NAME}!"));
static const u8 sText_TwoLinkTrainersIntroSendOutPkmn[] = _(BATTLE_TEXT("{B_LINK_OPPONENT1_NAME} sent out {B_LINK_OPPONENT_MON1_NAME}! {B_LINK_OPPONENT2_NAME} sent out {B_LINK_OPPONENT_MON2_NAME}!", "{B_LINK_OPPONENT1_NAME} posílá do boje {B_LINK_OPPONENT_MON1_NAME}! {B_LINK_OPPONENT2_NAME} posílá do boje {B_LINK_OPPONENT_MON2_NAME}!"));
static const u8 sText_LinkTrainerSentOutPkmn2[] = _(BATTLE_TEXT("{B_LINK_OPPONENT1_NAME} sent out {B_LINK_OPPONENT_MON2_NAME}!", "{B_LINK_OPPONENT1_NAME} posílá do boje {B_LINK_OPPONENT_MON2_NAME}!"));
static const u8 sText_LinkTrainerMultiSentOutPkmn[] = _(BATTLE_TEXT("{B_LINK_SCR_TRAINER_NAME} sent out {B_BUFF1}!", "{B_LINK_SCR_TRAINER_NAME} posílá do boje {B_BUFF1}!"));
static const u8 sText_GoPkmn[] = _(BATTLE_TEXT("Go! {B_PLAYER_MON1_NAME}!", "Do boje, {B_PLAYER_MON1_NAME}!"));
static const u8 sText_GoTwoPkmn[] = _(BATTLE_TEXT("Go! {B_PLAYER_MON1_NAME} and {B_PLAYER_MON2_NAME}!", "Do boje, {B_PLAYER_MON1_NAME} a {B_PLAYER_MON2_NAME}!"));
static const u8 sText_GoPkmn2[] = _(BATTLE_TEXT("Go! {B_BUFF1}!", "Do boje, {B_BUFF1}!"));
static const u8 sText_DoItPkmn[] = _(BATTLE_TEXT("You're in charge, {B_BUFF1}!", "Teď je to na tobě, {B_BUFF1}!"));
static const u8 sText_GoForItPkmn[] = _(BATTLE_TEXT("Go for it, {B_BUFF1}!", "Do toho, {B_BUFF1}!"));
static const u8 sText_BeCarefulPkmn[] = _(BATTLE_TEXT("Be careful, {B_PLAYER_MON1_NAME}!", "Opatrně, {B_PLAYER_MON1_NAME}!"));
static const u8 sText_JustALittleMorePkmn[] = _(BATTLE_TEXT("Just a little more! Hang in there, {B_BUFF1}!", "Ještě chvíli! Vydrž, {B_BUFF1}!")); //currently unused, will require code changes
static const u8 sText_YourFoesWeakGetEmPkmn[] = _(BATTLE_TEXT("Your opponent's weak! Get 'em, {B_BUFF1}!", "Soupeř je slabý! Do toho, {B_BUFF1}!"));
static const u8 sText_LinkPartnerSentOutPkmn1GoPkmn[] = _(BATTLE_TEXT("{B_LINK_PARTNER_NAME} sent out {B_LINK_PLAYER_MON1_NAME}! Go, {B_LINK_PLAYER_MON2_NAME}!", "{B_LINK_PARTNER_NAME} posílá do boje {B_LINK_PLAYER_MON1_NAME}! Do toho, {B_LINK_PLAYER_MON2_NAME}!"));
static const u8 sText_LinkPartnerSentOutPkmn2GoPkmn[] = _(BATTLE_TEXT("{B_LINK_PARTNER_NAME} sent out {B_LINK_PLAYER_MON2_NAME}! Go, {B_LINK_PLAYER_MON1_NAME}!", "{B_LINK_PARTNER_NAME} posílá do boje {B_LINK_PLAYER_MON2_NAME}! Do toho, {B_LINK_PLAYER_MON1_NAME}!"));
static const u8 sText_LinkPartnerSentOutPkmn1[] = _(BATTLE_TEXT("{B_LINK_PARTNER_NAME} sent out {B_BUFF1}!", "{B_LINK_PARTNER_NAME} posílá do boje {B_BUFF1}!"));
static const u8 sText_LinkPartnerSentOutPkmn2[] = _(BATTLE_TEXT("{B_LINK_PARTNER_NAME} sent out {B_BUFF1}!", "{B_LINK_PARTNER_NAME} posílá do boje {B_BUFF1}!"));
static const u8 sText_LinkPartnerWithdrewPkmn1[] = _(BATTLE_TEXT("{B_LINK_PARTNER_NAME} withdrew {B_LINK_PLAYER_MON1_NAME}!", "{B_LINK_PARTNER_NAME} stahuje z boje {B_LINK_PLAYER_MON1_NAME}!"));
static const u8 sText_LinkPartnerWithdrewPkmn2[] = _(BATTLE_TEXT("{B_LINK_PARTNER_NAME} withdrew {B_LINK_PLAYER_MON2_NAME}!", "{B_LINK_PARTNER_NAME} stahuje z boje {B_LINK_PLAYER_MON2_NAME}!"));
static const u8 sText_PkmnSwitchOut[] = _(BATTLE_TEXT("{B_BUFF1}, switch out! Come back!", "{B_BUFF1}, střídáme! Vrať se!")); //currently unused, I believe its used for when you switch on a Pokémon in shift mode
static const u8 sText_PkmnThatsEnough[] = _(BATTLE_TEXT("{B_BUFF1}, that's enough! Come back!", "{B_BUFF1}, to stačí! Vrať se!"));
static const u8 sText_PkmnComeBack[] = _(BATTLE_TEXT("{B_BUFF1}, come back!", "{B_BUFF1}, vrať se!"));
static const u8 sText_PkmnOkComeBack[] = _(BATTLE_TEXT("OK, {B_BUFF1}! Come back!", "Dobře, {B_BUFF1}! Vrať se!"));
static const u8 sText_PkmnGoodComeBack[] = _(BATTLE_TEXT("Good job, {B_BUFF1}! Come back!", "Skvělá práce, {B_BUFF1}! Vrať se!"));
static const u8 sText_Trainer1WithdrewPkmn[] = _(BATTLE_TEXT("{B_TRAINER1_NAME_WITH_CLASS} withdrew {B_BUFF1}!", "{B_TRAINER1_NAME_WITH_CLASS} stahuje z boje {B_BUFF1}!"));
static const u8 sText_Trainer2WithdrewPkmn[] = _(BATTLE_TEXT("{B_TRAINER2_NAME_WITH_CLASS} withdrew {B_BUFF1}!", "{B_TRAINER2_NAME_WITH_CLASS} stahuje z boje {B_BUFF1}!"));
static const u8 sText_LinkTrainer1WithdrewPkmn[] = _(BATTLE_TEXT("{B_LINK_OPPONENT1_NAME} withdrew {B_BUFF1}!", "{B_LINK_OPPONENT1_NAME} stahuje z boje {B_BUFF1}!"));
static const u8 sText_LinkTrainer2WithdrewPkmn[] = _(BATTLE_TEXT("{B_LINK_OPPONENT2_NAME} withdrew {B_BUFF1}!", "{B_LINK_OPPONENT2_NAME} stahuje z boje {B_BUFF1}!"));
static const u8 sText_WildPkmnPrefix[] = _(BATTLE_TEXT("The wild ", "Divoký "));
static const u8 sText_FoePkmnPrefix[] = _(BATTLE_TEXT("The opposing ", "Soupeřův "));
static const u8 sText_WildPkmnPrefixLower[] = _(BATTLE_TEXT("the wild ", "divoký "));
static const u8 sText_FoePkmnPrefixLower[] = _(BATTLE_TEXT("the opposing ", "soupeřův "));
static const u8 sText_EmptyString8[] = _("");
static const u8 sText_FoePkmnPrefix2[] = _(BATTLE_TEXT("Opposing", "Soupeřův"));
static const u8 sText_AllyPkmnPrefix[] = _(BATTLE_TEXT("Ally", "Spojenecký"));
static const u8 sText_FoePkmnPrefix3[] = _(BATTLE_TEXT("Opposing", "Soupeřův"));
static const u8 sText_AllyPkmnPrefix2[] = _(BATTLE_TEXT("Ally", "Spojenecký"));
static const u8 sText_FoePkmnPrefix4[] = _(BATTLE_TEXT("Opposing", "Soupeřův"));
static const u8 sText_AllyPkmnPrefix3[] = _(BATTLE_TEXT("Ally", "Spojenecký"));
static const u8 sText_AttackerUsedX[] = _(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} used {B_BUFF3}!", "{B_ATK_NAME_WITH_PREFIX} použil {B_BUFF3}!"));
static const u8 sText_ExclamationMark[] = _("!");
static const u8 sText_ExclamationMark2[] = _("!");
static const u8 sText_ExclamationMark3[] = _("!");
static const u8 sText_ExclamationMark4[] = _("!");
static const u8 sText_ExclamationMark5[] = _("!");
static const u8 sText_HP[] = _("HP");
static const u8 sText_Attack[] = _(BATTLE_TEXT("Attack", "Útok"));
static const u8 sText_Defense[] = _(BATTLE_TEXT("Defense", "Obrana"));
static const u8 sText_Speed[] = _(BATTLE_TEXT("Speed", "Rychlost"));
static const u8 sText_SpAttack[] = _(BATTLE_TEXT("Sp. Atk", "Sp. útok"));
static const u8 sText_SpDefense[] = _(BATTLE_TEXT("Sp. Def", "Sp. obr."));
static const u8 sText_Accuracy[] = _(BATTLE_TEXT("accuracy", "přesnost"));
static const u8 sText_Evasiveness[] = _(BATTLE_TEXT("evasiveness", "vyhýbání"));

const u8 *const gStatNamesTable[NUM_BATTLE_STATS] =
{
    [STAT_HP]      = sText_HP,
    [STAT_ATK]     = sText_Attack,
    [STAT_DEF]     = sText_Defense,
    [STAT_SPEED]   = sText_Speed,
    [STAT_SPATK]   = sText_SpAttack,
    [STAT_SPDEF]   = sText_SpDefense,
    [STAT_ACC]     = sText_Accuracy,
    [STAT_EVASION] = sText_Evasiveness,
};
const u8 *const gPokeblockWasTooXStringTable[FLAVOR_COUNT] =
{
    [FLAVOR_SPICY]  = COMPOUND_STRING(BATTLE_TEXT("was too spicy!", "byl příliš pálivý!")),
    [FLAVOR_DRY]    = COMPOUND_STRING(BATTLE_TEXT("was too dry!", "byl příliš suchý!")),
    [FLAVOR_SWEET]  = COMPOUND_STRING(BATTLE_TEXT("was too sweet!", "byl příliš sladký!")),
    [FLAVOR_BITTER] = COMPOUND_STRING(BATTLE_TEXT("was too bitter!", "byl příliš hořký!")),
    [FLAVOR_SOUR]   = COMPOUND_STRING(BATTLE_TEXT("was too sour!", "byl příliš kyselý!")),
};

static const u8 sText_Someones[] = _(BATTLE_TEXT("someone's", "něčí"));
static const u8 sText_Lanettes[] = _(BATTLE_TEXT("LANETTE's", "LANETTIN")); //no decapitalize until it is everywhere
static const u8 sText_Bills[] = _(BATTLE_TEXT("BILL's", "BILLŮV"));
static const u8 sText_EnigmaBerry[] = _("ENIGMA BERRY"); //no decapitalize until it is everywhere
static const u8 sText_BerrySuffix[] = _(" BERRY"); //no decapitalize until it is everywhere
const u8 gText_EmptyString3[] = _("");

static const u8 sText_TwoInGameTrainersDefeated[] = _(BATTLE_TEXT("You defeated {B_TRAINER1_NAME_WITH_CLASS} and {B_TRAINER2_NAME_WITH_CLASS}!\p", "Porážíš soupeře {B_TRAINER1_NAME_WITH_CLASS} a {B_TRAINER2_NAME_WITH_CLASS}!\p"));

// New battle strings.
const u8 gText_drastically[] = _(BATTLE_TEXT(" drastically", " prudce"));
const u8 gText_severely[] = _(BATTLE_TEXT("severely ", "prudce "));
static const u8 sText_TerrainReturnedToNormal[] = _(BATTLE_TEXT("The terrain returned to normal!", "Terén se vrátil do normálu!")); // Unused

const u8 *const gBattleStringsTable[STRINGID_COUNT] =
{
    [STRINGID_TRAINER1LOSETEXT]                     = COMPOUND_STRING("{B_TRAINER1_LOSE_TEXT}"),
    [STRINGID_PKMNGAINEDEXP]                        = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} gained{B_BUFF2} {B_BUFF3} Exp. Points!\p", "{B_BUFF1} získal{B_BUFF2} {B_BUFF3} bodů zkušeností!\p")),
    [STRINGID_PKMNGREWTOLV]                         = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} grew to Lv. {B_BUFF2}!{WAIT_SE}\p", "{B_BUFF1} dosáhl úr. {B_BUFF2}!{WAIT_SE}\p")),
    [STRINGID_PKMNLEARNEDMOVE]                      = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} learned {B_BUFF2}!{WAIT_SE}\p", "{B_BUFF1} se naučil {B_BUFF2}!{WAIT_SE}\p")),
    [STRINGID_TRYTOLEARNMOVE1]                      = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} wants to learn the move {B_BUFF2}.\p", "{B_BUFF1} se chce naučit útok {B_BUFF2}.\p")),
    [STRINGID_TRYTOLEARNMOVE2]                      = COMPOUND_STRING(BATTLE_TEXT("However, {B_BUFF1} already knows four moves.\p", "{B_BUFF1} už ale zná čtyři útoky.\p")),
    [STRINGID_TRYTOLEARNMOVE3]                      = COMPOUND_STRING(BATTLE_TEXT("Should another move be forgotten and replaced with {B_BUFF2}?", "Má zapomenout jiný útok a nahradit ho útokem {B_BUFF2}?")),
    [STRINGID_PKMNFORGOTMOVE]                       = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} forgot {B_BUFF2}…\p", "{B_BUFF1} zapomněl {B_BUFF2}…\p")),
    [STRINGID_STOPLEARNINGMOVE]                     = COMPOUND_STRING(BATTLE_TEXT("{PAUSE 32}Do you want to give up on having {B_BUFF1} learn {B_BUFF2}?", "{PAUSE 32}Má se {B_BUFF1} přestat učit útok {B_BUFF2}?")),
    [STRINGID_DIDNOTLEARNMOVE]                      = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} did not learn {B_BUFF2}.\p", "{B_BUFF1} se nenaučil {B_BUFF2}.\p")),
    [STRINGID_PKMNLEARNEDMOVE2]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} learned {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} se naučil {B_BUFF1}!")),
    [STRINGID_PKMNPROTECTEDITSELF]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} protected itself!", "{B_SCR_NAME_WITH_PREFIX} se ochránil!")),
    [STRINGID_ITDOESNTAFFECT]                       = COMPOUND_STRING(BATTLE_TEXT("It doesn't affect {B_DEF_NAME_WITH_PREFIX2}…", "Na tento cíl to nepůsobí: {B_DEF_NAME_WITH_PREFIX2}…")),
    [STRINGID_ITDOESNTAFFECTSCR]                    = COMPOUND_STRING(BATTLE_TEXT("It doesn't affect {B_SCR_NAME_WITH_PREFIX2}…", "Na tento cíl to nepůsobí: {B_SCR_NAME_WITH_PREFIX2}…")),
    [STRINGID_BATTLERFAINTED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} fainted!\p", "{B_SCR_NAME_WITH_PREFIX} omdlel!\p")),
    [STRINGID_PLAYERGOTMONEY]                       = COMPOUND_STRING(BATTLE_TEXT("You got ¥{B_BUFF1} for winning!\p", "Odměna za vítězství: ¥{B_BUFF1}!\p")),
    [STRINGID_PLAYERWHITEOUT]                       = COMPOUND_STRING(BATTLE_TEXT("You have no more Pokémon that can fight!\p", "Už nemáš žádné Pokémony schopné boje!\p")),
    [STRINGID_PLAYERWHITEOUT2_WILD]                 = COMPOUND_STRING(BATTLE_TEXT("You panicked and dropped ¥{B_BUFF1}…", "V panice ti vypadlo ¥{B_BUFF1}…")),
    [STRINGID_PLAYERWHITEOUT2_TRAINER]              = COMPOUND_STRING(BATTLE_TEXT("You gave ¥{B_BUFF1} to the winner…", "Vítěz si od tebe vzal ¥{B_BUFF1}…")),
    [STRINGID_PLAYERWHITEOUT3]                      = COMPOUND_STRING(BATTLE_TEXT("You were overwhelmed by your defeat!", "Porážka tě úplně zdrtila!")),
    [STRINGID_PREVENTSESCAPE]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} prevents escape with {B_SCR_ABILITY}!\p", "{B_SCR_NAME_WITH_PREFIX} brání útěku schopností {B_SCR_ABILITY}!\p")),
    [STRINGID_HITXTIMES]                            = COMPOUND_STRING(BATTLE_TEXT("The Pokémon was hit {B_BUFF1} time{B_BUFF2}!", "Počet zásahů: {B_BUFF1}{B_BUFF2}!")),
    [STRINGID_PKMNFELLASLEEP]                       = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} fell asleep!", "{B_EFF_NAME_WITH_PREFIX} usnul!")),
    [STRINGID_PKMNMADESLEEP]                        = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} made {B_EFF_NAME_WITH_PREFIX2} sleep!", "{B_SCR_NAME_WITH_PREFIX} použil {B_BUFF1}; {B_EFF_NAME_WITH_PREFIX2} usnul!")), //not in gen 5+, ability popup
    [STRINGID_PKMNALREADYASLEEP]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is already asleep!", "{B_SCR_NAME_WITH_PREFIX} už spí!")),
    [STRINGID_PKMNALREADYASLEEP2]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is already asleep!", "{B_ATK_NAME_WITH_PREFIX} už spí!")),
    [STRINGID_PKMNWASPOISONED]                      = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} was poisoned!", "{B_EFF_NAME_WITH_PREFIX} byl otráven!")),
    [STRINGID_PKMNPOISONEDBY]                       = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} was poisoned by {B_SCR_NAME_WITH_PREFIX2}'s {B_BUFF1}!", "{B_EFF_NAME_WITH_PREFIX} byl otráven! Způsobil to {B_SCR_NAME_WITH_PREFIX2} pomocí {B_BUFF1}!")), //not in gen 5+, ability popup
    [STRINGID_PKMNHURTBYPOISON]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was hurt by its poisoning!", "{B_ATK_NAME_WITH_PREFIX} trpí otravou!")),
    [STRINGID_PKMNALREADYPOISONED]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is already poisoned!", "{B_SCR_NAME_WITH_PREFIX} už je otrávený!")),
    [STRINGID_PKMNBADLYPOISONED]                    = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} was badly poisoned!", "{B_EFF_NAME_WITH_PREFIX} byl těžce otráven!")),
    [STRINGID_PKMNENERGYDRAINED]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} had its energy drained!", "{B_SCR_NAME_WITH_PREFIX} přišel o část energie!")),
    [STRINGID_PKMNWASBURNED]                        = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} was burned!", "{B_EFF_NAME_WITH_PREFIX} utrpěl popálení!")),
    [STRINGID_PKMNBURNEDBY]                         = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} burned {B_EFF_NAME_WITH_PREFIX2}!", "{B_SCR_NAME_WITH_PREFIX} použil {B_BUFF1}; {B_EFF_NAME_WITH_PREFIX2} utrpěl popálení!")), //not in gen 5+, ability popup
    [STRINGID_PKMNHURTBYBURN]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was hurt by its burn!", "{B_ATK_NAME_WITH_PREFIX} trpí popálením!")),
    [STRINGID_PKMNWASFROZEN]                        = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} was frozen solid!", "{B_EFF_NAME_WITH_PREFIX} úplně zmrzl!")),
    [STRINGID_PKMNFROZENBY]                         = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} froze {B_EFF_NAME_WITH_PREFIX2} solid!", "{B_SCR_NAME_WITH_PREFIX} použil {B_BUFF1}; {B_EFF_NAME_WITH_PREFIX2} úplně zmrzl!")), //not in gen 5+, ability popup
    [STRINGID_PKMNISFROZEN]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is frozen solid!", "{B_ATK_NAME_WITH_PREFIX} je úplně zmrzlý!")),
    [STRINGID_PKMNWASDEFROSTED]                     = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} thawed out!", "{B_SCR_NAME_WITH_PREFIX} rozmrzl!")),
    [STRINGID_PKMNWASDEFROSTEDBY]                   = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_CURRENT_MOVE} melted the ice!", "{B_SCR_NAME_WITH_PREFIX} rozpustil led útokem {B_CURRENT_MOVE}!")),
    [STRINGID_PKMNWASPARALYZED]                     = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} is paralyzed, so it may be unable to move!", "{B_EFF_NAME_WITH_PREFIX} ochrnul a možná se nebude moci hýbat!")),
    [STRINGID_PKMNWASPARALYZEDBY]                   = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} paralyzed {B_EFF_NAME_WITH_PREFIX2}, so it may be unable to move!", "{B_SCR_NAME_WITH_PREFIX} použil {B_BUFF1}; {B_EFF_NAME_WITH_PREFIX2} ochrnul a možná se nebude moci hýbat!")), //not in gen 5+, ability popup
    [STRINGID_PKMNISPARALYZED]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} couldn't move because it's paralyzed!", "{B_ATK_NAME_WITH_PREFIX} se kvůli ochrnutí nemohl pohnout!")),
    [STRINGID_PKMNISALREADYPARALYZED]               = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is already paralyzed!", "{B_SCR_NAME_WITH_PREFIX} už je ochrnutý!")),
    [STRINGID_PKMNHEALEDPARALYSIS]                  = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} was cured of paralysis!", "{B_DEF_NAME_WITH_PREFIX} se zbavil ochrnutí!")),
    [STRINGID_STATSWONTINCREASE]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} won't go any higher!", "{B_SCR_NAME_WITH_PREFIX}: statistika {B_BUFF1} už nemůže vzrůst!")),
    [STRINGID_STATSWONTDECREASE]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} won't go any lower!", "{B_SCR_NAME_WITH_PREFIX}: statistika {B_BUFF1} už nemůže klesnout!")),
    [STRINGID_PKMNISCONFUSED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is confused!", "{B_ATK_NAME_WITH_PREFIX} je zmatený!")),
    [STRINGID_PKMNHEALEDCONFUSION]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} snapped out of its confusion!", "{B_ATK_NAME_WITH_PREFIX} se zbavil zmatení!")),
    [STRINGID_PKMNWASCONFUSED]                      = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} became confused!", "{B_EFF_NAME_WITH_PREFIX} byl zmaten!")),
    [STRINGID_PKMNALREADYCONFUSED]                  = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} is already confused!", "{B_DEF_NAME_WITH_PREFIX} už je zmatený!")),
    [STRINGID_PKMNFELLINLOVE]                       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} fell in love!", "{B_DEF_NAME_WITH_PREFIX} se zamiloval!")),
    [STRINGID_PKMNINLOVE]                           = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is in love with {B_SCR_NAME_WITH_PREFIX2}!", "{B_ATK_NAME_WITH_PREFIX} je zamilovaný! Jeho láska: {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNIMMOBILIZEDBYLOVE]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is immobilized by love!", "{B_ATK_NAME_WITH_PREFIX} se kvůli lásce nemůže hýbat!")),
    [STRINGID_PKMNCHANGEDTYPE]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} transformed into the {B_BUFF1} type!", "{B_ATK_NAME_WITH_PREFIX} změnil svůj typ na {B_BUFF1}!")),
    [STRINGID_PKMNFLINCHED]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} flinched and couldn't move!", "{B_ATK_NAME_WITH_PREFIX} zaváhal a nemohl se pohnout!")),
    [STRINGID_PKMNREGAINEDHEALTH]                   = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s HP was restored.", "{B_DEF_NAME_WITH_PREFIX} si obnovil HP.")),
    [STRINGID_PKMNHPFULL]                           = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s HP is full!", "{B_DEF_NAME_WITH_PREFIX} má plné HP!")),
    [STRINGID_PKMNRAISEDSPDEF]                      = COMPOUND_STRING(BATTLE_TEXT("Light Screen made {B_ATK_TEAM2} side stronger against special moves!", "Light Screen posílil {B_ATK_TEAM2} tým proti speciálním útokům!")),
    [STRINGID_PKMNRAISEDDEF]                        = COMPOUND_STRING(BATTLE_TEXT("Reflect made {B_ATK_TEAM2} side stronger against physical moves!", "Reflect posílil {B_ATK_TEAM2} tým proti fyzickým útokům!")),
    [STRINGID_PKMNAURORAVEIL]                       = COMPOUND_STRING(BATTLE_TEXT("Aurora Veil made {B_ATK_TEAM2} side stronger against physical and special moves!", "Aurora Veil posílil {B_ATK_TEAM2} tým proti fyzickým i speciálním útokům!")),
    [STRINGID_PKMNCOVEREDBYVEIL]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_TEAM1} side became cloaked in a mystical veil!", "{B_ATK_TEAM1} tým se zahalil tajemným závojem!")),
    [STRINGID_PKMNUSEDSAFEGUARD]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is protected by Safeguard!", "{B_SCR_NAME_WITH_PREFIX} je chráněn útokem Safeguard!")),
    [STRINGID_PKMNSAFEGUARDEXPIRED]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_TEAM1} side is no longer protected by the mystical veil!", "{B_ATK_TEAM1} tým už není chráněn tajemným závojem!")),
    [STRINGID_PKMNWENTTOSLEEP]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} went to sleep!", "{B_ATK_NAME_WITH_PREFIX} usnul!")), //not in gen 5+
    [STRINGID_PKMNSLEPTHEALTHY]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} slept and restored its HP!", "{B_ATK_NAME_WITH_PREFIX} usnul a obnovil si HP!")),
    [STRINGID_PKMNWHIPPEDWHIRLWIND]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} whipped up a whirlwind!", "{B_ATK_NAME_WITH_PREFIX} rozpoutal větrný vír!")),
    [STRINGID_PKMNTOOKSUNLIGHT]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} absorbed light!", "{B_ATK_NAME_WITH_PREFIX} pohltil světlo!")),
    [STRINGID_PKMNLOWEREDHEAD]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} tucked in its head!", "{B_ATK_NAME_WITH_PREFIX} zatáhl hlavu!")),
    [STRINGID_PKMNFLEWHIGH]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} flew up high!", "{B_ATK_NAME_WITH_PREFIX} vyletěl vysoko!")),
    [STRINGID_PKMNDUGHOLE]                          = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} burrowed its way under the ground!", "{B_ATK_NAME_WITH_PREFIX} se zahrabal pod zem!")),
    [STRINGID_PKMNSQUEEZEDBYBIND]                   = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} was squeezed by {B_SCR_NAME_WITH_PREFIX2}!", "{B_EFF_NAME_WITH_PREFIX} je sevřen! Svírá ho {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNTRAPPEDINVORTEX]                  = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} became trapped in the vortex!", "{B_EFF_NAME_WITH_PREFIX} uvízl ve víru!")),
    [STRINGID_PKMNWRAPPEDBY]                        = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} was wrapped by {B_SCR_NAME_WITH_PREFIX2}!", "{B_EFF_NAME_WITH_PREFIX} je omotán! Omotal ho {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNCLAMPED]                          = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} clamped down on {B_EFF_NAME_WITH_PREFIX2}!", "{B_SCR_NAME_WITH_PREFIX} sevřel cíl: {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNHURTBY]                           = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is hurt by {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} je zraňován útokem {B_BUFF1}!")),
    [STRINGID_PKMNFREEDFROM]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was freed from {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} se vymanil z účinků útoku {B_BUFF1}!")),
    [STRINGID_PKMNCRASHED]                          = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} kept going and crashed!", "{B_ATK_NAME_WITH_PREFIX} pokračoval v útoku a narazil!")),
    [STRINGID_PKMNSHROUDEDINMIST]                   = gText_PkmnShroudedInMist,
    [STRINGID_PKMNPROTECTEDBYMIST]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is protected by the mist!", "{B_SCR_NAME_WITH_PREFIX} je chráněn mlhou!")),
    [STRINGID_PKMNGETTINGPUMPED]                    = gText_PkmnGettingPumped,
    [STRINGID_PKMNHITWITHRECOIL]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was damaged by the recoil!", "{B_ATK_NAME_WITH_PREFIX} utrpěl zpětné zranění!")),
    [STRINGID_PKMNPROTECTEDITSELF2]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} protected itself!", "{B_ATK_NAME_WITH_PREFIX} se ochránil!")),
    [STRINGID_PKMNBUFFETEDBYSANDSTORM]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is buffeted by the sandstorm!", "{B_ATK_NAME_WITH_PREFIX} je bičován písečnou bouří!")),
    [STRINGID_PKMNPELTEDBYHAIL]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is buffeted by the hail!", "{B_ATK_NAME_WITH_PREFIX} je bičován kroupami!")),
    [STRINGID_PKMNSEEDED]                           = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} was seeded!", "{B_DEF_NAME_WITH_PREFIX} byl oset semeny!")),
    [STRINGID_PKMNAVOIDEDATTACK]                    = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} avoided the attack!", "{B_DEF_NAME_WITH_PREFIX} se vyhnul útoku!")),
    [STRINGID_BATTLERAVOIDEDATTACK]                 = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} avoided the attack!", "{B_SCR_NAME_WITH_PREFIX} se vyhnul útoku!")),
    [STRINGID_PKMNSAPPEDBYLEECHSEED]                = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s health is sapped by Leech Seed!", "{B_SCR_NAME_WITH_PREFIX} ztrácí HP kvůli útoku Leech Seed!")),
    [STRINGID_PKMNFASTASLEEP]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is fast asleep.", "{B_ATK_NAME_WITH_PREFIX} tvrdě spí.")),
    [STRINGID_PKMNWOKEUP]                           = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} woke up!", "{B_ATK_NAME_WITH_PREFIX} se probudil!")),
    [STRINGID_PKMNWOKEUPINUPROAR]                   = COMPOUND_STRING(BATTLE_TEXT("The uproar woke {B_EFF_NAME_WITH_PREFIX2}!", "Kvůli hluku se probudil {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNCAUSEDUPROAR]                     = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} caused an uproar!", "{B_EFF_NAME_WITH_PREFIX} spustil povyk!")),
    [STRINGID_PKMNMAKINGUPROAR]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is making an uproar!", "{B_ATK_NAME_WITH_PREFIX} dělá povyk!")),
    [STRINGID_PKMNCALMEDDOWN]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} calmed down.", "{B_ATK_NAME_WITH_PREFIX} se uklidnil.")),
    [STRINGID_PKMNSTOCKPILED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} stockpiled {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} nahromadil zásoby: {B_BUFF1}!")),
    [STRINGID_PKMNCANTSLEEPINUPROAR2]               = COMPOUND_STRING(BATTLE_TEXT("But {B_DEF_NAME_WITH_PREFIX2} can't sleep in an uproar!", "Kvůli hluku ale {B_DEF_NAME_WITH_PREFIX2} nemůže usnout!")),
    [STRINGID_UPROARKEPTPKMNAWAKE]                  = COMPOUND_STRING(BATTLE_TEXT("But the uproar kept {B_DEF_NAME_WITH_PREFIX2} awake!", "Kvůli hluku ale {B_DEF_NAME_WITH_PREFIX2} neusnul!")),
    [STRINGID_PKMNSTAYEDAWAKEUSING]                 = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} stayed awake!", "{B_DEF_NAME_WITH_PREFIX} zůstal vzhůru!")),
    [STRINGID_PKMNSTORINGENERGY]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is storing energy!", "{B_ATK_NAME_WITH_PREFIX} shromažďuje energii!")),
    [STRINGID_PKMNUNLEASHEDENERGY]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} unleashed its energy!", "{B_ATK_NAME_WITH_PREFIX} uvolnil svou energii!")),
    [STRINGID_PKMNFATIGUECONFUSION]                 = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} became confused due to fatigue!", "{B_SCR_NAME_WITH_PREFIX} je zmatený únavou!")),
    [STRINGID_PLAYERPICKEDUPMONEY]                  = COMPOUND_STRING(BATTLE_TEXT("You picked up ¥{B_BUFF1}!\p", "Na zemi bylo ¥{B_BUFF1}!\p")),
    [STRINGID_PKMNUNAFFECTED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} is unaffected!", "{B_DEF_NAME_WITH_PREFIX}: žádný účinek!")),
    [STRINGID_PKMNTRANSFORMEDINTO]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} transformed into {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} se proměnil! Nová podoba: {B_BUFF1}!")),
    [STRINGID_PKMNMADESUBSTITUTE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} put in a substitute!", "{B_ATK_NAME_WITH_PREFIX} vytvořil náhradníka!")),
    [STRINGID_PKMNHASSUBSTITUTE]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} already has a substitute!", "{B_ATK_NAME_WITH_PREFIX} už má náhradníka!")),
    [STRINGID_SUBSTITUTEDAMAGED]                    = COMPOUND_STRING(BATTLE_TEXT("The substitute took damage for {B_DEF_NAME_WITH_PREFIX2}!\p", "Náhradník utrpěl zranění místo Pokémona: {B_DEF_NAME_WITH_PREFIX2}!\p")),
    [STRINGID_PKMNSUBSTITUTEFADED]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s substitute faded!\p", "{B_SCR_NAME_WITH_PREFIX} přišel o náhradníka!\p")),
    [STRINGID_PKMNMUSTRECHARGE]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} must recharge!", "{B_ATK_NAME_WITH_PREFIX} musí načerpat síly!")),
    [STRINGID_PKMNRAGEBUILDING]                     = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s rage is building!", "{B_DEF_NAME_WITH_PREFIX} zuří čím dál víc!")),
    [STRINGID_PKMNMOVEWASDISABLED]                  = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1} was disabled!", "{B_DEF_NAME_WITH_PREFIX} má zablokovaný útok {B_BUFF1}!")),
    [STRINGID_PKMNMOVEISDISABLED]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s {B_CURRENT_MOVE} is disabled!\p", "{B_ATK_NAME_WITH_PREFIX} má zablokovaný útok {B_CURRENT_MOVE}!\p")),
    [STRINGID_PKMNMOVEDISABLEDNOMORE]               = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s move is no longer disabled!", "{B_SCR_NAME_WITH_PREFIX} už nemá útok zablokovaný!")),
    [STRINGID_PKMNGOTENCORE]                        = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} must do an encore!", "{B_DEF_NAME_WITH_PREFIX} musí zopakovat svůj útok!")),
    [STRINGID_PKMNGOTENCOREDMOVE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} can only use {B_CURRENT_MOVE}!\p", "{B_SCR_NAME_WITH_PREFIX} může použít jen {B_CURRENT_MOVE}!\p")),
    [STRINGID_PKMNENCOREENDED]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} ended its encore!", "{B_SCR_NAME_WITH_PREFIX} už nemusí opakovat útok!")),
    [STRINGID_PKMNTOOKAIM]                          = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} took aim at {B_DEF_NAME_WITH_PREFIX2}!", "{B_ATK_NAME_WITH_PREFIX} si zaměřil cíl: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNSKETCHEDMOVE]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} sketched {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} si načrtl útok {B_BUFF1}!")),
    [STRINGID_PKMNTRYINGTOTAKEFOE]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is hoping to take its attacker down with it!", "{B_ATK_NAME_WITH_PREFIX} chce vzít útočníka s sebou!")),
    [STRINGID_PKMNTOOKFOE]                          = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} took its attacker down with it!", "{B_DEF_NAME_WITH_PREFIX} vzal útočníka s sebou!")),
    [STRINGID_PKMNREDUCEDPP]                        = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} lost {B_BUFF2} PP from {B_BUFF1}!", "{B_DEF_NAME_WITH_PREFIX} ztratil {B_BUFF2} PP u útoku {B_BUFF1}!")),
    [STRINGID_PKMNSTOLEITEM]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} stole {B_BUFF2}'s {B_LAST_ITEM}!", "{B_ATK_NAME_WITH_PREFIX} kradl! Oběť: {B_BUFF2}; kořist: {B_LAST_ITEM}!")),
    [STRINGID_TARGETCANTESCAPENOW]                  = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} can no longer escape!", "{B_DEF_NAME_WITH_PREFIX} už nemůže utéct!")),
    [STRINGID_PKMNFELLINTONIGHTMARE]                = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} began having a nightmare!", "{B_DEF_NAME_WITH_PREFIX} začal mít noční můru!")),
    [STRINGID_PKMNLOCKEDINNIGHTMARE]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is locked in a nightmare!", "{B_ATK_NAME_WITH_PREFIX} je uvězněn v noční můře!")),
    [STRINGID_PKMNLAIDCURSE]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} cut its own HP and put a curse on {B_DEF_NAME_WITH_PREFIX2}!", "{B_ATK_NAME_WITH_PREFIX} obětoval část HP a seslal kletbu na cíl: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNAFFLICTEDBYCURSE]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is afflicted by the curse!", "{B_ATK_NAME_WITH_PREFIX} trpí kletbou!")),
    [STRINGID_SPIKESSCATTERED]                      = COMPOUND_STRING(BATTLE_TEXT("Spikes were scattered on the ground all around {B_DEF_TEAM2} side!", "Bodce se rozsypaly na zem! Obklopují {B_DEF_TEAM2} tým!")),
    [STRINGID_PKMNHURTBYSPIKES]                     = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} was hurt by the spikes!", "{B_SCR_NAME_WITH_PREFIX} se zranil o bodce!")),
    [STRINGID_PKMNIDENTIFIED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} was identified!", "{B_DEF_NAME_WITH_PREFIX} byl odhalen!")),
    [STRINGID_PKMNPERISHCOUNTFELL]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s perish count fell to {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX}: odpočet zániku klesl na {B_BUFF1}!")),
    [STRINGID_PKMNBRACEDITSELF]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} braced itself!", "{B_ATK_NAME_WITH_PREFIX} se zapřel!")),
    [STRINGID_PKMNENDUREDHIT]                       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} endured the hit!", "{B_DEF_NAME_WITH_PREFIX} vydržel zásah!")),
    [STRINGID_MAGNITUDESTRENGTH]                    = COMPOUND_STRING("Magnitude {B_BUFF1}!"),
    [STRINGID_PKMNCUTHPMAXEDATTACK]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} cut its own HP and maximized its Attack!", "{B_ATK_NAME_WITH_PREFIX} obětoval část HP a zvýšil svůj Útok na maximum!")),
    [STRINGID_PKMNCOPIEDSTATCHANGES]                = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} copied {B_EFF_NAME_WITH_PREFIX2}'s stat changes!", "{B_SCR_NAME_WITH_PREFIX} zkopíroval změny statistik! Předloha: {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNGOTFREE]                          = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was freed from {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} se vymanil z účinků útoku {B_BUFF1}!")),
    [STRINGID_PKMNSHEDLEECHSEED]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was freed from Leech Seed!", "{B_ATK_NAME_WITH_PREFIX} se zbavil účinků útoku Leech Seed!")),
    [STRINGID_PKMNBLEWAWAYSPIKES]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} blew away Spikes!", "{B_ATK_NAME_WITH_PREFIX} odfoukl bodce útoku Spikes!")), // Not in gen 5+
    [STRINGID_PKMNFLEDFROMBATTLE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} fled from battle!", "{B_ATK_NAME_WITH_PREFIX} utekl z boje!")),
    [STRINGID_PKMNFORESAWATTACK]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} foresaw an attack!", "{B_ATK_NAME_WITH_PREFIX} předvídal útok!")),
    [STRINGID_PKMNTOOKATTACK]                       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} took the {B_BUFF1} attack!", "{B_DEF_NAME_WITH_PREFIX} schytal útok {B_BUFF1}!")),
    [STRINGID_PKMNATTACK]                           = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1}'s attack!", "Útok Pokémona: {B_BUFF1}!")), // Not in gen 5+
    [STRINGID_PKMNCENTERATTENTION]                  = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} became the center of attention!", "{B_DEF_NAME_WITH_PREFIX} se stal středem pozornosti!")),
    [STRINGID_PKMNCHARGINGPOWER]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} began charging power!", "{B_ATK_NAME_WITH_PREFIX} začal hromadit sílu!")),
    [STRINGID_NATUREPOWERTURNEDINTO]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s Nature Power turned into {B_CURRENT_MOVE}!", "{B_ATK_NAME_WITH_PREFIX}: Nature Power se změnil na {B_CURRENT_MOVE}!")),
    [STRINGID_PKMNSTATUSNORMAL]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s status returned to normal!", "{B_ATK_NAME_WITH_PREFIX} se vrátil do normálního stavu!")),
    [STRINGID_PKMNHASNOMOVESLEFT]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} has no moves left that it can use!\p", "{B_ATK_NAME_WITH_PREFIX} už nemá žádné použitelné útoky!\p")),
    [STRINGID_PKMNSUBJECTEDTOTORMENT]               = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} was subjected to torment!", "{B_DEF_NAME_WITH_PREFIX} podlehl trýzni!")),
    [STRINGID_PKMNCANTUSEMOVETORMENT]               = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} can't use the same move twice in a row due to the torment!\p", "{B_ATK_NAME_WITH_PREFIX} kvůli trýzni nemůže použít stejný útok dvakrát za sebou!\p")),
    [STRINGID_PKMNTIGHTENINGFOCUS]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is tightening its focus!", "{B_SCR_NAME_WITH_PREFIX} se hluboce soustředí!")),
    [STRINGID_PKMNFELLFORTAUNT]                     = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} fell for the taunt!", "{B_DEF_NAME_WITH_PREFIX} podlehl provokaci!")),
    [STRINGID_PKMNCANTUSEMOVETAUNT]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} can't use {B_CURRENT_MOVE} after the taunt!\p", "{B_ATK_NAME_WITH_PREFIX} po provokaci nemůže použít {B_CURRENT_MOVE}!\p")),
    [STRINGID_PKMNREADYTOHELP]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is ready to help {B_DEF_NAME_WITH_PREFIX2}!", "{B_ATK_NAME_WITH_PREFIX} je připraven pomoci! Jeho spojenec: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNSWITCHEDITEMS]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} switched items with its target!", "{B_ATK_NAME_WITH_PREFIX} si s cílem vyměnil předměty!")),
    [STRINGID_PKMNCOPIEDFOE]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} copied {B_DEF_NAME_WITH_PREFIX2}'s Ability!", "{B_ATK_NAME_WITH_PREFIX} zkopíroval schopnost Pokémona: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNWISHCAMETRUE]                     = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1}'s wish came true!", "{B_BUFF1}: přání se splnilo!")),
    [STRINGID_PKMNPLANTEDROOTS]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} planted its roots!", "{B_ATK_NAME_WITH_PREFIX} zapustil kořeny!")),
    [STRINGID_PKMNABSORBEDNUTRIENTS]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} absorbed nutrients with its roots!", "{B_ATK_NAME_WITH_PREFIX} nasál kořeny živiny!")),
    [STRINGID_PKMNANCHOREDITSELF]                   = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} is anchored in place with its roots!", "{B_EFF_NAME_WITH_PREFIX} je pevně zakořeněný!")),
    [STRINGID_PKMNWASMADEDROWSY]                    = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} grew drowsy!", "{B_DEF_NAME_WITH_PREFIX} začal být ospalý!")),
    [STRINGID_PKMNKNOCKEDOFF]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} knocked off {B_EFF_NAME_WITH_PREFIX2}'s {B_LAST_ITEM}!", "{B_ATK_NAME_WITH_PREFIX} vyrazil předmět {B_LAST_ITEM}! Přišel o něj {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNSWAPPEDABILITIES]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} swapped Abilities with its target!", "{B_ATK_NAME_WITH_PREFIX} si s cílem vyměnil schopnosti!")),
    [STRINGID_PKMNSEALEDOPPONENTMOVE]               = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} sealed any moves its target shares with it!", "{B_ATK_NAME_WITH_PREFIX} zapečetil všechny útoky, které sdílí se svým cílem!")),
    [STRINGID_PKMNCANTUSEMOVESEALED]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} can't use its sealed {B_CURRENT_MOVE}!\p", "{B_ATK_NAME_WITH_PREFIX} nemůže použít zapečetěný útok {B_CURRENT_MOVE}!\p")),
    [STRINGID_PKMNWANTSGRUDGE]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} wants its target to bear a grudge!", "{B_ATK_NAME_WITH_PREFIX} chce uvalit zášť na svůj cíl!")),
    [STRINGID_PKMNLOSTPPGRUDGE]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} lost all of {B_BUFF1}'s PP due to the grudge!", "{B_ATK_NAME_WITH_PREFIX} kvůli zášti ztratil všechny PP útoku {B_BUFF1}!")),
    [STRINGID_PKMNSHROUDEDITSELF]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} shrouded itself with Magic Coat!", "{B_ATK_NAME_WITH_PREFIX} se zahalil do Magic Coat!")),
    [STRINGID_PKMNMOVEBOUNCED]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} bounced the {B_CURRENT_MOVE} back!", "{B_ATK_NAME_WITH_PREFIX} odrazil útok {B_CURRENT_MOVE} zpět!")),
    [STRINGID_PKMNWAITSFORTARGET]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is waiting for a target to make a move!", "{B_ATK_NAME_WITH_PREFIX} čeká, až jeho cíl zaútočí!")),
    [STRINGID_PKMNSNATCHEDMOVE]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} snatched {B_SCR_NAME_WITH_PREFIX2}'s move!", "{B_ATK_NAME_WITH_PREFIX} ukradl útok! Oběť: {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNMADEITRAIN]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} made it rain!", "{B_SCR_NAME_WITH_PREFIX} schopností {B_SCR_ABILITY} přivolal déšť!")), //not in gen 5+, ability popup
    [STRINGID_PKMNPROTECTEDBY]                      = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} was protected by {B_DEF_ABILITY}!", "{B_DEF_NAME_WITH_PREFIX} byl chráněn schopností {B_DEF_ABILITY}!")), //not in gen 5+, ability popup
    [STRINGID_PKMNPREVENTSUSAGE]                    = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY} prevents {B_ATK_NAME_WITH_PREFIX2} from using {B_CURRENT_MOVE}!", "{B_DEF_NAME_WITH_PREFIX} schopností {B_DEF_ABILITY} brání útoku! {B_ATK_NAME_WITH_PREFIX2} nemůže použít {B_CURRENT_MOVE}!")), //not in gen 5+, ability popup
    [STRINGID_PKMNRESTOREDHPUSING]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} had its HP restored.", "{B_SCR_NAME_WITH_PREFIX} si obnovil HP.")),
    [STRINGID_PKMNCHANGEDTYPEWITH]                  = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX}'s type changed to {B_BUFF1}!", "{B_EFF_NAME_WITH_PREFIX} změnil svůj typ na {B_BUFF1}!")),
    [STRINGID_PKMNPREVENTSROMANCEWITH]              = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY} prevents romance!", "{B_DEF_NAME_WITH_PREFIX} schopností {B_DEF_ABILITY} brání zamilování!")), //not in gen 5+, ability popup
    [STRINGID_PKMNPREVENTSCONFUSIONWITH]            = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} cannot be confused!", "{B_SCR_NAME_WITH_PREFIX} nemůže být zmaten!")),
    [STRINGID_PKMNRAISEDFIREPOWERWITH]              = COMPOUND_STRING(BATTLE_TEXT("The power of {B_SCR_NAME_WITH_PREFIX}'s Fire-type moves rose!", "{B_SCR_NAME_WITH_PREFIX} posílil své ohnivé útoky!")),
    [STRINGID_PKMNANCHORSITSELFWITH]                = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} is anchored in place with its suction cups!", "{B_EFF_NAME_WITH_PREFIX} se pevně přichytil přísavkami!")),
    [STRINGID_PKMNPREVENTSSTATLOSSWITH]             = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s stats were not lowered!", "{B_SCR_NAME_WITH_PREFIX}: statistiky se nesnížily!")),
    [STRINGID_PKMNHURTSWITH]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was hurt by {B_DEF_NAME_WITH_PREFIX2}'s {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} utrpěl zranění! Způsobil ho {B_DEF_NAME_WITH_PREFIX2} pomocí {B_BUFF1}!")),
    [STRINGID_PKMNTRACED]                           = COMPOUND_STRING(BATTLE_TEXT("It traced {B_BUFF1}'s {B_BUFF2}!", "Zkopíroval schopnost! Předloha: {B_BUFF1}; schopnost: {B_BUFF2}!")),
    [STRINGID_STATSHARPLY]                          = gText_StatSharply,
    [STRINGID_STATHARSHLY]                          = COMPOUND_STRING(BATTLE_TEXT("harshly ", "výrazně ")),
    [STRINGID_STATROSE]                             = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} rose{B_BUFF2}!", "{B_SCR_NAME_WITH_PREFIX}: statistika {B_BUFF1} vzrostla{B_BUFF2}!")),
    [STRINGID_STATFELL]                             = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} {B_BUFF2}fell!", "{B_SCR_NAME_WITH_PREFIX}: statistika {B_BUFF1} {B_BUFF2}klesla!")),
    [STRINGID_CRITICALHIT]                          = COMPOUND_STRING(BATTLE_TEXT("A critical hit!", "Kritický zásah!")),
    [STRINGID_ONEHITKO]                             = COMPOUND_STRING(BATTLE_TEXT("It's a one-hit KO!", "Vyřazení jediným úderem!")),
    [STRINGID_123POOF]                              = COMPOUND_STRING(BATTLE_TEXT("One…{PAUSE 10}two…{PAUSE 10}and…{PAUSE 10}{PAUSE 20}{PLAY_SE SE_BALL_BOUNCE_1}ta-da!\p", "Jedna…{PAUSE 10}dva…{PAUSE 10}a…{PAUSE 10}{PAUSE 20}{PLAY_SE SE_BALL_BOUNCE_1}tadá!\p")),
    [STRINGID_ANDELLIPSIS]                          = COMPOUND_STRING(BATTLE_TEXT("And…\p", "A…\p")),
    [STRINGID_NOTVERYEFFECTIVE]                     = COMPOUND_STRING(BATTLE_TEXT("It's not very effective…", "Není to moc účinné…")),
    [STRINGID_SUPEREFFECTIVE]                       = COMPOUND_STRING(BATTLE_TEXT("It's super effective!", "Je to mimořádně účinné!")),
    [STRINGID_GOTAWAYSAFELY]                        = sText_GotAwaySafely,
    [STRINGID_WILDPKMNFLED]                         = COMPOUND_STRING(BATTLE_TEXT("{PLAY_SE SE_FLEE}The wild {B_BUFF1} fled!", "{PLAY_SE SE_FLEE}Divoký {B_BUFF1} utekl!")),
    [STRINGID_NORUNNINGFROMTRAINERS]                = COMPOUND_STRING(BATTLE_TEXT("No! There's no running from a Trainer battle!\p", "Ne! Z boje proti trenérovi nemůžeš utéct!\p")),
    [STRINGID_CANTESCAPE]                           = COMPOUND_STRING(BATTLE_TEXT("You can't escape!\p", "Nemůžeš utéct!\p")),
    [STRINGID_DONTLEAVEBIRCH]                       = COMPOUND_STRING(BATTLE_TEXT("PROF. BIRCH: Don't leave me like this!\p", "PROF. BIRCH: Nenechávej mě tu!\p")), //no decapitalize until it is everywhere
    [STRINGID_BUTNOTHINGHAPPENED]                   = COMPOUND_STRING(BATTLE_TEXT("But nothing happened!", "Ale nic se nestalo!")),
    [STRINGID_BUTITFAILED]                          = COMPOUND_STRING(BATTLE_TEXT("But it failed!", "Ale nepovedlo se to!")),
    [STRINGID_ITHURTCONFUSION]                      = COMPOUND_STRING(BATTLE_TEXT("It hurt itself in its confusion!", "Ve zmatení si sám ublížil!")),
    [STRINGID_STARTEDTORAIN]                        = COMPOUND_STRING(BATTLE_TEXT("It started to rain!", "Začalo pršet!")),
    [STRINGID_DOWNPOURSTARTED]                      = COMPOUND_STRING(BATTLE_TEXT("A downpour started!", "Spustil se liják!")), // corresponds to DownpourText in pokegold and pokecrystal and is used by Rain Dance in GSC
    [STRINGID_RAINCONTINUES]                        = COMPOUND_STRING(BATTLE_TEXT("Rain continues to fall.", "Déšť dál padá.")), //not in gen 5+
    [STRINGID_DOWNPOURCONTINUES]                    = COMPOUND_STRING(BATTLE_TEXT("The downpour continues.", "Liják pokračuje.")), // unused
    [STRINGID_RAINSTOPPED]                          = COMPOUND_STRING(BATTLE_TEXT("The rain stopped.", "Přestalo pršet.")),
    [STRINGID_SANDSTORMBREWED]                      = COMPOUND_STRING(BATTLE_TEXT("A sandstorm kicked up!", "Zvedla se písečná bouře!")),
    [STRINGID_SANDSTORMRAGES]                       = COMPOUND_STRING(BATTLE_TEXT("The sandstorm is raging.", "Písečná bouře zuří.")),
    [STRINGID_SANDSTORMSUBSIDED]                    = COMPOUND_STRING(BATTLE_TEXT("The sandstorm subsided.", "Písečná bouře ustala.")),
    [STRINGID_SUNLIGHTGOTBRIGHT]                    = COMPOUND_STRING(BATTLE_TEXT("The sunlight turned harsh!", "Slunce začalo prudce pálit!")),
    [STRINGID_SUNLIGHTSTRONG]                       = COMPOUND_STRING(BATTLE_TEXT("The sunlight is strong.", "Slunce prudce pálí.")), //not in gen 5+
    [STRINGID_SUNLIGHTFADED]                        = COMPOUND_STRING(BATTLE_TEXT("The sunlight faded.", "Slunce už tolik nepálí.")),
    [STRINGID_STARTEDHAIL]                          = COMPOUND_STRING(BATTLE_TEXT("It started to hail!", "Začaly padat kroupy!")),
    [STRINGID_HAILCONTINUES]                        = COMPOUND_STRING(BATTLE_TEXT("The hail is crashing down.", "Kroupy dál padají.")),
    [STRINGID_HAILSTOPPED]                          = COMPOUND_STRING(BATTLE_TEXT("The hail stopped.", "Kroupy přestaly padat.")),
    [STRINGID_STATCHANGESGONE]                      = COMPOUND_STRING(BATTLE_TEXT("All stat changes were eliminated!", "Všechny změny statistik byly zrušeny!")),
    [STRINGID_COINSSCATTERED]                       = COMPOUND_STRING(BATTLE_TEXT("Coins were scattered everywhere!", "Všude se rozsypaly mince!")),
    [STRINGID_TOOWEAKFORSUBSTITUTE]                 = COMPOUND_STRING(BATTLE_TEXT("But it does not have enough HP left to make a substitute!", "Na vytvoření náhradníka ale nezbývá dost HP!")),
    [STRINGID_SHAREDPAIN]                           = COMPOUND_STRING(BATTLE_TEXT("The battlers shared their pain!", "Bojovníci si rozdělili svou bolest!")),
    [STRINGID_BELLCHIMED]                           = COMPOUND_STRING(BATTLE_TEXT("A bell chimed!", "Zazvonil zvonek!")),
    [STRINGID_FAINTINTHREE]                         = COMPOUND_STRING(BATTLE_TEXT("All Pokémon that heard the song will faint in three turns!", "Všichni Pokémoni, kteří slyšeli píseň, za tři tahy omdlí!")),
    [STRINGID_NOPPLEFT]                             = COMPOUND_STRING(BATTLE_TEXT("There's no PP left for this move!\p", "Tomuto útoku nezbývají žádné PP!\p")), //not in gen 5+
    [STRINGID_BUTNOPPLEFT]                          = COMPOUND_STRING(BATTLE_TEXT("But there was no PP left for the move!", "Útoku ale nezbývaly žádné PP!")),
    [STRINGID_PLAYERUSEDITEM]                       = COMPOUND_STRING(BATTLE_TEXT("You used {B_LAST_ITEM}!", "Používáš {B_LAST_ITEM}!")),
    [STRINGID_TRAINERBLOCKEDBALL]                   = COMPOUND_STRING(BATTLE_TEXT("The Trainer blocked your Poké Ball!", "Trenér zablokoval tvůj Poké Ball!")),
    [STRINGID_DONTBEATHIEF]                         = COMPOUND_STRING(BATTLE_TEXT("Don't be a thief!", "Nekraď cizí Pokémony!")),
    [STRINGID_ITDODGEDBALL]                         = COMPOUND_STRING(BATTLE_TEXT("It dodged your thrown Poké Ball! This Pokémon can't be caught!", "Vyhnul se tvému Poké Ballu! Tohoto Pokémona nelze chytit!")),
    [STRINGID_PKMNBROKEFREE]                        = COMPOUND_STRING(BATTLE_TEXT("Oh no! The Pokémon broke free!", "Ale ne! Pokémon se dostal ven!")),
    [STRINGID_ITAPPEAREDCAUGHT]                     = COMPOUND_STRING(BATTLE_TEXT("Aww! It appeared to be caught!", "Škoda! Už to vypadalo, že je chycený!")),
    [STRINGID_AARGHALMOSTHADIT]                     = COMPOUND_STRING(BATTLE_TEXT("Aargh! Almost had it!", "Sakra! Už skoro byl tvůj!")),
    [STRINGID_SHOOTSOCLOSE]                         = COMPOUND_STRING(BATTLE_TEXT("Gah! It was so close, too!", "Ach jo! Bylo to tak těsné!")),
    [STRINGID_GOTCHAPKMNCAUGHTPLAYER]               = COMPOUND_STRING(BATTLE_TEXT("Gotcha! {B_DEF_NAME} was caught!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}\p", "A je to! {B_DEF_NAME} byl chycen!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}\p")),
    [STRINGID_GOTCHAPKMNCAUGHTWALLY]                = COMPOUND_STRING(BATTLE_TEXT("Gotcha! {B_DEF_NAME} was caught!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}{PAUSE 127}", "A je to! {B_DEF_NAME} byl chycen!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}{PAUSE 127}")),
    [STRINGID_GIVENICKNAMECAPTURED]                 = COMPOUND_STRING(BATTLE_TEXT("Would you like to give {B_DEF_NAME} a nickname?", "{B_DEF_NAME}: chceš tomuto Pokémonovi dát přezdívku?")),
    [STRINGID_PKMNDATAADDEDTODEX]                   = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME}'s data has been added to the Pokédex!\p", "{B_DEF_NAME}: údaje byly přidány do Pokédexu!\p")),
    [STRINGID_ITISRAINING]                          = COMPOUND_STRING(BATTLE_TEXT("It's raining!", "Prší!")),
    [STRINGID_SANDSTORMISRAGING]                    = COMPOUND_STRING(BATTLE_TEXT("The sandstorm is raging!", "Písečná bouře zuří!")),
    [STRINGID_CANTESCAPE2]                          = COMPOUND_STRING(BATTLE_TEXT("You couldn't get away!\p", "Útěk se nezdařil!\p")),
    [STRINGID_PKMNIGNORESASLEEP]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} ignored orders and kept sleeping!", "{B_ATK_NAME_WITH_PREFIX} ignoroval rozkazy a spal dál!")),
    [STRINGID_PKMNIGNOREDORDERS]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} ignored orders!", "{B_ATK_NAME_WITH_PREFIX} ignoroval rozkazy!")),
    [STRINGID_PKMNBEGANTONAP]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} began to nap!", "{B_ATK_NAME_WITH_PREFIX} začal podřimovat!")),
    [STRINGID_PKMNLOAFING]                          = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is loafing around!", "{B_ATK_NAME_WITH_PREFIX} se jen tak poflakuje!")),
    [STRINGID_PKMNWONTOBEY]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} won't obey!", "{B_ATK_NAME_WITH_PREFIX} odmítá poslouchat!")),
    [STRINGID_PKMNTURNEDAWAY]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} turned away!", "{B_ATK_NAME_WITH_PREFIX} se odvrátil!")),
    [STRINGID_PKMNPRETENDNOTNOTICE]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} pretended not to notice!", "{B_ATK_NAME_WITH_PREFIX} dělal, že nic neslyší!")),
    [STRINGID_ENEMYABOUTTOSWITCHPKMN]               = COMPOUND_STRING(BATTLE_TEXT("{B_TRAINER1_NAME_WITH_CLASS} is about to send out {B_BUFF2}. Will you switch your Pokémon?", "{B_TRAINER1_NAME_WITH_CLASS} se chystá vyslat Pokémona {B_BUFF2}. Chceš vyměnit svého Pokémona?")),
    [STRINGID_CREPTCLOSER]                          = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_NAME} crept closer to {B_OPPONENT_MON1_NAME}!", "{B_PLAYER_NAME} se plíží blíž! Cíl: {B_OPPONENT_MON1_NAME}!")), //safari
    [STRINGID_CANTGETCLOSER]                        = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_NAME} can't get any closer!", "{B_PLAYER_NAME} se už nemůže přiblížit!")), //safari
    [STRINGID_PKMNWATCHINGCAREFULLY]                = COMPOUND_STRING(BATTLE_TEXT("{B_OPPONENT_MON1_NAME} is watching carefully!", "{B_OPPONENT_MON1_NAME} pozorně sleduje okolí!")), //safari
    [STRINGID_PKMNCURIOUSABOUTX]                    = COMPOUND_STRING(BATTLE_TEXT("{B_OPPONENT_MON1_NAME} is curious about the {B_BUFF1}!", "{B_OPPONENT_MON1_NAME} je zvědavý na {B_BUFF1}!")), //safari
    [STRINGID_PKMNENTHRALLEDBYX]                    = COMPOUND_STRING(BATTLE_TEXT("{B_OPPONENT_MON1_NAME} is enthralled by the {B_BUFF1}!", "{B_OPPONENT_MON1_NAME} je nadšený z {B_BUFF1}!")), //safari
    [STRINGID_PKMNIGNOREDX]                         = COMPOUND_STRING(BATTLE_TEXT("{B_OPPONENT_MON1_NAME} completely ignored the {B_BUFF1}!", "{B_OPPONENT_MON1_NAME} úplně ignoroval {B_BUFF1}!")), //safari
    [STRINGID_THREWPOKEBLOCKATPKMN]                 = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_NAME} threw a {POKEBLOCK} at the {B_OPPONENT_MON1_NAME}!", "{B_PLAYER_NAME} hází {POKEBLOCK}! Cíl: {B_OPPONENT_MON1_NAME}!")), //safari
    [STRINGID_OUTOFSAFARIBALLS]                     = COMPOUND_STRING(BATTLE_TEXT("{PLAY_SE SE_DING_DONG}ANNOUNCER: You're out of Safari Balls! Game over!\p", "{PLAY_SE SE_DING_DONG}HLASATEL: Došly ti Safari Bally! Hra končí!\p")), //safari
    [STRINGID_PKMNSITEMCUREDPARALYSIS]              = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} cured its paralysis!", "{B_SCR_NAME_WITH_PREFIX} se pomocí {B_LAST_ITEM} zbavil ochrnutí!")),
    [STRINGID_PKMNSITEMCUREDPOISON]                 = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} cured its poison!", "{B_SCR_NAME_WITH_PREFIX} se pomocí {B_LAST_ITEM} zbavil otravy!")),
    [STRINGID_PKMNSITEMHEALEDBURN]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} cured its burn!", "{B_SCR_NAME_WITH_PREFIX} se pomocí {B_LAST_ITEM} zbavil popálení!")),
    [STRINGID_PKMNSITEMDEFROSTEDIT]                 = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} defrosted it!", "{B_SCR_NAME_WITH_PREFIX} pomocí {B_LAST_ITEM} rozmrzl!")),
    [STRINGID_PKMNSITEMWOKEIT]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} woke it up!", "{B_SCR_NAME_WITH_PREFIX} se pomocí {B_LAST_ITEM} probudil!")),
    [STRINGID_PKMNSITEMSNAPPEDOUT]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} snapped it out of its confusion!", "{B_SCR_NAME_WITH_PREFIX} se pomocí {B_LAST_ITEM} zbavil zmatení!")),
    [STRINGID_PKMNSITEMCUREDPROBLEM]                = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} cured its {B_BUFF1} problem!", "{B_SCR_NAME_WITH_PREFIX} se pomocí {B_LAST_ITEM} zbavil potíží: {B_BUFF1}!")), // Not in Gen 5+
    [STRINGID_PKMNSITEMRESTOREDHEALTH]              = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} restored its health using its {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX} si obnovil zdraví pomocí {B_LAST_ITEM}!")),
    [STRINGID_PKMNSITEMRESTOREDPP]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} restored PP to its move {B_BUFF1} using its {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX} si obnovil PP útoku {B_BUFF1} pomocí {B_LAST_ITEM}!")),
    [STRINGID_PKMNSITEMRESTOREDSTATUS]              = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} returned its stats to normal using its {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX} si pomocí {B_LAST_ITEM} vrátil statistiky do normálu!")),
    [STRINGID_PKMNSITEMRESTOREDHPALITTLE]           = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} restored a little HP using its {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX} si obnovil trochu HP pomocí {B_LAST_ITEM}!")),
    [STRINGID_ITEMALLOWSONLYYMOVE]                  = COMPOUND_STRING(BATTLE_TEXT("{B_LAST_ITEM} only allows the use of {B_CURRENT_MOVE}!\p", "{B_LAST_ITEM} dovoluje použít jen {B_CURRENT_MOVE}!\p")),
    [STRINGID_PKMNHUNGONWITHX]                      = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} hung on using its {B_LAST_ITEM}!", "{B_DEF_NAME_WITH_PREFIX} vydržel díky {B_LAST_ITEM}!")),
    [STRINGID_EMPTYSTRING3]                         = gText_EmptyString3,
    [STRINGID_PKMNSXRESTOREDHPALITTLE2]             = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} had its HP restored.", "{B_ATK_NAME_WITH_PREFIX} si obnovil HP.")),
    [STRINGID_PKMNSXWHIPPEDUPSANDSTORM]             = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} whipped up a sandstorm!", "{B_SCR_NAME_WITH_PREFIX} schopností {B_SCR_ABILITY} rozpoutal písečnou bouři!")), //not in gen 5+, ability popup
    [STRINGID_PKMNSXPREVENTSYLOSS]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} was not lowered!", "{B_SCR_NAME_WITH_PREFIX}: statistika {B_BUFF1} se nesnížila!")),
    [STRINGID_PKMNSXINFATUATEDY]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} fell in love!", "{B_ATK_NAME_WITH_PREFIX} se zamiloval!")),
    [STRINGID_PKMNSXMADEYINEFFECTIVE]               = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s item cannot be removed!", "{B_SCR_NAME_WITH_PREFIX} nemůže přijít o svůj předmět!")),
    [STRINGID_ITSUCKEDLIQUIDOOZE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} sucked up the liquid ooze!", "{B_EFF_NAME_WITH_PREFIX} nasál tekutý sliz!")),
    [STRINGID_PKMNTRANSFORMED]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} transformed!", "{B_SCR_NAME_WITH_PREFIX} se proměnil!")),
    [STRINGID_ELECTRICITYWEAKENED]                  = COMPOUND_STRING(BATTLE_TEXT("Electricity's power was weakened!", "Síla elektrických útoků zeslábla!")),
    [STRINGID_FIREWEAKENED]                         = COMPOUND_STRING(BATTLE_TEXT("Fire's power was weakened!", "Síla ohnivých útoků zeslábla!")),
    [STRINGID_PKMNHIDUNDERWATER]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} hid underwater!", "{B_ATK_NAME_WITH_PREFIX} se schoval pod vodu!")),
    [STRINGID_PKMNSPRANGUP]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} sprang up!", "{B_ATK_NAME_WITH_PREFIX} vyskočil do výšky!")),
    [STRINGID_HMMOVESCANTBEFORGOTTEN]               = COMPOUND_STRING(BATTLE_TEXT("HM moves can't be forgotten now.\p", "Útoky HM teď nelze zapomenout.\p")),
    [STRINGID_XFOUNDONEY]                           = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} found one {B_LAST_ITEM}!", "{B_ATK_NAME_WITH_PREFIX} našel jeden předmět {B_LAST_ITEM}!")),
    [STRINGID_PLAYERDEFEATEDTRAINER1]               = sText_PlayerDefeatedLinkTrainerTrainer1,
    [STRINGID_SOOTHINGAROMA]                        = COMPOUND_STRING(BATTLE_TEXT("A soothing aroma wafted through the area!", "Okolím se rozlinula uklidňující vůně!")),
    [STRINGID_ITEMSCANTBEUSEDNOW]                   = COMPOUND_STRING(BATTLE_TEXT("Items can't be used now.{PAUSE 64}", "Předměty teď nelze používat.{PAUSE 64}")), // Not present in Gen 5+
    [STRINGID_USINGITEMSTATOFPKMNROSE]              = COMPOUND_STRING(BATTLE_TEXT("The {B_LAST_ITEM}{B_BUFF2} boosted {B_SCR_NAME_WITH_PREFIX2}'s {B_BUFF1}!", "Předmět {B_LAST_ITEM} zvýšil{B_BUFF2} statistiku {B_BUFF1}! Posílený je {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_USINGITEMSTATOFPKMNFELL]              = COMPOUND_STRING(BATTLE_TEXT("The {B_LAST_ITEM}{B_BUFF2} lowered {B_SCR_NAME_WITH_PREFIX2}'s {B_BUFF1}!", "Předmět {B_LAST_ITEM} {B_BUFF2}snížil statistiku {B_BUFF1}! Oslabený je {B_SCR_NAME_WITH_PREFIX2}!")), // This string does not exist in Gen 5+. Used to print more info that's otherwise obscured such as using Room Service
    [STRINGID_PKMNUSEDXTOGETPUMPED]                 = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} used the {B_LAST_ITEM} to get pumped!", "{B_SCR_NAME_WITH_PREFIX} se pomocí {B_LAST_ITEM} nabudil!")),
    [STRINGID_PKMNSXMADEYUSELESS]                   = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} made {B_CURRENT_MOVE} useless!", "{B_SCR_NAME_WITH_PREFIX} schopností {B_SCR_ABILITY} zneškodnil útok {B_CURRENT_MOVE}!")), //not in gen 5+, ability popup
    [STRINGID_PKMNTRAPPEDBYSANDTOMB]                = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} became trapped by the quicksand!", "{B_EFF_NAME_WITH_PREFIX} uvízl v tekutém písku!")),
    [STRINGID_EMPTYSTRING4]                         = COMPOUND_STRING(""),
    [STRINGID_ABOOSTED]                             = COMPOUND_STRING(BATTLE_TEXT(" a boosted", " navíc")),
    [STRINGID_PKMNSXINTENSIFIEDSUN]                 = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} intensified the sun's rays!", "{B_SCR_NAME_WITH_PREFIX} schopností {B_SCR_ABILITY} zesílil sluneční svit!")), //not in gen 5+, ability popup
    [STRINGID_YOUTHROWABALLNOWRIGHT]                = COMPOUND_STRING(BATTLE_TEXT("You throw a Ball now, right? I… I'll do my best!", "Teď mám hodit Ball, že? Já… Udělám, co bude v mých silách!")),
    [STRINGID_PKMNSXTOOKATTACK]                     = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} took the attack!", "{B_DEF_NAME_WITH_PREFIX} zachytil útok!")),
    [STRINGID_PKMNCHOSEXASDESTINY]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} chose Doom Desire as its destiny!", "{B_ATK_NAME_WITH_PREFIX} si zvolil Doom Desire jako svůj osud!")),
    [STRINGID_PKMNLOSTFOCUS]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} lost its focus and couldn't move!", "{B_ATK_NAME_WITH_PREFIX} se přestal soustředit a nemohl se pohnout!")),
    [STRINGID_USENEXTPKMN]                          = COMPOUND_STRING(BATTLE_TEXT("Use next Pokémon?", "Nasadit dalšího Pokémona?")),
    [STRINGID_PKMNFLEDUSINGITS]                     = COMPOUND_STRING(BATTLE_TEXT("{PLAY_SE SE_FLEE}{B_ATK_NAME_WITH_PREFIX} fled using its {B_LAST_ITEM}!\p", "{PLAY_SE SE_FLEE}{B_ATK_NAME_WITH_PREFIX} utekl díky předmětu {B_LAST_ITEM}!\p")),
    [STRINGID_PKMNFLEDUSING]                        = COMPOUND_STRING(BATTLE_TEXT("{PLAY_SE SE_FLEE}{B_ATK_NAME_WITH_PREFIX} fled using {B_ATK_ABILITY}!\p", "{PLAY_SE SE_FLEE}{B_ATK_NAME_WITH_PREFIX} utekl díky schopnosti {B_ATK_ABILITY}!\p")), //not in gen 5+
    [STRINGID_PKMNWASDRAGGEDOUT]                    = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} was dragged out!\p", "{B_DEF_NAME_WITH_PREFIX} byl vytažen do boje!\p")),
    [STRINGID_PKMNSITEMNORMALIZEDSTATUS]            = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} normalized its status!", "{B_SCR_NAME_WITH_PREFIX}: předmět {B_LAST_ITEM} vyléčil jeho stav!")), // Not in Gen 5+
    [STRINGID_TRAINER1USEDITEM]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_TRAINER_NAME_WITH_CLASS} used {B_LAST_ITEM}!", "{B_ATK_TRAINER_NAME_WITH_CLASS} použil předmět {B_LAST_ITEM}!")),
    [STRINGID_BOXISFULL]                            = COMPOUND_STRING(BATTLE_TEXT("The Box is full! You can't catch any more!\p", "Box je plný! Další Pokémony už nelze chytit!\p")),
    [STRINGID_PKMNSXMADEITINEFFECTIVE]              = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} made it ineffective!", "{B_SCR_NAME_WITH_PREFIX} zmařil účinek útoku!")),
    [STRINGID_PKMNSXPREVENTSFLINCHING]              = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX}'s {B_EFF_ABILITY} prevents flinching!", "{B_EFF_NAME_WITH_PREFIX}: schopnost {B_EFF_ABILITY} brání zaváhání!")), //not in gen 5+, ability popup
    [STRINGID_PKMNALREADYHASBURN]                   = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} is already burned!", "{B_DEF_NAME_WITH_PREFIX} už je popálený!")),
    [STRINGID_PKMNSXBLOCKSY]                        = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} blocks {B_CURRENT_MOVE}!", "{B_SCR_NAME_WITH_PREFIX}: schopnost {B_SCR_ABILITY} blokuje útok {B_CURRENT_MOVE}!")), //not in gen 5+, ability popup
    [STRINGID_PKMNSXWOREOFF]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_TEAM1} side's {B_BUFF1} wore off!", "{B_ATK_TEAM1} tým už nechrání účinek útoku {B_BUFF1}!")),
    [STRINGID_THEWALLSHATTERED]                     = COMPOUND_STRING(BATTLE_TEXT("The wall shattered!", "Stěna se roztříštila!")), //not in gen5+, uses "your teams light screen wore off!" etc instead
    [STRINGID_PKMNSXCUREDITSYPROBLEM]               = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} cured its {B_BUFF1} problem!", "{B_SCR_NAME_WITH_PREFIX}: schopnost {B_SCR_ABILITY} vyléčila stav {B_BUFF1}!")), //not in gen 5+, ability popup
    [STRINGID_ATTACKERCANTESCAPE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} can't escape!", "{B_ATK_NAME_WITH_PREFIX} nemůže utéct!")),
    [STRINGID_PKMNOBTAINEDX]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} obtained {B_BUFF1}.", "{B_ATK_NAME_WITH_PREFIX} získal předmět {B_BUFF1}.")),
    [STRINGID_PKMNOBTAINEDX2]                       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} obtained {B_BUFF2}.", "{B_DEF_NAME_WITH_PREFIX} získal předmět {B_BUFF2}.")),
    [STRINGID_PKMNOBTAINEDXYOBTAINEDZ]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} obtained {B_BUFF1}.\p{B_DEF_NAME_WITH_PREFIX} obtained {B_BUFF2}.", "{B_ATK_NAME_WITH_PREFIX} získal předmět {B_BUFF1}.\p{B_DEF_NAME_WITH_PREFIX} získal předmět {B_BUFF2}.")),
    [STRINGID_BUTNOEFFECT]                          = COMPOUND_STRING(BATTLE_TEXT("But it had no effect!", "Ale nemělo to žádný účinek!")),
    [STRINGID_TWOENEMIESDEFEATED]                   = sText_TwoInGameTrainersDefeated,
    [STRINGID_TRAINER2LOSETEXT]                     = COMPOUND_STRING("{B_TRAINER2_LOSE_TEXT}"),
    [STRINGID_PKMNINCAPABLEOFPOWER]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} appears incapable of using its power!", "{B_ATK_NAME_WITH_PREFIX} zřejmě nedokáže použít svou sílu!")),
    [STRINGID_GLINTAPPEARSINEYE]                    = COMPOUND_STRING(BATTLE_TEXT("A glint appears in {B_SCR_NAME_WITH_PREFIX2}'s eyes!", "V očích se zablesklo! Je připravený: {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNGETTINGINTOPOSITION]              = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is getting into position!", "{B_SCR_NAME_WITH_PREFIX} zaujímá postoj!")),
    [STRINGID_PKMNBEGANGROWLINGDEEPLY]              = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} began growling deeply!", "{B_SCR_NAME_WITH_PREFIX} začal hluboce vrčet!")),
    [STRINGID_PKMNEAGERFORMORE]                     = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is eager for more!", "{B_SCR_NAME_WITH_PREFIX} chce bojovat dál!")),
    [STRINGID_DEFEATEDOPPONENTBYREFEREE]            = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_MON1_NAME} defeated the opponent {B_OPPONENT_MON1_NAME} in a REFEREE's decision!", "Rozhodčí rozhodl: {B_PLAYER_MON1_NAME} vítězí, {B_OPPONENT_MON1_NAME} prohrává!")),
    [STRINGID_LOSTTOOPPONENTBYREFEREE]              = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_MON1_NAME} lost to the opponent {B_OPPONENT_MON1_NAME} in a REFEREE's decision!", "Rozhodčí rozhodl: {B_PLAYER_MON1_NAME} prohrává, {B_OPPONENT_MON1_NAME} vítězí!")),
    [STRINGID_TIEDOPPONENTBYREFEREE]                = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_MON1_NAME} tied the opponent {B_OPPONENT_MON1_NAME} in a REFEREE's decision!", "Rozhodčí rozhodl: {B_PLAYER_MON1_NAME} a {B_OPPONENT_MON1_NAME} remizují!")),
    [STRINGID_QUESTIONFORFEITMATCH]                 = COMPOUND_STRING(BATTLE_TEXT("Would you like to forfeit the match and quit now?", "Chceš vzdát zápas a skončit?")),
    [STRINGID_FORFEITEDMATCH]                       = COMPOUND_STRING(BATTLE_TEXT("The match was forfeited.", "Zápas byl vzdán.")),
    [STRINGID_PKMNTRANSFERREDSOMEONESPC]            = gText_PkmnTransferredSomeonesPC,
    [STRINGID_PKMNTRANSFERREDLANETTESPC]            = gText_PkmnTransferredLanettesPC,
    [STRINGID_PKMNBOXSOMEONESPCFULL]                = gText_PkmnTransferredSomeonesPCBoxFull,
    [STRINGID_PKMNBOXLANETTESPCFULL]                = gText_PkmnTransferredLanettesPCBoxFull,
    [STRINGID_TRAINER1WINTEXT]                      = COMPOUND_STRING("{B_TRAINER1_WIN_TEXT}"),
    [STRINGID_TRAINER2WINTEXT]                      = COMPOUND_STRING("{B_TRAINER2_WIN_TEXT}"),
    [STRINGID_ENDUREDSTURDY]                        = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} endured the hit using {B_DEF_ABILITY}!", "{B_DEF_NAME_WITH_PREFIX} vydržel zásah díky schopnosti {B_DEF_ABILITY}!")),
    [STRINGID_POWERHERB]                            = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} became fully charged due to its {B_LAST_ITEM}!", "{B_ATK_NAME_WITH_PREFIX} se plně nabil díky předmětu {B_LAST_ITEM}!")),
    [STRINGID_HURTBYITEM]                           = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was hurt by the {B_LAST_ITEM}!", "{B_ATK_NAME_WITH_PREFIX} utrpěl zranění kvůli předmětu {B_LAST_ITEM}!")),
    [STRINGID_GRAVITYINTENSIFIED]                   = COMPOUND_STRING(BATTLE_TEXT("Gravity intensified!", "Gravitace zesílila!")),
    [STRINGID_TARGETWOKEUP]                         = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} woke up!", "{B_EFF_NAME_WITH_PREFIX} se probudil!")),
    [STRINGID_TAILWINDBLEW]                         = COMPOUND_STRING(BATTLE_TEXT("A tailwind started blowing on {B_ATK_TEAM2} side!", "Teď {B_ATK_TEAM2} tým podporuje vítr v zádech!")),
    [STRINGID_PKMNWENTBACK]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} went back to {B_ATK_TRAINER_NAME}!", "{B_ATK_NAME_WITH_PREFIX} se vrací! Jeho trenér: {B_ATK_TRAINER_NAME}!")),
    [STRINGID_PKMNCANTUSEITEMSANYMORE]              = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} can't use items anymore!", "{B_DEF_NAME_WITH_PREFIX} už nemůže používat předměty!")),
    [STRINGID_PKMNFLUNG]                            = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} flung its {B_LAST_ITEM}!", "{B_EFF_NAME_WITH_PREFIX} mrštil předmětem {B_LAST_ITEM}!")),
    [STRINGID_PKMNPREVENTEDFROMHEALING]             = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} was prevented from healing!", "{B_DEF_NAME_WITH_PREFIX} se nemůže léčit!")),
    [STRINGID_PKMNSWITCHEDATKANDDEF]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} switched its Attack and Defense!", "{B_ATK_NAME_WITH_PREFIX} si prohodil Útok a Obranu!")),
    [STRINGID_PKMNSABILITYSUPPRESSED]               = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s Ability was suppressed!", "{B_DEF_NAME_WITH_PREFIX}: jeho schopnost byla potlačena!")),
    [STRINGID_SHIELDEDFROMCRITICALHITS]             = COMPOUND_STRING(BATTLE_TEXT("Lucky Chant shielded {B_ATK_TEAM2} side from critical hits!", "Lucky Chant chrání {B_ATK_TEAM2} tým před kritickými zásahy!")), // Currently not present in Champions
    [STRINGID_PKMNACQUIREDABILITY]                  = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} acquired {B_DEF_ABILITY}!", "{B_DEF_NAME_WITH_PREFIX} získal schopnost {B_DEF_ABILITY}!")),
    [STRINGID_POISONSPIKESSCATTERED]                = COMPOUND_STRING(BATTLE_TEXT("Toxic spikes were scattered on the ground all around {B_DEF_TEAM2} side!", "Jedovaté bodce teď obklopují {B_DEF_TEAM2} tým!")),
    [STRINGID_PKMNSWITCHEDSTATCHANGES]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} switched stat changes with its target!", "{B_ATK_NAME_WITH_PREFIX} si s cílem prohodil změny statistik!")),
    [STRINGID_PKMNSURROUNDEDWITHVEILOFWATER]        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} surrounded itself with a veil of water!", "{B_ATK_NAME_WITH_PREFIX} se zahalil vodním závojem!")),
    [STRINGID_PKMNLEVITATEDONELECTROMAGNETISM]      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} levitated with electromagnetism!", "{B_ATK_NAME_WITH_PREFIX} se vznáší díky elektromagnetismu!")),
    [STRINGID_PKMNTWISTEDDIMENSIONS]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} twisted the dimensions!", "{B_ATK_NAME_WITH_PREFIX} pokřivil prostor!")),
    [STRINGID_POINTEDSTONESFLOAT]                   = COMPOUND_STRING(BATTLE_TEXT("Pointed stones float in the air on {B_DEF_TEAM2} side!", "Ostré kameny teď obklopují {B_DEF_TEAM2} tým!")),
    [STRINGID_TRAPPEDBYSWIRLINGMAGMA]               = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} became trapped by swirling magma!", "{B_EFF_NAME_WITH_PREFIX} uvízl ve vířícím magmatu!")),
    [STRINGID_VANISHEDINSTANTLY]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} vanished instantly!", "{B_ATK_NAME_WITH_PREFIX} okamžitě zmizel!")),
    [STRINGID_PROTECTEDTEAM]                        = COMPOUND_STRING(BATTLE_TEXT("{B_CURRENT_MOVE} now protects {B_ATK_TEAM2} side!", "{B_CURRENT_MOVE} teď chrání {B_ATK_TEAM2} tým!")),
    [STRINGID_SHAREDITSGUARD]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} shared its guard with the target!", "{B_ATK_NAME_WITH_PREFIX} se s cílem podělil o obranu!")),
    [STRINGID_SHAREDITSPOWER]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} shared its power with the target!", "{B_ATK_NAME_WITH_PREFIX} se s cílem podělil o sílu!")),
    [STRINGID_SWAPSDEFANDSPDEFOFALLPOKEMON]         = COMPOUND_STRING(BATTLE_TEXT("It created a bizarre area in which Defense and Sp. Def stats are swapped!", "Vznikl podivný prostor, ve kterém se prohodily Obrana a Spec. obrana!")),
    [STRINGID_BECAMENIMBLE]                         = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} became nimble!", "{B_EFF_NAME_WITH_PREFIX} získal na obratnosti!")),
    [STRINGID_HURLEDINTOTHEAIR]                     = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} was hurled into the air!", "{B_DEF_NAME_WITH_PREFIX} byl vymrštěn do vzduchu!")),
    [STRINGID_HELDITEMSLOSEEFFECTS]                 = COMPOUND_STRING(BATTLE_TEXT("It created a bizarre area in which Pokémon's held items lose their effects!", "Vznikl podivný prostor, ve kterém držené předměty Pokémonů ztrácejí účinky!")),
    [STRINGID_FELLSTRAIGHTDOWN]                     = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} fell straight down!", "{B_EFF_NAME_WITH_PREFIX} spadl přímo dolů!")),
    [STRINGID_TARGETCHANGEDTYPE]                    = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} transformed into the {B_BUFF1} type!", "{B_DEF_NAME_WITH_PREFIX} získal typ {B_BUFF1}!")),
    [STRINGID_KINDOFFER]                            = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} took the kind offer!", "{B_DEF_NAME_WITH_PREFIX} přijal laskavou nabídku!")),
    [STRINGID_RESETSTARGETSSTATLEVELS]              = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX}'s stat changes were removed!", "{B_EFF_NAME_WITH_PREFIX}: změny statistik byly zrušeny!")),
    [STRINGID_ALLYSWITCHPOSITION]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} and {B_SCR_NAME_WITH_PREFIX2} switched places!", "{B_ATK_NAME_WITH_PREFIX} a {B_SCR_NAME_WITH_PREFIX2} si vyměnili místa!")),
    [STRINGID_REFLECTTARGETSTYPE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} became the same type as {B_DEF_NAME_WITH_PREFIX2}!", "{B_ATK_NAME_WITH_PREFIX} získal stejný typ jako {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_EMBARGOENDS]                          = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} can use items again!", "{B_ATK_NAME_WITH_PREFIX} může znovu používat předměty!")),
    [STRINGID_ELECTROMAGNETISM]                     = COMPOUND_STRING(BATTLE_TEXT("electromagnetism", "elektromagnetismus")),
    [STRINGID_BUFFERENDS]                           = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} wore off!", "{B_SCR_NAME_WITH_PREFIX}: {B_BUFF1} už nepůsobí!")),
    [STRINGID_TELEKINESISENDS]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was freed from the telekinesis!", "{B_ATK_NAME_WITH_PREFIX} se osvobodil z telekineze!")),
    [STRINGID_TAILWINDENDS]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_TEAM1} side's tailwind petered out!", "{B_ATK_TEAM1} tým už nepodporuje vítr v zádech!")),
    [STRINGID_LUCKYCHANTENDS]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_TEAM1} side's Lucky Chant wore off!", "{B_ATK_TEAM1} tým už nechrání Lucky Chant!")),
    [STRINGID_TRICKROOMENDS]                        = COMPOUND_STRING(BATTLE_TEXT("The twisted dimensions returned to normal!", "Pokřivený prostor se vrátil do normálu!")),
    [STRINGID_WONDERROOMENDS]                       = COMPOUND_STRING(BATTLE_TEXT("Wonder Room wore off, and Defense and Sp. Def stats returned to normal!", "Účinek útoku Wonder Room pominul. Obrana a Spec. obrana se vrátily do normálu!")),
    [STRINGID_MAGICROOMENDS]                        = COMPOUND_STRING(BATTLE_TEXT("Magic Room wore off, and held items' effects returned to normal!", "Účinek útoku Magic Room pominul. Držené předměty znovu fungují!")),
    [STRINGID_MUDSPORTENDS]                         = COMPOUND_STRING(BATTLE_TEXT("The effects of Mud Sport have faded.", "Účinky útoku Mud Sport pominuly.")),
    [STRINGID_WATERSPORTENDS]                       = COMPOUND_STRING(BATTLE_TEXT("The effects of Water Sport have faded.", "Účinky útoku Water Sport pominuly.")),
    [STRINGID_GRAVITYENDS]                          = COMPOUND_STRING(BATTLE_TEXT("Gravity returned to normal!", "Gravitace se vrátila do normálu!")),
    [STRINGID_AQUARINGHEAL]                         = COMPOUND_STRING(BATTLE_TEXT("A veil of water restored {B_ATK_NAME_WITH_PREFIX2}'s HP!", "Vodní závoj obnovil HP! Zotavuje se {B_ATK_NAME_WITH_PREFIX2}!")),
    [STRINGID_ELECTRICTERRAINENDS]                  = COMPOUND_STRING(BATTLE_TEXT("The electricity disappeared from the battlefield.", "Elektřina zmizela z bojiště.")),
    [STRINGID_MISTYTERRAINENDS]                     = COMPOUND_STRING(BATTLE_TEXT("The mist disappeared from the battlefield.", "Mlha zmizela z bojiště.")),
    [STRINGID_PSYCHICTERRAINENDS]                   = COMPOUND_STRING(BATTLE_TEXT("The weirdness disappeared from the battlefield!", "Podivná síla zmizela z bojiště!")),
    [STRINGID_GRASSYTERRAINENDS]                    = COMPOUND_STRING(BATTLE_TEXT("The grass disappeared from the battlefield.", "Tráva zmizela z bojiště.")),
    [STRINGID_TARGETABILITYSTATRAISE]               = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY} {B_BUFF2}raised its {B_BUFF1}!", "{B_DEF_NAME_WITH_PREFIX}: schopnost {B_DEF_ABILITY}{B_BUFF2} zvýšila statistiku {B_BUFF1}!")), // Not in Gen 5+
    [STRINGID_STATWASMAXEDOUT]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} maxed its {B_BUFF1}!", "{B_SCR_NAME_WITH_PREFIX}: statistika {B_BUFF1} dosáhla maxima!")),
    [STRINGID_ATTACKERABILITYSTATRAISE]             = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s {B_ATK_ABILITY} {B_BUFF2}raised its {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX}: schopnost {B_ATK_ABILITY}{B_BUFF2} zvýšila statistiku {B_BUFF1}!")), // Not in Gen 5+
    [STRINGID_POISONHEALHPUP]                       = COMPOUND_STRING(BATTLE_TEXT("The poisoning healed {B_ATK_NAME_WITH_PREFIX2} a little bit!", "Otrava obnovila trochu HP! Zotavuje se {B_ATK_NAME_WITH_PREFIX2}!")), // Not in Gen 5+
    [STRINGID_BADDREAMSDMG]                         = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} is tormented!", "{B_DEF_NAME_WITH_PREFIX} je sužován nočními můrami!")),
    [STRINGID_MOLDBREAKERENTERS]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} breaks the mold!", "{B_SCR_NAME_WITH_PREFIX} boří zaběhlé pořádky!")),
    [STRINGID_TERAVOLTENTERS]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is radiating a bursting aura!", "{B_SCR_NAME_WITH_PREFIX} vyzařuje praskající auru!")),
    [STRINGID_TURBOBLAZEENTERS]                     = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is radiating a blazing aura!", "{B_SCR_NAME_WITH_PREFIX} vyzařuje žhnoucí auru!")),
    [STRINGID_SLOWSTARTENTERS]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is slow to get going!", "{B_SCR_NAME_WITH_PREFIX} se rozjíždí pomalu!")),
    [STRINGID_SLOWSTARTEND]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} finally got its act together!", "{B_ATK_NAME_WITH_PREFIX} se konečně vzchopil!")),
    [STRINGID_SOLARPOWERHPDROP]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s {B_ATK_ABILITY} takes its toll!", "{B_ATK_NAME_WITH_PREFIX}: schopnost {B_ATK_ABILITY} si vybírá svou daň!")), // Not in Gen 5+
    [STRINGID_PKMNWASHURT]                          = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was hurt!", "{B_ATK_NAME_WITH_PREFIX} utrpěl zranění!")),
    [STRINGID_ANTICIPATIONACTIVATES]                = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} shuddered!", "{B_SCR_NAME_WITH_PREFIX} se otřásl!")),
    [STRINGID_FOREWARNACTIVATES]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_ABILITY} alerted {B_SCR_NAME_WITH_PREFIX2} to {B_EFF_NAME_WITH_PREFIX2}'s {B_BUFF1}!", "{B_SCR_ABILITY} varuje! {B_SCR_NAME_WITH_PREFIX2} ví, že {B_EFF_NAME_WITH_PREFIX2} zná útok {B_BUFF1}!")),
    [STRINGID_ICEBODYHPGAIN]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s {B_ATK_ABILITY} healed it a little bit!", "{B_ATK_NAME_WITH_PREFIX}: schopnost {B_ATK_ABILITY} obnovila trochu HP!")), // Not in Gen 5+
    [STRINGID_SNOWWARNINGHAIL]                      = COMPOUND_STRING(BATTLE_TEXT("It started to hail!", "Začalo krupobití!")),
    [STRINGID_FRISKACTIVATES]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} was frisked, revealing its {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX}: prohlídka odhalila předmět {B_LAST_ITEM}!")),
    [STRINGID_UNNERVEENTERS]                        = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_TEAM1} side is too nervous to eat Berries!", "{B_EFF_TEAM1} tým je příliš nervózní, než aby jedl bobule!")),
    [STRINGID_HARVESTBERRY]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} harvested its {B_LAST_ITEM}!", "{B_ATK_NAME_WITH_PREFIX} sklidil předmět {B_LAST_ITEM}!")),
    [STRINGID_PROTEANTYPECHANGE]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s {B_ATK_ABILITY} transformed it into the {B_BUFF1} type!", "{B_ATK_NAME_WITH_PREFIX}: schopnost {B_ATK_ABILITY} změnila jeho typ na {B_BUFF1}!")),
    [STRINGID_SYMBIOSISITEMPASS]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} shared its {B_LAST_ITEM} with {B_EFF_NAME_WITH_PREFIX2}!", "{B_SCR_NAME_WITH_PREFIX} předal předmět {B_LAST_ITEM}! Získal ho {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_STEALTHROCKDMG]                       = COMPOUND_STRING(BATTLE_TEXT("Pointed stones dug into {B_SCR_NAME_WITH_PREFIX2}!", "Ostré kameny se zabodly do těla! Zraněný je {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_TOXICSPIKESABSORBED]                  = COMPOUND_STRING(BATTLE_TEXT("The toxic spikes disappeared from the ground around {B_EFF_TEAM2} side!", "Jedovaté bodce už neobklopují {B_EFF_TEAM2} tým!")),
    [STRINGID_TOXICSPIKESPOISONED]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} was poisoned!", "{B_SCR_NAME_WITH_PREFIX} byl otráven!")),
    [STRINGID_TOXICSPIKESBADLYPOISONED]             = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} was badly poisoned!", "{B_SCR_NAME_WITH_PREFIX} byl silně otráven!")),
    [STRINGID_STICKYWEBSWITCHIN]                    = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} was caught in a sticky web!", "{B_EFF_NAME_WITH_PREFIX} uvízl v lepkavé síti!")),
    [STRINGID_HEALINGWISHCAMETRUE]                  = COMPOUND_STRING(BATTLE_TEXT("The healing wish came true for {B_SCR_NAME_WITH_PREFIX2}!", "Léčivé přání se splnilo! Uzdravuje se {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_HEALINGWISHHEALED]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} regained health!", "{B_SCR_NAME_WITH_PREFIX} se uzdravil!")),
    [STRINGID_LUNARDANCECAMETRUE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} became cloaked in mystical moonlight!", "{B_SCR_NAME_WITH_PREFIX} se zahalil do tajemného měsíčního světla!")),
    [STRINGID_CURSEDBODYDISABLED]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1} was disabled!", "{B_ATK_NAME_WITH_PREFIX}: útok {B_BUFF1} byl zablokován!")),
    [STRINGID_ATTACKERACQUIREDABILITY]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} acquired {B_ATK_ABILITY}!", "{B_ATK_NAME_WITH_PREFIX} získal schopnost {B_ATK_ABILITY}!")), // Not in Gen 5+
    [STRINGID_TARGETABILITYSTATLOWER]               = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY} {B_BUFF2}lowered its {B_BUFF1}!", "{B_DEF_NAME_WITH_PREFIX}: schopnost {B_DEF_ABILITY} {B_BUFF2}snížila statistiku {B_BUFF1}!")), // Not in Gen 5+
    [STRINGID_TARGETSTATWONTGOHIGHER]               = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1} won't go any higher!", "{B_DEF_NAME_WITH_PREFIX}: statistika {B_BUFF1} už nemůže růst!")),
    [STRINGID_PKMNMOVEBOUNCEDABILITY]               = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s {B_CURRENT_MOVE} was bounced back!", "{B_DEF_NAME_WITH_PREFIX}: útok {B_CURRENT_MOVE} byl odražen zpět!")),
    [STRINGID_IMPOSTERTRANSFORM]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} transformed into {B_DEF_NAME_WITH_PREFIX2}!", "{B_ATK_NAME_WITH_PREFIX} změnil podobu! Teď vypadá jako {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_ASSAULTVESTDOESNTALLOW]               = COMPOUND_STRING(BATTLE_TEXT("The effects of the {B_LAST_ITEM} prevent status moves from being used!\p", "Kvůli předmětu {B_LAST_ITEM} nelze použít stavové útoky!\p")),
    [STRINGID_GRAVITYPREVENTSUSAGE]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} can't use {B_CURRENT_MOVE} because of gravity!\p", "{B_ATK_NAME_WITH_PREFIX} nemůže kvůli gravitaci použít útok {B_CURRENT_MOVE}!\p")),
    [STRINGID_HEALBLOCKPREVENTSUSAGE]               = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was prevented from healing!\p", "{B_ATK_NAME_WITH_PREFIX} se nemůže léčit!\p")),
    [STRINGID_NOTDONEYET]                           = COMPOUND_STRING(BATTLE_TEXT("This move effect is not done yet!\p", "Efekt tohoto útoku zatím není hotový!\p")),
    [STRINGID_STICKYWEBUSED]                        = COMPOUND_STRING(BATTLE_TEXT("A sticky web has been laid out on the ground on {B_DEF_TEAM2} side!", "Lepkavá síť teď obklopuje {B_DEF_TEAM2} tým!")),
    [STRINGID_QUASHSUCCESS]                         = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s move was postponed!", "{B_DEF_NAME_WITH_PREFIX}: jeho útok byl odložen!")),
    [STRINGID_PKMNBLEWAWAYTOXICSPIKES]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} blew away Toxic Spikes!", "{B_ATK_NAME_WITH_PREFIX} odvál Toxic Spikes!")),
    [STRINGID_PKMNBLEWAWAYSTICKYWEB]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} blew away Sticky Web!", "{B_ATK_NAME_WITH_PREFIX} odvál Sticky Web!")),
    [STRINGID_PKMNBLEWAWAYSTEALTHROCK]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} blew away Stealth Rock!", "{B_ATK_NAME_WITH_PREFIX} odvál Stealth Rock!")),
    [STRINGID_IONDELUGEON]                          = COMPOUND_STRING(BATTLE_TEXT("A deluge of ions showers the battlefield!", "Bojiště zaplavila sprška iontů!")),
    [STRINGID_TOPSYTURVYSWITCHEDSTATS]              = COMPOUND_STRING(BATTLE_TEXT("All stat changes on {B_DEF_NAME_WITH_PREFIX2} were inverted!", "Všechny změny statistik se obrátily! Týká se to cíle: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_TERRAINBECOMESMISTY]                  = COMPOUND_STRING(BATTLE_TEXT("Mist swirled around the battlefield!", "Bojiště zahalila vířící mlha!")),
    [STRINGID_TERRAINBECOMESGRASSY]                 = COMPOUND_STRING(BATTLE_TEXT("Grass grew to cover the battlefield!", "Bojiště porostlo trávou!")),
    [STRINGID_TERRAINBECOMESELECTRIC]               = COMPOUND_STRING(BATTLE_TEXT("An electric current ran across the battlefield!", "Bojištěm proběhl elektrický proud!")),
    [STRINGID_TERRAINBECOMESPSYCHIC]                = COMPOUND_STRING(BATTLE_TEXT("The battlefield got weird!", "Bojiště ovládla podivná síla!")),
    [STRINGID_TARGETELECTRIFIED]                    = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s moves have been electrified!", "{B_DEF_NAME_WITH_PREFIX}: jeho útoky získaly elektrický náboj!")),
    [STRINGID_MEGAEVOREACTING]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s {B_LAST_ITEM} is reacting to {B_ATK_TRAINER_NAME}'s Mega Ring!", "{B_ATK_NAME_WITH_PREFIX}: předmět {B_LAST_ITEM} reaguje na Mega Ring! Jeho trenér: {B_ATK_TRAINER_NAME}!")), //actually displays the type of mega ring in inventory, but we didnt implement them :(
    [STRINGID_MEGAEVOEVOLVED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} has Mega Evolved into Mega {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} prošel megaevolucí a získal podobu Mega {B_BUFF1}!")),
    [STRINGID_DRASTICALLY]                          = gText_drastically,
    [STRINGID_SEVERELY]                             = gText_severely,
    [STRINGID_INFESTATION]                          = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} has been afflicted with an infestation by {B_SCR_NAME_WITH_PREFIX2}!", "{B_EFF_NAME_WITH_PREFIX} je zamořený havětí! Způsobil to {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_NOEFFECTONTARGET]                     = COMPOUND_STRING(BATTLE_TEXT("It won't have any effect on {B_DEF_NAME_WITH_PREFIX2}!", "Na tento cíl to nebude mít účinek: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_BURSTINGFLAMESHIT]                    = COMPOUND_STRING(BATTLE_TEXT("The bursting flames hit {B_EFF_NAME_WITH_PREFIX2}!", "Vyšlehly plameny! Zásah utrpěl {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_BESTOWITEMGIVING]                     = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} received {B_LAST_ITEM} from {B_ATK_NAME_WITH_PREFIX2}!", "{B_DEF_NAME_WITH_PREFIX} získal předmět {B_LAST_ITEM}! Předal ho {B_ATK_NAME_WITH_PREFIX2}!")),
    [STRINGID_THIRDTYPEADDED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} type was added to {B_DEF_NAME_WITH_PREFIX2}!", "Byl přidán typ {B_BUFF1}! Získal ho {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_FELLFORFEINT]                         = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} fell for the feint!", "{B_EFF_NAME_WITH_PREFIX} naletěl na lest!")),
    [STRINGID_POKEMONCANNOTUSEMOVE]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} cannot use {B_CURRENT_MOVE}!", "{B_ATK_NAME_WITH_PREFIX} nemůže použít útok {B_CURRENT_MOVE}!")),
    [STRINGID_COVEREDINPOWDER]                      = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} is covered in powder!", "{B_DEF_NAME_WITH_PREFIX} je pokrytý práškem!")),
    [STRINGID_POWDEREXPLODES]                       = COMPOUND_STRING(BATTLE_TEXT("When the flame touched the powder on the Pokémon, it exploded!", "Když se plamen dotkl prášku na Pokémonovi, došlo k výbuchu!")),
    [STRINGID_BELCHCANTSELECT]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} hasn't eaten any held Berries, so it can't possibly belch!\p", "{B_ATK_NAME_WITH_PREFIX} zatím nesnědl žádnou drženou bobuli, takže nemůže říhnout!\p")),
    [STRINGID_SPECTRALTHIEFSTEAL]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} stole the target's boosted stats!", "{B_ATK_NAME_WITH_PREFIX} ukradl cíli zvýšení statistik!")),
    [STRINGID_GRAVITYGROUNDING]                     = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} fell from the sky due to the gravity!", "{B_DEF_NAME_WITH_PREFIX} spadl z nebe kvůli gravitaci!")),
    [STRINGID_MISTYTERRAINPREVENTS]                 = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} surrounds itself with a protective mist!", "{B_DEF_NAME_WITH_PREFIX} se zahalil ochrannou mlhou!")),
    [STRINGID_GRASSYTERRAINHEALS]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is healed by the grassy terrain!", "{B_ATK_NAME_WITH_PREFIX} se léčí díky travnatému terénu!")),
    [STRINGID_ELECTRICTERRAINPREVENTS]              = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} surrounds itself with electrified terrain!", "{B_DEF_NAME_WITH_PREFIX} se obklopil elektrickým terénem!")),
    [STRINGID_PSYCHICTERRAINPREVENTS]               = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is protected by the Psychic Terrain!", "{B_SCR_NAME_WITH_PREFIX} je chráněný účinkem útoku Psychic Terrain!")),
    [STRINGID_SAFETYGOGGLESPROTECTED]               = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is not affected thanks to its {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX} je v bezpečí díky předmětu {B_LAST_ITEM}!")),
    [STRINGID_FLOWERVEILPROTECTED]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} surrounded itself with a veil of petals!", "{B_SCR_NAME_WITH_PREFIX} se zahalil závojem z okvětních lístků!")),
    [STRINGID_FLOWERVEILPROTECTEDTARGET]            = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} surrounded itself with a veil of petals!", "{B_DEF_NAME_WITH_PREFIX} se zahalil závojem z okvětních lístků!")),
    [STRINGID_AROMAVEILPROTECTED]                   = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} is protected by an aromatic veil!", "{B_DEF_NAME_WITH_PREFIX} je chráněný voňavým závojem!")),
    [STRINGID_CELEBRATEMESSAGE]                     = COMPOUND_STRING(BATTLE_TEXT("Congratulations, {B_PLAYER_NAME}!", "Gratulujeme, {B_PLAYER_NAME}!")),
    [STRINGID_USEDINSTRUCTEDMOVE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} followed {B_ATK_NAME_WITH_PREFIX2}'s instructions!", "{B_DEF_NAME_WITH_PREFIX} poslechl pokyny! Vydal je {B_ATK_NAME_WITH_PREFIX2}!")),
    [STRINGID_THROATCHOPENDS]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} can use sound-based moves again!", "{B_ATK_NAME_WITH_PREFIX} může znovu používat zvukové útoky!")),
    [STRINGID_PKMNCANTUSEMOVETHROATCHOP]            = COMPOUND_STRING(BATTLE_TEXT("The effects of Throat Chop prevent {B_ATK_NAME_WITH_PREFIX2} from using certain moves!\p", "Throat Chop brání v použití některých útoků! Omezený je {B_ATK_NAME_WITH_PREFIX2}!\p")),
    [STRINGID_LASERFOCUS]                           = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} concentrated intensely!", "{B_ATK_NAME_WITH_PREFIX} se hluboce soustředil!")),
    [STRINGID_GEMACTIVATES]                         = COMPOUND_STRING(BATTLE_TEXT("The {B_LAST_ITEM} strengthened {B_ATK_NAME_WITH_PREFIX2}'s power!", "Předmět {B_LAST_ITEM} posílil útok! Posílený je {B_ATK_NAME_WITH_PREFIX2}!")),
    [STRINGID_BERRYDMGREDUCES]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} lessened the damage it took!", "{B_SCR_NAME_WITH_PREFIX}: předmět {B_LAST_ITEM} zmírnil zranění!")),
    [STRINGID_AIRBALLOONFLOAT]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} floats in the air with its Air Balloon!", "{B_SCR_NAME_WITH_PREFIX} se vznáší díky předmětu Air Balloon!")),
    [STRINGID_AIRBALLOONPOP]                        = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s Air Balloon popped!", "{B_DEF_NAME_WITH_PREFIX}: předmět Air Balloon praskl!")),
    [STRINGID_INCINERATEBURN]                       = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX}'s {B_LAST_ITEM} was burnt up!", "{B_EFF_NAME_WITH_PREFIX}: předmět {B_LAST_ITEM} shořel!")),
    [STRINGID_BUGBITE]                              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} stole and ate its target's {B_LAST_ITEM}!", "{B_ATK_NAME_WITH_PREFIX} ukradl cíli předmět {B_LAST_ITEM} a snědl ho!")),
    [STRINGID_ILLUSIONWOREOFF]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s illusion wore off!", "{B_SCR_NAME_WITH_PREFIX}: iluze pominula!")),
    [STRINGID_ATTACKERCUREDTARGETSTATUS]            = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} cured {B_DEF_NAME_WITH_PREFIX2}'s problem!", "{B_ATK_NAME_WITH_PREFIX} vyléčil potíže! Uzdravil se {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_ATTACKERLOSTFIRETYPE]                 = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} burned itself out!", "{B_EFF_NAME_WITH_PREFIX} spálil veškerý svůj oheň!")),
    [STRINGID_HEALERCURE]                           = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was cured of {B_SCR_NAME_WITH_PREFIX2}!", "{B_ATK_NAME_WITH_PREFIX} se uzdravil. Pomohl mu {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_SCRIPTINGABILITYSTATRAISE]            = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} {B_BUFF2}raised its {B_BUFF1}!", "{B_SCR_NAME_WITH_PREFIX}: schopnost {B_SCR_ABILITY}{B_BUFF2} zvýšila statistiku {B_BUFF1}!")),
    [STRINGID_RECEIVERABILITYTAKEOVER]              = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} was taken over!", "{B_SCR_NAME_WITH_PREFIX}: schopnost {B_SCR_ABILITY} byla převzata!")),
    [STRINGID_PKNMABSORBINGPOWER]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is absorbing power!", "{B_ATK_NAME_WITH_PREFIX} vstřebává sílu!")),
    [STRINGID_NOONEWILLBEABLETORUNAWAY]             = COMPOUND_STRING(BATTLE_TEXT("No one will be able to leave the battlefield during the next turn!", "V příštím tahu nebude moct nikdo opustit bojiště!")),
    [STRINGID_DESTINYKNOTACTIVATES]                 = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} fell in love because of the {B_LAST_ITEM}!", "{B_DEF_NAME_WITH_PREFIX} se zamiloval kvůli předmětu {B_LAST_ITEM}!")),
    [STRINGID_CLOAKEDINAFREEZINGLIGHT]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} became cloaked in a freezing light!", "{B_ATK_NAME_WITH_PREFIX} se zahalil do mrazivého světla!")),
    [STRINGID_CLEARAMULETWONTLOWERSTATS]            = COMPOUND_STRING(BATTLE_TEXT("The effects of the {B_LAST_ITEM} held by {B_SCR_NAME_WITH_PREFIX2} prevents its stats from being lowered!", "Předmět {B_LAST_ITEM} brání snížení statistik! Drží ho {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_FERVENTWISHREACHED]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_TRAINER_NAME}'s fervent wish has reached {B_ATK_NAME_WITH_PREFIX2}!", "{B_ATK_TRAINER_NAME} vyslal vroucí přání! Pocítil ho {B_ATK_NAME_WITH_PREFIX2}!")),
    [STRINGID_AIRLOCKACTIVATES]                     = COMPOUND_STRING(BATTLE_TEXT("The effects of the weather disappeared.", "Účinky počasí pominuly.")),
    [STRINGID_PRESSUREENTERS]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is exerting its pressure!", "{B_SCR_NAME_WITH_PREFIX} vyvíjí nátlak!")),
    [STRINGID_DARKAURAENTERS]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is radiating a dark aura!", "{B_SCR_NAME_WITH_PREFIX} vyzařuje temnou auru!")),
    [STRINGID_FAIRYAURAENTERS]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is radiating a fairy aura!", "{B_SCR_NAME_WITH_PREFIX} vyzařuje vílí auru!")),
    [STRINGID_AURABREAKENTERS]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} reversed all other Pokémon's auras!", "{B_SCR_NAME_WITH_PREFIX} obrátil aury všech ostatních Pokémonů!")),
    [STRINGID_COMATOSEENTERS]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is drowsing!", "{B_SCR_NAME_WITH_PREFIX} podřimuje!")),
    [STRINGID_SCREENCLEANERENTERS]                  = COMPOUND_STRING(BATTLE_TEXT("All screens on the field were cleansed!", "Všechny ochranné stěny na bojišti zmizely!")),
    [STRINGID_FETCHEDPOKEBALL]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} found a {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX} našel předmět {B_LAST_ITEM}!")),
    [STRINGID_ASANDSTORMKICKEDUP]                   = COMPOUND_STRING(BATTLE_TEXT("A sandstorm kicked up!", "Zvedla se písečná bouře!")),
    [STRINGID_PKMNSWILLPERISHIN3TURNS]              = COMPOUND_STRING(BATTLE_TEXT("Both Pokémon will faint in three turns!", "Oba Pokémoni za tři tahy omdlí!")),
    [STRINGID_AURAFLAREDTOLIFE]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s aura flared to life!", "{B_ATK_NAME_WITH_PREFIX}: jeho aura vzplála!")),
    [STRINGID_ASONEENTERS]                          = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} has two Abilities!", "{B_SCR_NAME_WITH_PREFIX} má dvě schopnosti!")),
    [STRINGID_CURIOUSMEDICINEENTERS]                = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX}'s stat changes were removed!", "{B_EFF_NAME_WITH_PREFIX}: změny statistik byly zrušeny!")),
    [STRINGID_CANACTFASTERTHANKSTO]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} can act faster than normal, thanks to its {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} může díky účinku {B_BUFF1} jednat rychleji!")),
    [STRINGID_MICLEBERRYACTIVATES]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} boosted the accuracy of its next move using {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX} zvýšil přesnost příštího útoku pomocí předmětu {B_LAST_ITEM}!")),
    [STRINGID_PKMNSHOOKOFFTHETAUNT]                 = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} shook off the taunt!", "{B_SCR_NAME_WITH_PREFIX} setřásl provokaci!")),
    [STRINGID_PKMNGOTOVERITSINFATUATION]            = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} got over its infatuation!", "{B_SCR_NAME_WITH_PREFIX} se zbavil poblouznění!")),
    [STRINGID_ITEMCANNOTBEREMOVED]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s item cannot be removed!", "{B_ATK_NAME_WITH_PREFIX}: jeho předmět nelze odebrat!")),
    [STRINGID_STICKYBARBTRANSFER]                   = COMPOUND_STRING(BATTLE_TEXT("The {B_LAST_ITEM} attached itself to {B_ATK_NAME_WITH_PREFIX2}!", "Předmět {B_LAST_ITEM} se přichytil! Teď ho má {B_ATK_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNBURNHEALED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX}'s burn was cured!", "{B_DEF_NAME_WITH_PREFIX}: popálení bylo vyléčeno!")),
    [STRINGID_REDCARDACTIVATE]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} held up its Red Card against {B_ATK_NAME_WITH_PREFIX2}!", "{B_SCR_NAME_WITH_PREFIX} vytáhl Red Card! Musí odejít {B_ATK_NAME_WITH_PREFIX2}!")),
    [STRINGID_EJECTBUTTONACTIVATE]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is switched out with the {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX} je střídán díky předmětu {B_LAST_ITEM}!")),
    [STRINGID_ATKGOTOVERINFATUATION]                = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} cured its infatuation status using its {B_LAST_ITEM}!", "{B_SCR_NAME_WITH_PREFIX} se zbavil poblouznění pomocí předmětu {B_LAST_ITEM}!")),
    [STRINGID_TORMENTEDNOMORE]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is no longer tormented!", "{B_SCR_NAME_WITH_PREFIX} už není sužován!")),
    [STRINGID_HEALBLOCKEDNOMORE]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} is no longer prevented from healing!", "{B_SCR_NAME_WITH_PREFIX} se může znovu léčit!")),
    [STRINGID_ATTACKERBECAMEFULLYCHARGED]           = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} became fully charged due to its bond with its trainer!\p", "{B_ATK_NAME_WITH_PREFIX} se plně nabil díky poutu se svým trenérem!\p")),
    [STRINGID_ATTACKERBECAMEASHSPECIES]             = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} became Ash-Greninja!\p", "{B_ATK_NAME_WITH_PREFIX} získal podobu Ash-Greninja!\p")),
    [STRINGID_EXTREMELYHARSHSUNLIGHT]               = COMPOUND_STRING(BATTLE_TEXT("The sunlight turned extremely harsh!", "Sluneční svit je nesnesitelně silný!")),
    [STRINGID_EXTREMESUNLIGHTFADED]                 = COMPOUND_STRING(BATTLE_TEXT("The extremely harsh sunlight faded!", "Nesnesitelně silný sluneční svit pominul!")),
    [STRINGID_MOVEEVAPORATEDINTHEHARSHSUNLIGHT]     = COMPOUND_STRING(BATTLE_TEXT("The Water-type attack evaporated in the extremely harsh sunlight!", "Vodní útok se vypařil v nesnesitelně silném slunečním svitu!")),
    [STRINGID_EXTREMELYHARSHSUNLIGHTWASNOTLESSENED] = COMPOUND_STRING(BATTLE_TEXT("The extremely harsh sunlight was not lessened at all!", "Nesnesitelně silný sluneční svit vůbec nepolevil!")),
    [STRINGID_HEAVYRAIN]                            = COMPOUND_STRING(BATTLE_TEXT("A heavy rain began to fall!", "Spustil se prudký déšť!")),
    [STRINGID_HEAVYRAINLIFTED]                      = COMPOUND_STRING(BATTLE_TEXT("The heavy rain has lifted!", "Prudký déšť ustal!")),
    [STRINGID_MOVEFIZZLEDOUTINTHEHEAVYRAIN]         = COMPOUND_STRING(BATTLE_TEXT("The Fire-type attack fizzled out in the heavy rain!", "Ohnivý útok zhasl v prudkém dešti!")),
    [STRINGID_NORELIEFROMHEAVYRAIN]                 = COMPOUND_STRING(BATTLE_TEXT("There is no relief from this heavy rain!", "Prudký déšť vůbec nepolevuje!")),
    [STRINGID_MYSTERIOUSAIRCURRENT]                 = COMPOUND_STRING(BATTLE_TEXT("Mysterious strong winds are protecting Flying-type Pokémon!", "Tajemné silné větry chrání létající Pokémony!")),
    [STRINGID_STRONGWINDSDISSIPATED]                = COMPOUND_STRING(BATTLE_TEXT("The mysterious strong winds have dissipated!", "Tajemné silné větry ustaly!")),
    [STRINGID_MYSTERIOUSAIRCURRENTBLOWSON]          = COMPOUND_STRING(BATTLE_TEXT("The mysterious strong winds blow on regardless!", "Tajemné silné větry vanou dál!")),
    [STRINGID_ATTACKWEAKENEDBSTRONGWINDS]           = COMPOUND_STRING(BATTLE_TEXT("The mysterious strong winds weakened the attack!", "Tajemné silné větry zeslabily útok!")),
    [STRINGID_STUFFCHEEKSCANTSELECT]                = COMPOUND_STRING(BATTLE_TEXT("It can't use the move because it doesn't have a Berry!\p", "Útok nelze použít bez bobule!\p")),
    [STRINGID_PKMNREVERTEDTOPRIMAL]                 = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s Primal Reversion! It reverted to its primal state!", "{B_SCR_NAME_WITH_PREFIX}: Primal Reversion! Vrátil se do prapůvodní podoby!")),
    [STRINGID_BUTPOKEMONCANTUSETHEMOVE]             = COMPOUND_STRING(BATTLE_TEXT("But {B_ATK_NAME_WITH_PREFIX2} can't use the move!", "Ale {B_ATK_NAME_WITH_PREFIX2} tento útok nemůže použít!")),
    [STRINGID_BUTHOOPACANTUSEIT]                    = COMPOUND_STRING(BATTLE_TEXT("But {B_ATK_NAME_WITH_PREFIX2} can't use it the way it is now!", "Ale {B_ATK_NAME_WITH_PREFIX2} to ve své nynější podobě nemůže použít!")),
    [STRINGID_BROKETHROUGHPROTECTION]               = COMPOUND_STRING(BATTLE_TEXT("It broke through {B_EFF_NAME_WITH_PREFIX2}'s protection!", "Ochrana byla prolomena! V ohrožení je {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_ABILITYALLOWSONLYMOVE]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_ABILITY} only allows the use of {B_CURRENT_MOVE}!\p", "Schopnost {B_ATK_ABILITY} dovoluje použít jen útok {B_CURRENT_MOVE}!\p")),
    [STRINGID_SWAPPEDABILITIES]                     = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} swapped Abilities with its target!", "{B_DEF_NAME_WITH_PREFIX} si s cílem prohodil schopnosti!")),
    [STRINGID_PKMNHEALEDPOISON]                     = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} was cured of its poisoning!", "{B_DEF_NAME_WITH_PREFIX} byl vyléčen z otravy!")),
    [STRINGID_BATTLERTYPECHANGEDTO]                 = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s type changed to {B_BUFF1}!", "{B_SCR_NAME_WITH_PREFIX} změnil svůj typ na {B_BUFF1}!")),
    [STRINGID_BOTHCANNOLONGERESCAPE]                = COMPOUND_STRING(BATTLE_TEXT("Neither Pokémon can run away!", "Ani jeden Pokémon nemůže utéct!")),
    [STRINGID_CANTESCAPEDUETOUSEDMOVE]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} can no longer escape because it used No Retreat!", "{B_ATK_NAME_WITH_PREFIX} už nemůže utéct, protože použil No Retreat!")),
    [STRINGID_PKMNBECAMEWEAKERTOFIRE]               = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} became weaker to fire!", "{B_DEF_NAME_WITH_PREFIX} je zranitelnější vůči ohni!")),
    [STRINGID_ABOUTTOUSEPOLTERGEIST]                = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} is about to be attacked by its {B_LAST_ITEM}!", "{B_EFF_NAME_WITH_PREFIX} bude napaden vlastním předmětem {B_LAST_ITEM}!")),
    [STRINGID_CANTESCAPEBECAUSEOFCURRENTMOVE]       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} can no longer escape because of Octolock!", "{B_DEF_NAME_WITH_PREFIX} už nemůže utéct kvůli útoku Octolock!")),
    [STRINGID_NEUTRALIZINGGASENTERS]                = COMPOUND_STRING(BATTLE_TEXT("Neutralizing gas filled the area!", "Prostor zaplnil neutralizující plyn!")),
    [STRINGID_NEUTRALIZINGGASOVER]                  = COMPOUND_STRING(BATTLE_TEXT("The effects of the neutralizing gas wore off!", "Účinky neutralizujícího plynu pominuly!")),
    [STRINGID_TARGETTOOHEAVY]                       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} is too heavy to be lifted!", "{B_DEF_NAME_WITH_PREFIX} je příliš těžký na zvednutí!")),
    [STRINGID_PKMNTOOKTARGETHIGH]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} took {B_DEF_NAME_WITH_PREFIX2} into the sky!", "{B_ATK_NAME_WITH_PREFIX} se vznesl do nebe! S ním i {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNINSNAPTRAP]                       = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} got trapped by a snap trap!", "{B_EFF_NAME_WITH_PREFIX} uvízl ve sklapovací pasti!")),
    [STRINGID_METEORBEAMCHARGING]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is overflowing with space power!", "{B_ATK_NAME_WITH_PREFIX} překypuje vesmírnou silou!")),
    [STRINGID_HEATUPBEAK]                           = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} started heating up its beak!", "{B_SCR_NAME_WITH_PREFIX} si začal zahřívat zobák!")),
    [STRINGID_COURTCHANGE]                          = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} swapped the battle effects affecting each side of the field!", "{B_ATK_NAME_WITH_PREFIX} prohodil bojové účinky mezi stranami bojiště!")),
    [STRINGID_ZPOWERSURROUNDS]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} surrounded itself with its Z-Power!", "{B_ATK_NAME_WITH_PREFIX} se obklopil svou Z-silou!")),
    [STRINGID_ZMOVEUNLEASHED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} unleashes its full-force Z-Move!", "{B_ATK_NAME_WITH_PREFIX} vypouští Z-útok plnou silou!")),
    [STRINGID_ZMOVERESETSSTATS]                     = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} returned its decreased stats to normal using its Z-Power!", "{B_SCR_NAME_WITH_PREFIX} obnovil snížené statistiky svou Z-silou!")),
    [STRINGID_ZMOVEALLSTATSUP]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} boosted its stats using its Z-Power!", "{B_SCR_NAME_WITH_PREFIX} zvýšil své statistiky Z-silou!")),
    [STRINGID_ZMOVEZBOOSTCRIT]                      = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} boosted its critical-hit ratio using its Z-Power!", "{B_SCR_NAME_WITH_PREFIX} zvýšil šanci na kritický zásah svou Z-silou!")),
    [STRINGID_ZMOVERESTOREHP]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} restored its HP using its Z-Power!", "{B_SCR_NAME_WITH_PREFIX} obnovil své HP Z-silou!")),
    [STRINGID_ZMOVESTATUP]                          = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} boosted its stats using its Z-Power!", "{B_SCR_NAME_WITH_PREFIX} zvýšil své statistiky Z-silou!")),
    [STRINGID_ZMOVEHPTRAP]                          = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s HP was restored by the Z-Power!", "{B_SCR_NAME_WITH_PREFIX}: Z-síla obnovila jeho HP!")),
    [STRINGID_ATTACKEREXPELLEDTHEPOISON]            = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} managed to expel the poison so you wouldn't worry!", "{B_ATK_NAME_WITH_PREFIX} vypudil jed, abys neměl starosti!")),
    [STRINGID_ATTACKERSHOOKITSELFAWAKE]             = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} shook itself awake so you wouldn't worry!", "{B_ATK_NAME_WITH_PREFIX} se probudil, abys neměl starosti!")),
    [STRINGID_ATTACKERBROKETHROUGHPARALYSIS]        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} gathered all its energy to break through its paralysis so you wouldn't worry!", "{B_ATK_NAME_WITH_PREFIX} sebral všechny síly a překonal ochrnutí, abys neměl starosti!")),
    [STRINGID_ATTACKERHEALEDITSBURN]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} cured its burn through sheer determination so you wouldn't worry!", "{B_ATK_NAME_WITH_PREFIX} silou vůle vyléčil popálení, abys neměl starosti!")),
    [STRINGID_ATTACKERMELTEDTHEICE]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} melted the ice with its fiery determination so you wouldn't worry!", "{B_ATK_NAME_WITH_PREFIX} žhavým odhodláním rozpustil led, abys neměl starosti!")),
    [STRINGID_TARGETTOUGHEDITOUT]                   = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} toughed it out so you wouldn't feel sad!", "{B_DEF_NAME_WITH_PREFIX} to vydržel, abys nebyl smutný!")),
    [STRINGID_ATTACKERLOSTELECTRICTYPE]             = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} used up all its electricity!", "{B_EFF_NAME_WITH_PREFIX} vyčerpal veškerou svou elektřinu!")),
    [STRINGID_ATTACKERSWITCHEDSTATWITHTARGET]       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} switched {B_BUFF1} with its target!", "{B_ATK_NAME_WITH_PREFIX} si s cílem prohodil statistiku {B_BUFF1}!")),
    [STRINGID_BEINGHITCHARGEDPKMNWITHPOWER]         = COMPOUND_STRING(BATTLE_TEXT("Being hit by {B_CURRENT_MOVE} charged {B_EFF_NAME_WITH_PREFIX2} with power!", "Zásah útokem {B_CURRENT_MOVE} dodal sílu! Nabil se {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_SUNLIGHTACTIVATEDABILITY]             = COMPOUND_STRING(BATTLE_TEXT("The harsh sunlight activated {B_SCR_NAME_WITH_PREFIX2}'s Protosynthesis!", "Silný sluneční svit aktivoval Protosynthesis! Tuto schopnost má {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_ORICHALCUMPULSEACTIVATES]             = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} turned the sunlight harsh, sending its ancient pulse into a frenzy!", "{B_SCR_NAME_WITH_PREFIX} zesílil sluneční svit a rozběsnil svůj pradávný tep!")),
    [STRINGID_ORICHALCUMPULSEACTIVATESINSUN]        = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} basked in the sunlight, sending its ancient pulse into a frenzy!", "{B_SCR_NAME_WITH_PREFIX} se vyhřívá na slunci a rozběsnil svůj pradávný tep!")),
    [STRINGID_STATWASHEIGHTENED]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_BUFF1} was heightened!", "{B_SCR_NAME_WITH_PREFIX}: statistika {B_BUFF1} vzrostla!")),
    [STRINGID_ELECTRICTERRAINACTIVATEDABILITY]      = COMPOUND_STRING(BATTLE_TEXT("The Electric Terrain activated {B_SCR_NAME_WITH_PREFIX2}'s Quark Drive!", "Electric Terrain aktivoval Quark Drive! Tuto schopnost má {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_HADRONENGINEACTIVATES]                = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} turned the ground into Electric Terrain, energizing its futuristic engine!", "{B_SCR_NAME_WITH_PREFIX} vytvořil Electric Terrain a nabil svůj futuristický motor!")),
    [STRINGID_HADRONENGINEACTIVATESINTERRAIN]       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} used the Electric Terrain to energize its futuristic engine!", "{B_SCR_NAME_WITH_PREFIX} využil Electric Terrain k nabití svého futuristického motoru!")),
    [STRINGID_ABILITYWEAKENEDSURROUNDINGMONSSTAT]   = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} weakened the {B_BUFF1} of all surrounding Pokémon!\p", "{B_SCR_NAME_WITH_PREFIX}: schopnost {B_SCR_ABILITY} snížila statistiku {B_BUFF1} všem okolním Pokémonům!\p")),
    [STRINGID_ATTACKERGAINEDSTRENGTHFROMTHEFALLEN]  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} gained strength from the fallen!", "{B_SCR_NAME_WITH_PREFIX} čerpá sílu z padlých!")),
    [STRINGID_PKMNSABILITYPREVENTSABILITY]          = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_SCR_ABILITY} prevents {B_DEF_NAME_WITH_PREFIX2}'s {B_DEF_ABILITY} from working!", "{B_SCR_NAME_WITH_PREFIX}: schopnost {B_SCR_ABILITY} blokuje schopnost {B_DEF_ABILITY}! Postižený je {B_DEF_NAME_WITH_PREFIX2}!")), //not in gen 5+, ability popup
    [STRINGID_PREPARESHELLTRAP]                     = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} set a shell trap!", "{B_SCR_NAME_WITH_PREFIX} nastražil lasturovou past!")),
    [STRINGID_SHELLTRAPDIDNTWORK]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s shell trap didn't work!", "{B_ATK_NAME_WITH_PREFIX}: jeho lasturová past selhala!")),
    [STRINGID_SPIKESDISAPPEAREDFROMTEAM]            = COMPOUND_STRING(BATTLE_TEXT("The spikes disappeared from the ground around {B_ATK_TEAM2} side!", "Bodce už neobklopují {B_ATK_TEAM2} tým!")),
    [STRINGID_TOXICSPIKESDISAPPEAREDFROMTEAM]       = COMPOUND_STRING(BATTLE_TEXT("The toxic spikes disappeared from the ground around {B_ATK_TEAM2} side!", "Jedovaté bodce už neobklopují {B_ATK_TEAM2} tým!")),
    [STRINGID_STICKYWEBDISAPPEAREDFROMTEAM]         = COMPOUND_STRING(BATTLE_TEXT("The sticky web has disappeared from the ground on {B_ATK_TEAM2} side!", "Lepkavá síť už neobklopuje {B_ATK_TEAM2} tým!")),
    [STRINGID_STEALTHROCKDISAPPEAREDFROMTEAM]       = COMPOUND_STRING(BATTLE_TEXT("The pointed stones disappeared from {B_ATK_TEAM2} side!", "Ostré kameny už neobklopují {B_ATK_TEAM2} tým!")),
    [STRINGID_COULDNTFULLYPROTECT]                  = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} couldn't fully protect itself and got hurt!", "{B_DEF_NAME_WITH_PREFIX} se nedokázal plně ochránit a utrpěl zranění!")),
    [STRINGID_STOCKPILEDEFFECTWOREOFF]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s stockpiled effect wore off!", "{B_ATK_NAME_WITH_PREFIX}: nashromážděný účinek pominul!")),
    [STRINGID_PKMNREVIVEDREADYTOFIGHT]              = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} was revived and is ready to fight again!", "{B_BUFF1} byl oživen a může znovu bojovat!")),
    [STRINGID_ITEMRESTOREDSPECIESHEALTH]            = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} had its HP restored.", "{B_BUFF1}: jeho HP byla obnovena.")),
    [STRINGID_ITEMCUREDSPECIESSTATUS]               = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} had its status healed!", "{B_BUFF1}: jeho stav byl vyléčen!")), // Not in Gen 5+
    [STRINGID_ITEMRESTOREDSPECIESPP]                = COMPOUND_STRING(BATTLE_TEXT("The PP of {B_BUFF1}'s {B_BUFF2} was restored!", "{B_BUFF1}: útoku {B_BUFF2} byly obnoveny PP!")),
    [STRINGID_THUNDERCAGETRAPPED]                   = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} trapped {B_EFF_NAME_WITH_PREFIX2}!", "{B_SCR_NAME_WITH_PREFIX} nastražil past! Uvízl v ní {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNHURTBYFROSTBITE]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was hurt by its frostbite!", "{B_ATK_NAME_WITH_PREFIX} trpí omrzlinami!")),
    [STRINGID_PKMNGOTFROSTBITE]                     = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} got frostbite!", "{B_EFF_NAME_WITH_PREFIX} utrpěl omrzliny!")),
    [STRINGID_PKMNSITEMHEALEDFROSTBITE]             = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_LAST_ITEM} cured its frostbite!", "{B_SCR_NAME_WITH_PREFIX}: předmět {B_LAST_ITEM} vyléčil omrzliny!")),
    [STRINGID_ATTACKERHEALEDITSFROSTBITE]           = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} cured its frostbite through sheer determination so you wouldn't worry!", "{B_ATK_NAME_WITH_PREFIX} silou vůle vyléčil omrzliny, abys neměl starosti!")),
    [STRINGID_PKMNFROSTBITEHEALED]                  = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s frostbite was cured!", "{B_SCR_NAME_WITH_PREFIX}: omrzliny byly vyléčeny!")),
    [STRINGID_PKMNFROSTBITEHEALEDBY]                = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s {B_CURRENT_MOVE} cured its frostbite!", "{B_SCR_NAME_WITH_PREFIX}: útok {B_CURRENT_MOVE} vyléčil omrzliny!")),
    [STRINGID_MIRRORHERBCOPIED]                     = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} used its Mirror Herb to mirror its opponent's stat changes!", "{B_SCR_NAME_WITH_PREFIX} pomocí Mirror Herb zkopíroval soupeřovy změny statistik!")),
    [STRINGID_STARTEDSNOW]                          = COMPOUND_STRING(BATTLE_TEXT("It started to snow!", "Začalo sněžit!")),
    [STRINGID_SNOWCONTINUES]                        = COMPOUND_STRING(BATTLE_TEXT("Snow continues to fall.", "Sníh dál padá.")), //not in gen 5+ (lol)
    [STRINGID_SNOWSTOPPED]                          = COMPOUND_STRING(BATTLE_TEXT("The snow stopped.", "Přestalo sněžit.")),
    [STRINGID_SNOWWARNINGSNOW]                      = COMPOUND_STRING(BATTLE_TEXT("It started to snow!", "Začalo sněžit!")),
    [STRINGID_PKMNITEMMELTED]                       = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} corroded {B_DEF_NAME_WITH_PREFIX2}'s {B_LAST_ITEM}!", "{B_ATK_NAME_WITH_PREFIX} naleptal předmět {B_LAST_ITEM}! Drží ho {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_ULTRABURSTREACTING]                   = COMPOUND_STRING(BATTLE_TEXT("Bright light is about to burst out of {B_ATK_NAME_WITH_PREFIX2}!", "Jasným světlem brzy zazáří {B_ATK_NAME_WITH_PREFIX2}!")),
    [STRINGID_ULTRABURSTCOMPLETED]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} regained its true power through Ultra Burst!", "{B_ATK_NAME_WITH_PREFIX} získal zpět svou pravou sílu díky Ultra Burst!")),
    [STRINGID_TEAMGAINEDEXP]                        = COMPOUND_STRING(BATTLE_TEXT("The rest of your team gained Exp. Points thanks to the Exp. Share!\p", "Zbytek tvého týmu získal zkušenosti díky Exp. Share!\p")),
    [STRINGID_CURRENTMOVECANTSELECT]                = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} cannot be used!\p", "{B_BUFF1} nelze použít!\p")),
    [STRINGID_TARGETISBEINGSALTCURED]               = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} is being salt cured!", "{B_EFF_NAME_WITH_PREFIX} byl zasolen!")),
    [STRINGID_TARGETISHURTBYSALTCURE]               = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is hurt by {B_BUFF1}!", "{B_ATK_NAME_WITH_PREFIX} utrpěl zranění! Příčina: {B_BUFF1}!")),
    [STRINGID_TARGETCOVEREDINSTICKYCANDYSYRUP]      = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} got covered in sticky candy syrup!", "{B_EFF_NAME_WITH_PREFIX} byl politý lepkavým sladkým sirupem!")),
    [STRINGID_SHARPSTEELFLOATS]                     = COMPOUND_STRING(BATTLE_TEXT("Sharp-pointed pieces of steel started floating around {B_DEF_TEAM2} Pokémon!", "Ostré kusy oceli začaly ohrožovat {B_DEF_TEAM2} tým!")),
    [STRINGID_SHARPSTEELDMG]                        = COMPOUND_STRING(BATTLE_TEXT("The sharp steel bit into {B_DEF_NAME_WITH_PREFIX2}!", "Ostrá ocel se zakousla do cíle: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_PKMNBLEWAWAYSHARPSTEEL]               = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} blew away sharp steel!", "{B_ATK_NAME_WITH_PREFIX} odfoukl ostrou ocel!")),
    [STRINGID_SHARPSTEELDISAPPEAREDFROMTEAM]        = COMPOUND_STRING(BATTLE_TEXT("The pieces of steel surrounding {B_ATK_TEAM2} Pokémon disappeared!", "Ocel už neohrožuje {B_ATK_TEAM2} tým!")),
    [STRINGID_TEAMTRAPPEDWITHVINES]                 = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_TEAM1} Pokémon got trapped with vines!", "{B_EFF_TEAM1} tým uvěznily liány!")),
    [STRINGID_PKMNHURTBYVINES]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is hurt by G-Max Vine Lash's ferocious beating!", "{B_ATK_NAME_WITH_PREFIX} trpí pod zuřivými údery G-Max Vine Lash!")),
    [STRINGID_TEAMCAUGHTINVORTEX]                   = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_TEAM1} Pokémon got caught in a vortex of water!", "{B_EFF_TEAM1} tým pohltil vodní vír!")),
    [STRINGID_PKMNHURTBYVORTEX]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is hurt by G-Max Cannonade's vortex!", "{B_ATK_NAME_WITH_PREFIX} trpí ve víru G-Max Cannonade!")),
    [STRINGID_TEAMSURROUNDEDBYFIRE]                 = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_TEAM1} Pokémon were surrounded by fire!", "{B_EFF_TEAM1} tým obklopily plameny!")),
    [STRINGID_PKMNBURNINGUP]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is burning up within G-Max Wildfire's flames!", "{B_ATK_NAME_WITH_PREFIX} hoří v plamenech G-Max Wildfire!")),
    [STRINGID_TEAMSURROUNDEDBYROCKS]                = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_TEAM1} Pokémon became surrounded by rocks!", "{B_EFF_TEAM1} tým obklopily skály!")),
    [STRINGID_PKMNHURTBYROCKSTHROWN]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is hurt by rocks thrown out by G-Max Volcalith!", "{B_ATK_NAME_WITH_PREFIX} utrpěl zranění od kamenů z G-Max Volcalith!")),
    [STRINGID_MOVEBLOCKEDBYDYNAMAX]                 = COMPOUND_STRING(BATTLE_TEXT("The move was blocked by the power of Dynamax!", "Útok zablokovala síla Dynamaxu!")),
    [STRINGID_ZEROTOHEROTRANSFORMATION]             = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} underwent a heroic transformation!", "{B_SCR_NAME_WITH_PREFIX} prošel hrdinskou proměnou!")),
    [STRINGID_THETWOMOVESBECOMEONE]                 = COMPOUND_STRING(BATTLE_TEXT("The two moves have become one! It's a combined move!{PAUSE 16}", "Oba útoky se spojily v jeden! Je to kombinovaný útok!{PAUSE 16}")),
    [STRINGID_ARAINBOWAPPEAREDONSIDE]               = COMPOUND_STRING(BATTLE_TEXT("A rainbow appeared in the sky on {B_EFF_TEAM2} side!", "Nad bojištěm se objevila duha pro {B_EFF_TEAM2} tým!")),
    [STRINGID_THERAINBOWDISAPPEARED]                = COMPOUND_STRING(BATTLE_TEXT("The rainbow on {B_ATK_TEAM2} side disappeared!", "Duha už nechrání {B_ATK_TEAM2} tým!")),
    [STRINGID_WAITINGFORPARTNERSMOVE]               = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is waiting for {B_ATK_PARTNER_NAME}'s move…{PAUSE 16}", "{B_ATK_NAME_WITH_PREFIX} čeká na útok partnera {B_ATK_PARTNER_NAME}…{PAUSE 16}")),
    [STRINGID_SEAOFFIREENVELOPEDSIDE]               = COMPOUND_STRING(BATTLE_TEXT("A sea of fire enveloped {B_EFF_TEAM2} side!", "Moře plamenů obklopilo {B_EFF_TEAM2} tým!")),
    [STRINGID_HURTBYTHESEAOFFIRE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} was hurt by the sea of fire!", "{B_ATK_NAME_WITH_PREFIX} utrpěl zranění v moři plamenů!")),
    [STRINGID_THESEAOFFIREDISAPPEARED]              = COMPOUND_STRING(BATTLE_TEXT("The sea of fire around {B_ATK_TEAM2} side disappeared!", "Moře plamenů už neohrožuje {B_ATK_TEAM2} tým!")),
    [STRINGID_SWAMPENVELOPEDSIDE]                   = COMPOUND_STRING(BATTLE_TEXT("A swamp enveloped {B_EFF_TEAM2} side!", "Bažina pohltila {B_EFF_TEAM2} tým!")),
    [STRINGID_THESWAMPDISAPPEARED]                  = COMPOUND_STRING(BATTLE_TEXT("The swamp around {B_ATK_TEAM2} side disappeared!", "Bažina už neomezuje {B_ATK_TEAM2} tým!")),
    [STRINGID_PKMNTELLCHILLINGRECEPTIONJOKE]        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is preparing to tell a chillingly bad joke!", "{B_ATK_NAME_WITH_PREFIX} se chystá říct mrazivě špatný vtip!")),
    [STRINGID_HOSPITALITYRESTORATION]               = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} drank down all the matcha that {B_SCR_NAME_WITH_PREFIX2} made!", "{B_EFF_NAME_WITH_PREFIX} vypil všechnu matchu, kterou připravil {B_SCR_NAME_WITH_PREFIX2}!")),
    [STRINGID_ELECTROSHOTCHARGING]                  = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} absorbed electricity!", "{B_ATK_NAME_WITH_PREFIX} pohltil elektřinu!")),
    [STRINGID_ITEMWASUSEDUP]                        = COMPOUND_STRING(BATTLE_TEXT("The {B_LAST_ITEM} was used up…", "Předmět {B_LAST_ITEM} se spotřeboval…")),
    [STRINGID_ATTACKERLOSTITSTYPE]                  = COMPOUND_STRING(BATTLE_TEXT("{B_EFF_NAME_WITH_PREFIX} lost its {B_BUFF1} type!", "{B_EFF_NAME_WITH_PREFIX} ztratil typ {B_BUFF1}!")),
    [STRINGID_SHEDITSTAIL]                          = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} shed its tail to create a decoy!", "{B_ATK_NAME_WITH_PREFIX} odhodil ocas a vytvořil návnadu!")),
    [STRINGID_CLOAKEDINAHARSHLIGHT]                 = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} became cloaked in a harsh light!", "{B_ATK_NAME_WITH_PREFIX} se zahalil do oslnivého světla!")),
    [STRINGID_SUPERSWEETAROMAWAFTS]                 = COMPOUND_STRING(BATTLE_TEXT("A supersweet aroma is wafting from the syrup covering {B_EFF_NAME_WITH_PREFIX2}!", "Přesladce voní sirup, kterým je pokrytý {B_EFF_NAME_WITH_PREFIX2}!")),
    [STRINGID_DIMENSIONSWERETWISTED]                = COMPOUND_STRING(BATTLE_TEXT("The dimensions were twisted!", "Rozměry se pokřivily!")),
    [STRINGID_BIZARREARENACREATED]                  = COMPOUND_STRING(BATTLE_TEXT("A bizarre area was created in which Pokémon's held items lose their effects!", "Vznikla podivná oblast, ve které držené předměty ztrácejí účinek!")),
    [STRINGID_BIZARREAREACREATED]                   = COMPOUND_STRING(BATTLE_TEXT("A bizarre area was created in which Defense and Sp. Def stats are swapped!", "Vznikla podivná oblast, ve které si obrana a spec. obrana vymění hodnoty!")),
    [STRINGID_TIDYINGUPCOMPLETE]                    = COMPOUND_STRING(BATTLE_TEXT("Tidying up complete!", "Úklid je hotový!")),
    [STRINGID_PKMNTERASTALLIZEDINTO]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} terastallized into the {B_BUFF1} type!", "{B_ATK_NAME_WITH_PREFIX} terastalizoval do typu {B_BUFF1}!")), // Does not exist, meant to mimic form change strings
    [STRINGID_BOOSTERENERGYACTIVATES]               = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} used its {B_LAST_ITEM} to activate {B_SCR_ABILITY}!", "{B_SCR_NAME_WITH_PREFIX} použil {B_LAST_ITEM} a aktivoval schopnost {B_SCR_ABILITY}!")),
    [STRINGID_FOGCREPTUP]                           = COMPOUND_STRING(BATTLE_TEXT("Fog crept up as thick as soup!", "Přivalila se mlha hustá jako polévka!")),
    [STRINGID_FOGISDEEP]                            = COMPOUND_STRING(BATTLE_TEXT("The fog is deep…", "Mlha je hustá…")),
    [STRINGID_FOGLIFTED]                            = COMPOUND_STRING(BATTLE_TEXT("The fog lifted.", "Mlha se rozplynula.")),
    [STRINGID_PKMNMADESHELLGLEAM]                   = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME_WITH_PREFIX} made its shell gleam! It's distorting type matchups!", "{B_DEF_NAME_WITH_PREFIX} rozzářil svůj krunýř! Mění tím účinnost typů!")),
    [STRINGID_FICKLEBEAMDOUBLED]                    = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is going all out for this attack!", "{B_ATK_NAME_WITH_PREFIX} dává do tohoto útoku všechno!")),
    [STRINGID_COMMANDERACTIVATES]                   = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} was swallowed by {B_BUFF1} and became {B_BUFF1}'s commander!", "{B_SCR_NAME_WITH_PREFIX} byl spolknut Pokémonem {B_BUFF1} a teď velí Pokémonovi {B_BUFF1}!")),
    [STRINGID_POKEFLUTECATCHY]                      = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_NAME} played the {B_LAST_ITEM}.\pNow, that's a catchy tune!", "{B_PLAYER_NAME} zahrál na {B_LAST_ITEM}.\pTo je ale chytlavá melodie!")),
    [STRINGID_POKEFLUTE]                            = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_NAME} played the {B_LAST_ITEM}.", "{B_PLAYER_NAME} zahrál na {B_LAST_ITEM}.")),
    [STRINGID_MONHEARINGFLUTEAWOKE]                 = COMPOUND_STRING(BATTLE_TEXT("The Pokémon hearing the flute awoke!", "Pokémoni, kteří slyšeli flétnu, se probudili!")),
    [STRINGID_SUNLIGHTISHARSH]                      = COMPOUND_STRING(BATTLE_TEXT("The sunlight is harsh!", "Slunce ostře pálí!")),
    [STRINGID_ITISHAILING]                          = COMPOUND_STRING(BATTLE_TEXT("It's hailing!", "Padají kroupy!")),
    [STRINGID_ITISSNOWING]                          = COMPOUND_STRING(BATTLE_TEXT("It's snowing!", "Sněží!")),
    [STRINGID_ISCOVEREDWITHGRASS]                   = COMPOUND_STRING(BATTLE_TEXT("The battlefield is covered with grass!", "Bojiště pokryla tráva!")),
    [STRINGID_MISTSWIRLSAROUND]                     = COMPOUND_STRING(BATTLE_TEXT("Mist swirls around the battlefield!", "Kolem bojiště víří mlha!")),
    [STRINGID_ELECTRICCURRENTISRUNNING]             = COMPOUND_STRING(BATTLE_TEXT("An electric current is running across the battlefield!", "Bojištěm proudí elektřina!")),
    [STRINGID_SEEMSWEIRD]                           = COMPOUND_STRING(BATTLE_TEXT("The battlefield seems weird!", "Bojiště vypadá podivně!")),
    [STRINGID_WAGGLINGAFINGER]                      = COMPOUND_STRING(BATTLE_TEXT("Waggling a finger let it use {B_CURRENT_MOVE}!", "Zamával prstem a použil útok {B_CURRENT_MOVE}!")),
    [STRINGID_BLOCKEDBYSLEEPCLAUSE]                 = COMPOUND_STRING(BATTLE_TEXT("Sleep Clause kept {B_DEF_NAME_WITH_PREFIX2} awake!", "Pravidlo spánku: {B_DEF_NAME_WITH_PREFIX2} zůstává vzhůru!")),
    [STRINGID_SUPEREFFECTIVETWOFOES]                = COMPOUND_STRING(BATTLE_TEXT("It's super effective on {B_DEF_NAME_WITH_PREFIX2} and {B_DEF_PARTNER_NAME}!", "Je to velmi účinné! Cíle: {B_DEF_NAME_WITH_PREFIX2} a {B_DEF_PARTNER_NAME}!")),
    [STRINGID_NOTVERYEFFECTIVETWOFOES]              = COMPOUND_STRING(BATTLE_TEXT("It's not very effective on {B_DEF_NAME_WITH_PREFIX2} and {B_DEF_PARTNER_NAME}.", "Není to moc účinné. Cíle: {B_DEF_NAME_WITH_PREFIX2} a {B_DEF_PARTNER_NAME}.")),
    [STRINGID_ITDOESNTAFFECTTWOFOES]                = COMPOUND_STRING(BATTLE_TEXT("It doesn't affect {B_DEF_NAME_WITH_PREFIX2} and {B_DEF_PARTNER_NAME}…", "Bez účinku! Odolali: {B_DEF_NAME_WITH_PREFIX2} a {B_DEF_PARTNER_NAME}…")),
    [STRINGID_SENDCAUGHTMONPARTYORBOX]              = COMPOUND_STRING(BATTLE_TEXT("Add {B_DEF_NAME} to your party?", "Přidat do týmu Pokémona {B_DEF_NAME}?")),
    [STRINGID_PKMNSENTTOPCAFTERCATCH]               = gText_PkmnSentToPCAfterCatch,
    [STRINGID_PKMNDYNAMAXED]                        = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} grew huge into its Dynamax form!", "{B_ATK_NAME_WITH_PREFIX} vyrostl do obří formy Dynamax!")),
    [STRINGID_PKMNGIGANTAMAXED]                     = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} grew huge into its Gigantamax form!", "{B_ATK_NAME_WITH_PREFIX} vyrostl do obří formy Gigantamax!")),
    [STRINGID_TIMETODYNAMAX]                        = COMPOUND_STRING(BATTLE_TEXT("Time to Dynamax!", "Je čas na Dynamax!")),
    [STRINGID_TIMETOGIGANTAMAX]                     = COMPOUND_STRING(BATTLE_TEXT("Time to Gigantamax!", "Je čas na Gigantamax!")),
    [STRINGID_QUESTIONFORFEITBATTLE]                = COMPOUND_STRING(BATTLE_TEXT("Would you like to give up on this battle and quit now? Quitting the battle is the same as losing the battle.", "Chceš se vzdát a ukončit souboj? Ukončení souboje se počítá jako prohra.")),
    [STRINGID_POWERCONSTRUCTPRESENCEOFMANY]         = COMPOUND_STRING(BATTLE_TEXT("You sense the presence of many!", "Cítíš přítomnost mnoha dalších!")),
    [STRINGID_POWERCONSTRUCTTRANSFORM]              = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} transformed into its Complete Forme!", "{B_ATK_NAME_WITH_PREFIX} se proměnil do úplné formy!")),
    [STRINGID_ABILITYSHIELDPROTECTS]                = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX}'s Ability is protected by the effects of its {B_LAST_ITEM}!", "{B_ATK_NAME_WITH_PREFIX}: schopnost chrání účinek {B_LAST_ITEM}!")),
    [STRINGID_MONTOOSCAREDTOMOVE]                   = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} is too scared to move!", "{B_ATK_NAME_WITH_PREFIX} se strachem nemůže ani pohnout!")),
    [STRINGID_GHOSTGETOUTGETOUT]                    = COMPOUND_STRING(BATTLE_TEXT("GHOST: Get out…… Get out……", "DUCH: Zmiz…… Zmiz……")),
    [STRINGID_SILPHSCOPEUNVEILED]                   = COMPOUND_STRING(BATTLE_TEXT("SILPH SCOPE unveiled the GHOST's\nidentity!", "SILPH SCOPE odhalil, kdo se\nukrývá za duchem!")),
    [STRINGID_GHOSTWASMAROWAK]                      = COMPOUND_STRING(BATTLE_TEXT("The GHOST was MAROWAK!\p", "Ten DUCH byl MAROWAK!\p")),
    [STRINGID_TRAINER1MON1COMEBACK]                 = COMPOUND_STRING(BATTLE_TEXT("{B_TRAINER1_NAME}: {B_OPPONENT_MON1_NAME}, come back!", "{B_TRAINER1_NAME}: {B_OPPONENT_MON1_NAME}, vrať se!")),
    [STRINGID_THREWROCK]                            = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_NAME} threw a ROCK\nat the {B_OPPONENT_MON1_NAME}!", "{B_PLAYER_NAME} hodil KÁMEN.\nCíl: {B_OPPONENT_MON1_NAME}!")),
    [STRINGID_THREWBAIT]                            = COMPOUND_STRING(BATTLE_TEXT("{B_PLAYER_NAME} threw some BAIT\nat the {B_OPPONENT_MON1_NAME}!", "{B_PLAYER_NAME} hodil NÁVNADU.\nCíl: {B_OPPONENT_MON1_NAME}!")),
    [STRINGID_PKMNANGRY]                            = COMPOUND_STRING(BATTLE_TEXT("{B_OPPONENT_MON1_NAME} is angry!", "{B_OPPONENT_MON1_NAME} zuří!")),
    [STRINGID_PKMNEATING]                           = COMPOUND_STRING(BATTLE_TEXT("{B_OPPONENT_MON1_NAME} is eating!", "{B_OPPONENT_MON1_NAME} jí!")),
    [STRINGID_PKMNDISGUISEWASBUSTED]                = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s disguise was busted!", "{B_SCR_NAME_WITH_PREFIX}: jeho převlek byl odhalen!")),
    [STRINGID_ZENMODETRIGGERED]                     = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_ABILITY} triggered!", "Schopnost {B_SCR_ABILITY} se aktivovala!")),
    [STRINGID_ZENMODEENDED]                         = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_ABILITY} ended!", "Účinek schopnosti {B_SCR_ABILITY} skončil!")),
    [STRINGID_SCRCUREDPARALYSIS]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} was cured of paralysis!", "{B_SCR_NAME_WITH_PREFIX} se zbavil ochrnutí!")),
    [STRINGID_SCRCUREDPOISON]                       = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} was cured of its poisoning!", "{B_SCR_NAME_WITH_PREFIX} se zbavil otravy!")),
    [STRINGID_SCRCUREDBURN]                         = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s burn was cured!", "{B_SCR_NAME_WITH_PREFIX} se zbavil popálenin!")),
    [STRINGID_SCRCUREDSLEEP]                        = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} woke up!", "{B_SCR_NAME_WITH_PREFIX} se probudil!")),
    [STRINGID_SCRCUREDCONFUSION]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX} snapped out of its confusion!", "{B_SCR_NAME_WITH_PREFIX} se vzpamatoval ze zmatení!")),
    [STRINGID_PARTYCUREDPARALYSIS]                  = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} was cured of paralysis!", "{B_BUFF1} se zbavil ochrnutí!")),
    [STRINGID_PARTYCUREDPOISON]                     = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} was cured of its poisoning!", "{B_BUFF1} se zbavil otravy!")),
    [STRINGID_PARTYCUREDBURN]                       = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1}'s burn was cured!", "{B_BUFF1} se zbavil popálenin!")),
    [STRINGID_PARTYCUREDSLEEP]                      = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} woke up!", "{B_BUFF1} se probudil!")),
    [STRINGID_PARTYCUREDFREEZE]                     = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1} thawed out!", "{B_BUFF1} rozmrzl!")),
    [STRINGID_PARTYCUREDFROSTBITE]                  = COMPOUND_STRING(BATTLE_TEXT("{B_BUFF1}'s frostbite was cured!", "{B_BUFF1} se zbavil omrzlin!")),
    [STRINGID_PKMNATKNOTLOWERED]                    = COMPOUND_STRING(BATTLE_TEXT("{B_SCR_NAME_WITH_PREFIX}'s Attack was not lowered!", "{B_SCR_NAME_WITH_PREFIX}: statistika útoku neklesla!")),
    [STRINGID_VICTORYCATCH]                         = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_NAME} is weak!\nThrow a Poké Ball now!", "{B_DEF_NAME} je oslabený!\nTeď hoď Poké Ball!")),
    [STRINGID_CANTUSEMOVE]                          = COMPOUND_STRING(BATTLE_TEXT("This move can't be used!\p", "Tento útok nelze použít!\p")),
    [STRINGID_REFLECTWOREOFF]                       = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_TEAM1} side's Reflect wore off!", "{B_DEF_TEAM1} tým: účinek Reflect vyprchal!")),
    [STRINGID_LIGHTSCREENWOREOFF]                   = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_TEAM1} side's Light Screen wore off!", "{B_DEF_TEAM1} tým: účinek Light Screen vyprchal!")),
    [STRINGID_AURORAVEILWOREOFF]                    = COMPOUND_STRING(BATTLE_TEXT("{B_DEF_TEAM1} side's Aurora Veil wore off!", "{B_DEF_TEAM1} tým: účinek Aurora Veil vyprchal!")),
    [STRINGID_MOSTLYINEFFECTIVE]                    = COMPOUND_STRING(BATTLE_TEXT("It's mostly ineffective…", "Je to skoro neúčinné…")),
    [STRINGID_EXTREMELYEFFECTIVE]                   = COMPOUND_STRING(BATTLE_TEXT("It's extremely effective!", "Je to mimořádně účinné!")),
    [STRINGID_NOTVERYEFFECTIVEONDEF]                = COMPOUND_STRING(BATTLE_TEXT("It's not very effective on {B_DEF_NAME_WITH_PREFIX2}.", "Není to moc účinné. Cíl: {B_DEF_NAME_WITH_PREFIX2}.")),
    [STRINGID_SUPEREFFECTIVEONDEF]                  = COMPOUND_STRING(BATTLE_TEXT("It's super effective on {B_DEF_NAME_WITH_PREFIX2}!", "Je to velmi účinné! Cíl: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_MOSTLYINEFFECTIVEONDEF]               = COMPOUND_STRING(BATTLE_TEXT("It's mostly ineffective on {B_DEF_NAME_WITH_PREFIX2}.", "Je to skoro neúčinné. Cíl: {B_DEF_NAME_WITH_PREFIX2}.")),
    [STRINGID_EXTREMELYEFFECTIVEONDEF]              = COMPOUND_STRING(BATTLE_TEXT("It's extremely effective on {B_DEF_NAME_WITH_PREFIX2}!", "Je to mimořádně účinné! Cíl: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_EXTREMELYEFFECTIVETWOFOES]            = COMPOUND_STRING(BATTLE_TEXT("It's extremely effective on {B_DEF_NAME_WITH_PREFIX2} and {B_DEF_PARTNER_NAME}!", "Je to mimořádně účinné! Cíle: {B_DEF_NAME_WITH_PREFIX2} a {B_DEF_PARTNER_NAME}!")),
    [STRINGID_MOSTLYINEFFECTIVETWOFOES]             = COMPOUND_STRING(BATTLE_TEXT("It's mostly ineffective on {B_DEF_NAME_WITH_PREFIX2} and {B_DEF_PARTNER_NAME}.", "Je to skoro neúčinné. Cíle: {B_DEF_NAME_WITH_PREFIX2} a {B_DEF_PARTNER_NAME}.")),
    [STRINGID_CRITICALHITONDEF]                     = COMPOUND_STRING(BATTLE_TEXT("A critical hit on {B_DEF_NAME_WITH_PREFIX2}!", "Kritický zásah! Cíl: {B_DEF_NAME_WITH_PREFIX2}!")),
    [STRINGID_S]                                    = COMPOUND_STRING(BATTLE_TEXT("s", "")),
    [STRINGID_LOSTSOMEOFITSHP]                      = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} lost some of its HP!", "{B_ATK_NAME_WITH_PREFIX} ztratil část svých HP!")),
    [STRINGID_BELCHCANTUSE]                         = COMPOUND_STRING(BATTLE_TEXT("{B_ATK_NAME_WITH_PREFIX} hasn't eaten any held Berries, so it can't possibly belch!\p", "{B_ATK_NAME_WITH_PREFIX} zatím nesnědl žádnou drženou bobuli, takže nemůže použít Belch!\p")),
};

const u16 gTrainerUsedItemStringIds[] =
{
    STRINGID_PLAYERUSEDITEM, STRINGID_TRAINER1USEDITEM
};

const u16 gZEffectStringIds[] =
{
    [B_MSG_Z_RESET_STATS] = STRINGID_ZMOVERESETSSTATS,
    [B_MSG_Z_ALL_STATS_UP]= STRINGID_ZMOVEALLSTATSUP,
    [B_MSG_Z_BOOST_CRITS] = STRINGID_ZMOVEZBOOSTCRIT,
    [B_MSG_Z_FOLLOW_ME]   = STRINGID_PKMNCENTERATTENTION,
    [B_MSG_Z_RECOVER_HP]  = STRINGID_ZMOVERESTOREHP,
    [B_MSG_Z_STAT_UP]     = STRINGID_ZMOVESTATUP,
    [B_MSG_Z_HP_TRAP]     = STRINGID_ZMOVEHPTRAP,
};

const u16 gMentalHerbCureStringIds[] =
{
    [B_MSG_MENTALHERBCURE_INFATUATION] = STRINGID_ATKGOTOVERINFATUATION,
    [B_MSG_MENTALHERBCURE_TORMENT]     = STRINGID_TORMENTEDNOMORE,
    [B_MSG_MENTALHERBCURE_DISABLE]     = STRINGID_PKMNMOVEDISABLEDNOMORE,
    [B_MSG_MENTALHERBCURE_HEALBLOCK]   = STRINGID_HEALBLOCKEDNOMORE,
    [B_MSG_MENTALHERBCURE_ENCORE]      = STRINGID_PKMNENCOREENDED,
    [B_MSG_MENTALHERBCURE_TAUNT]       = STRINGID_PKMNSHOOKOFFTHETAUNT,
};

const u16 gStartingStatusStringIds[B_MSG_STARTING_STATUS_COUNT] =
{
    [B_MSG_TERRAIN_SET_MISTY]    = STRINGID_TERRAINBECOMESMISTY,
    [B_MSG_TERRAIN_SET_ELECTRIC] = STRINGID_TERRAINBECOMESELECTRIC,
    [B_MSG_TERRAIN_SET_PSYCHIC]  = STRINGID_TERRAINBECOMESPSYCHIC,
    [B_MSG_TERRAIN_SET_GRASSY]   = STRINGID_TERRAINBECOMESGRASSY,
    [B_MSG_SET_TRICK_ROOM]       = STRINGID_DIMENSIONSWERETWISTED,
    [B_MSG_SET_MAGIC_ROOM]       = STRINGID_BIZARREARENACREATED,
    [B_MSG_SET_WONDER_ROOM]      = STRINGID_BIZARREAREACREATED,
    [B_MSG_SET_TAILWIND]         = STRINGID_TAILWINDBLEW,
    [B_MSG_SET_RAINBOW]          = STRINGID_ARAINBOWAPPEAREDONSIDE,
    [B_MSG_SET_SEA_OF_FIRE]      = STRINGID_SEAOFFIREENVELOPEDSIDE,
    [B_MSG_SET_SWAMP]            = STRINGID_SWAMPENVELOPEDSIDE,
    [B_MSG_SET_SPIKES]           = STRINGID_SPIKESSCATTERED,
    [B_MSG_SET_POISON_SPIKES]    = STRINGID_POISONSPIKESSCATTERED,
    [B_MSG_SET_STICKY_WEB]       = STRINGID_STICKYWEBUSED,
    [B_MSG_SET_STEALTH_ROCK]     = STRINGID_POINTEDSTONESFLOAT,
    [B_MSG_SET_SHARP_STEEL]      = STRINGID_SHARPSTEELFLOATS,
};

const u16 gTerrainStringIds[B_MSG_TERRAIN_COUNT] =
{
    [B_MSG_TERRAIN_SET_MISTY] = STRINGID_TERRAINBECOMESMISTY,
    [B_MSG_TERRAIN_SET_ELECTRIC] = STRINGID_TERRAINBECOMESELECTRIC,
    [B_MSG_TERRAIN_SET_PSYCHIC] = STRINGID_TERRAINBECOMESPSYCHIC,
    [B_MSG_TERRAIN_SET_GRASSY] = STRINGID_TERRAINBECOMESGRASSY,
    [B_MSG_TERRAIN_END_MISTY] = STRINGID_MISTYTERRAINENDS,
    [B_MSG_TERRAIN_END_ELECTRIC] = STRINGID_ELECTRICTERRAINENDS,
    [B_MSG_TERRAIN_END_PSYCHIC] = STRINGID_PSYCHICTERRAINENDS,
    [B_MSG_TERRAIN_END_GRASSY] = STRINGID_GRASSYTERRAINENDS,
};

const u16 gTerrainPreventsStringIds[] =
{
    [B_MSG_TERRAINPREVENTS_MISTY]    = STRINGID_MISTYTERRAINPREVENTS,
    [B_MSG_TERRAINPREVENTS_ELECTRIC] = STRINGID_ELECTRICTERRAINPREVENTS,
    [B_MSG_TERRAINPREVENTS_PSYCHIC]  = STRINGID_PSYCHICTERRAINPREVENTS
};

const u16 gHealingWishStringIds[] =
{
    STRINGID_HEALINGWISHCAMETRUE,
    STRINGID_LUNARDANCECAMETRUE
};

const u16 gDmgHazardsStringIds[] =
{
    [B_MSG_PKMNHURTBYSPIKES]   = STRINGID_PKMNHURTBYSPIKES,
    [B_MSG_STEALTHROCKDMG]     = STRINGID_STEALTHROCKDMG,
    [B_MSG_SHARPSTEELDMG]      = STRINGID_SHARPSTEELDMG,
    [B_MSG_POINTEDSTONESFLOAT] = STRINGID_POINTEDSTONESFLOAT,
    [B_MSG_SPIKESSCATTERED]    = STRINGID_SPIKESSCATTERED,
    [B_MSG_SHARPSTEELFLOATS]   = STRINGID_SHARPSTEELFLOATS,
};

const u16 gSwitchInAbilityStringIds[] =
{
    [B_MSG_SWITCHIN_MOLDBREAKER] = STRINGID_MOLDBREAKERENTERS,
    [B_MSG_SWITCHIN_TERAVOLT] = STRINGID_TERAVOLTENTERS,
    [B_MSG_SWITCHIN_TURBOBLAZE] = STRINGID_TURBOBLAZEENTERS,
    [B_MSG_SWITCHIN_SLOWSTART] = STRINGID_SLOWSTARTENTERS,
    [B_MSG_SWITCHIN_UNNERVE] = STRINGID_UNNERVEENTERS,
    [B_MSG_SWITCHIN_ANTICIPATION] = STRINGID_ANTICIPATIONACTIVATES,
    [B_MSG_SWITCHIN_FOREWARN] = STRINGID_FOREWARNACTIVATES,
    [B_MSG_SWITCHIN_PRESSURE] = STRINGID_PRESSUREENTERS,
    [B_MSG_SWITCHIN_DARKAURA] = STRINGID_DARKAURAENTERS,
    [B_MSG_SWITCHIN_FAIRYAURA] = STRINGID_FAIRYAURAENTERS,
    [B_MSG_SWITCHIN_AURABREAK] = STRINGID_AURABREAKENTERS,
    [B_MSG_SWITCHIN_COMATOSE] = STRINGID_COMATOSEENTERS,
    [B_MSG_SWITCHIN_SCREENCLEANER] = STRINGID_SCREENCLEANERENTERS,
    [B_MSG_SWITCHIN_ASONE] = STRINGID_ASONEENTERS,
    [B_MSG_SWITCHIN_CURIOUS_MEDICINE] = STRINGID_CURIOUSMEDICINEENTERS,
    [B_MSG_SWITCHIN_PASTEL_VEIL] = STRINGID_PKMNHEALEDPOISON,
    [B_MSG_SWITCHIN_NEUTRALIZING_GAS] = STRINGID_NEUTRALIZINGGASENTERS,
};

const u16 gNoEscapeStringIds[] =
{
    [B_MSG_CANT_ESCAPE]          = STRINGID_CANTESCAPE,
    [B_MSG_DONT_LEAVE_BIRCH]     = STRINGID_DONTLEAVEBIRCH,
    [B_MSG_PREVENTS_ESCAPE]      = STRINGID_PREVENTSESCAPE,
    [B_MSG_CANT_ESCAPE_2]        = STRINGID_CANTESCAPE2,
    [B_MSG_ATTACKER_CANT_ESCAPE] = STRINGID_ATTACKERCANTESCAPE
};

const u16 gMoveWeatherChangeStringIds[] =
{
    [B_MSG_STARTED_RAIN]      = STRINGID_STARTEDTORAIN,
    [B_MSG_STARTED_DOWNPOUR]  = STRINGID_DOWNPOURSTARTED, // Unused
    [B_MSG_WEATHER_FAILED]    = STRINGID_BUTITFAILED,
    [B_MSG_STARTED_SANDSTORM] = STRINGID_SANDSTORMBREWED,
    [B_MSG_STARTED_SUNLIGHT]  = STRINGID_SUNLIGHTGOTBRIGHT,
    [B_MSG_STARTED_HAIL]      = STRINGID_STARTEDHAIL,
    [B_MSG_STARTED_SNOW]      = STRINGID_STARTEDSNOW,
    [B_MSG_STARTED_FOG]       = STRINGID_FOGCREPTUP, // Unused, can use for custom moves that set fog
};

const u16 gAbilityWeatherChangeStringId[] =
{
    [B_MSG_STARTED_DRIZZLE]        = STRINGID_STARTEDTORAIN,
    [B_MSG_STARTED_SAND_STREAM]    = STRINGID_SANDSTORMBREWED,
    [B_MSG_STARTED_DROUGHT]        = STRINGID_SUNLIGHTGOTBRIGHT,
    [B_MSG_STARTED_HAIL_WARNING]   = STRINGID_STARTEDHAIL,
    [B_MSG_STARTED_SNOW_WARNING]   = STRINGID_STARTEDSNOW,
    [B_MSG_STARTED_DESOLATE_LAND]  = STRINGID_EXTREMELYHARSHSUNLIGHT,
    [B_MSG_STARTED_PRIMORDIAL_SEA] = STRINGID_HEAVYRAIN,
    [B_MSG_STARTED_STRONG_WINDS]   = STRINGID_MYSTERIOUSAIRCURRENT,
};

const u16 gWeatherEndsStringIds[B_MSG_WEATHER_END_COUNT] =
{
    [B_MSG_WEATHER_END_RAIN]                       = STRINGID_RAINSTOPPED,
    [B_MSG_WEATHER_END_SUN]                        = STRINGID_SUNLIGHTFADED,
    [B_MSG_WEATHER_END_SANDSTORM]                  = STRINGID_SANDSTORMSUBSIDED,
    [B_MSG_WEATHER_END_HAIL]                       = STRINGID_HAILSTOPPED,
    [B_MSG_WEATHER_END_SNOW]                       = STRINGID_SNOWSTOPPED,
    [B_MSG_WEATHER_END_FOG]                        = STRINGID_FOGLIFTED,
    [B_MSG_WEATHER_END_EXTREMELY_HARSH_SUNLIGHT]   = STRINGID_EXTREMESUNLIGHTFADED,
    [B_MSG_WEATHER_END_HEAVY_RAIN]                 = STRINGID_HEAVYRAINLIFTED,
    [B_MSG_WEATHER_END_STRONG_WINDS]               = STRINGID_STRONGWINDSDISSIPATED,
};

const u16 gWeatherTurnStringIds[] =
{
    [B_MSG_WEATHER_TURN_RAIN]         = STRINGID_RAINCONTINUES,
    [B_MSG_WEATHER_TURN_DOWNPOUR]     = STRINGID_DOWNPOURCONTINUES,
    [B_MSG_WEATHER_TURN_SUN]          = STRINGID_SUNLIGHTSTRONG,
    [B_MSG_WEATHER_TURN_SANDSTORM]    = STRINGID_SANDSTORMRAGES,
    [B_MSG_WEATHER_TURN_HAIL]         = STRINGID_HAILCONTINUES,
    [B_MSG_WEATHER_TURN_SNOW]         = STRINGID_SNOWCONTINUES,
    [B_MSG_WEATHER_TURN_FOG]          = STRINGID_FOGISDEEP,
    [B_MSG_WEATHER_TURN_STRONG_WINDS] = STRINGID_MYSTERIOUSAIRCURRENTBLOWSON,
};

const u16 gSandStormHailDmgStringIds[] =
{
    [B_MSG_SANDSTORM] = STRINGID_PKMNBUFFETEDBYSANDSTORM,
    [B_MSG_HAIL]      = STRINGID_PKMNPELTEDBYHAIL
};

const u16 gProtectLikeUsedStringIds[] =
{
    [B_MSG_PROTECTED_ITSELF] = STRINGID_PKMNPROTECTEDITSELF2,
    [B_MSG_BRACED_ITSELF]    = STRINGID_PKMNBRACEDITSELF,
    [B_MSG_PROTECTED_TEAM]   = STRINGID_PROTECTEDTEAM,
};

const u16 gBrokeProtectionStringIds[] =
{
    [B_MSG_FEINT]                 = STRINGID_FELLFORFEINT,
    [B_MSG_BROKE_THROUGH_PROTECT] = STRINGID_BROKETHROUGHPROTECTION,
};

const u16 gReflectLightScreenSafeguardStringIds[] =
{
    [B_MSG_SIDE_STATUS_FAILED]     = STRINGID_BUTITFAILED,
    [B_MSG_SET_REFLECT_SINGLE]     = STRINGID_PKMNRAISEDDEF,
    [B_MSG_SET_REFLECT_DOUBLE]     = STRINGID_PKMNRAISEDDEF,
    [B_MSG_SET_LIGHTSCREEN_SINGLE] = STRINGID_PKMNRAISEDSPDEF,
    [B_MSG_SET_LIGHTSCREEN_DOUBLE] = STRINGID_PKMNRAISEDSPDEF,
    [B_MSG_SET_SAFEGUARD]          = STRINGID_PKMNCOVEREDBYVEIL,
    [B_MSG_SET_AURORA_VEIL]        = STRINGID_PKMNAURORAVEIL,
};

const u16 gLeechSeedStringIds[] =
{
    [B_MSG_LEECH_SEED_SET]   = STRINGID_PKMNSEEDED,
    [B_MSG_LEECH_SEED_MISS]  = STRINGID_PKMNAVOIDEDATTACK,
    [B_MSG_LEECH_SEED_FAIL]  = STRINGID_ITDOESNTAFFECT,
    [B_MSG_LEECH_SEED_DRAIN] = STRINGID_PKMNSAPPEDBYLEECHSEED,
    [B_MSG_LEECH_SEED_OOZE]  = STRINGID_ITSUCKEDLIQUIDOOZE,
};

const u16 gRestUsedStringIds[] =
{
    [B_MSG_REST]          = STRINGID_PKMNWENTTOSLEEP,
    [B_MSG_REST_STATUSED] = STRINGID_PKMNSLEPTHEALTHY
};

const u16 gUproarOverTurnStringIds[] =
{
    [B_MSG_UPROAR_CONTINUES] = STRINGID_PKMNMAKINGUPROAR,
    [B_MSG_UPROAR_ENDS]      = STRINGID_PKMNCALMEDDOWN
};

const u16 gWokeUpStringIds[] =
{
    [B_MSG_WOKE_UP]        = STRINGID_PKMNWOKEUP,
    [B_MSG_WOKE_UP_UPROAR] = STRINGID_PKMNWOKEUPINUPROAR
};

const u16 gUproarAwakeStringIds[] =
{
    [B_MSG_CANT_SLEEP_UPROAR]  = STRINGID_PKMNCANTSLEEPINUPROAR2,
    [B_MSG_UPROAR_KEPT_AWAKE]  = STRINGID_UPROARKEPTPKMNAWAKE,
};

const u16 gStatUpStringIds[] =
{
    [B_MSG_STAT_CHANGED]            = STRINGID_STATROSE,
    [B_MSG_STAT_WONT_CHANGE]        = STRINGID_STATSWONTINCREASE,
    [B_MSG_STAT_MAXED]              = STRINGID_STATWASMAXEDOUT,
    [B_MSG_STAT_CHANGE_EMPTY]       = STRINGID_EMPTYSTRING3,
    [B_MSG_STAT_CHANGED_ITEM]       = STRINGID_USINGITEMSTATOFPKMNROSE,
    [B_MSG_STAT_CHANGED_BELLY_DRUM] = STRINGID_PKMNCUTHPMAXEDATTACK,
    [B_MSG_USED_DIRE_HIT]           = STRINGID_PKMNUSEDXTOGETPUMPED,
};

// Mostly redundant, combine with above in a future pr
const u16 gStatDownStringIds[] =
{
    [B_MSG_STAT_CHANGED]            = STRINGID_STATFELL,
    [B_MSG_STAT_WONT_CHANGE]        = STRINGID_STATSWONTDECREASE,
    [B_MSG_STAT_CHANGE_EMPTY]       = STRINGID_EMPTYSTRING3,
    [B_MSG_STAT_CHANGED_ITEM]       = STRINGID_USINGITEMSTATOFPKMNFELL,
    [B_MSG_STAT_CHANGED_BELLY_DRUM] = STRINGID_PKMNCUTHPMAXEDATTACK, // Message for contrary is still printed
};

// Index copied from move's index in sTrappingMoves
const u16 gWrappedStringIds[NUM_TRAPPING_MOVES] =
{
    [B_MSG_WRAPPED_BIND]        = STRINGID_PKMNSQUEEZEDBYBIND,     // MOVE_BIND
    [B_MSG_WRAPPED_WRAP]        = STRINGID_PKMNWRAPPEDBY,          // MOVE_WRAP
    [B_MSG_WRAPPED_FIRE_SPIN]   = STRINGID_PKMNTRAPPEDINVORTEX,    // MOVE_FIRE_SPIN
    [B_MSG_WRAPPED_CLAMP]       = STRINGID_PKMNCLAMPED,            // MOVE_CLAMP
    [B_MSG_WRAPPED_WHIRLPOOL]   = STRINGID_PKMNTRAPPEDINVORTEX,    // MOVE_WHIRLPOOL
    [B_MSG_WRAPPED_SAND_TOMB]   = STRINGID_PKMNTRAPPEDBYSANDTOMB,  // MOVE_SAND_TOMB
    [B_MSG_WRAPPED_MAGMA_STORM] = STRINGID_TRAPPEDBYSWIRLINGMAGMA, // MOVE_MAGMA_STORM
    [B_MSG_WRAPPED_INFESTATION] = STRINGID_INFESTATION,            // MOVE_INFESTATION
    [B_MSG_WRAPPED_SNAP_TRAP]   = STRINGID_PKMNINSNAPTRAP,         // MOVE_SNAP_TRAP
    [B_MSG_WRAPPED_THUNDER_CAGE]= STRINGID_THUNDERCAGETRAPPED,     // MOVE_THUNDER_CAGE
};

const u16 gMistUsedStringIds[] =
{
    [B_MSG_SET_MIST]    = STRINGID_PKMNSHROUDEDINMIST,
    [B_MSG_MIST_FAILED] = STRINGID_BUTITFAILED
};

const u16 gFocusEnergyUsedStringIds[] =
{
    [B_MSG_GETTING_PUMPED]      = STRINGID_PKMNGETTINGPUMPED,
    [B_MSG_FOCUS_ENERGY_FAILED] = STRINGID_BUTITFAILED
};

const u16 gTransformUsedStringIds[] =
{
    [B_MSG_TRANSFORMED]      = STRINGID_PKMNTRANSFORMEDINTO,
    [B_MSG_TRANSFORM_FAILED] = STRINGID_BUTITFAILED
};

const u16 gSubstituteUsedStringIds[] =
{
    [B_MSG_SET_SUBSTITUTE]    = STRINGID_PKMNMADESUBSTITUTE,
    [B_MSG_SUBSTITUTE_FAILED] = STRINGID_TOOWEAKFORSUBSTITUTE
};

const u16 gGotPoisonedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASPOISONED,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNPOISONEDBY
};

const u16 gGotParalyzedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASPARALYZED,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNWASPARALYZEDBY
};

const u16 gFellAsleepStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNFELLASLEEP,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNMADESLEEP,
};

const u16 gGotBurnedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASBURNED,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNBURNEDBY
};

const u16 gGotFrostbiteStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNGOTFROSTBITE,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNGOTFROSTBITE,
};

const u16 gFrostbiteHealedStringIds[] =
{
    [B_MSG_FROSTBITE_HEALED]         = STRINGID_PKMNFROSTBITEHEALED,
    [B_MSG_FROSTBITE_HEALED_BY_MOVE] = STRINGID_PKMNFROSTBITEHEALEDBY
};

const u16 gGotFrozenStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASFROZEN,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNFROZENBY
};

const u16 gGotDefrostedStringIds[] =
{
    [B_MSG_DEFROSTED]         = STRINGID_PKMNWASDEFROSTED,
    [B_MSG_DEFROSTED_BY_MOVE] = STRINGID_PKMNWASDEFROSTEDBY
};

const u16 gKOFailedStringIds[] =
{
    [B_MSG_KO_MISS]       = STRINGID_PKMNAVOIDEDATTACK,
    [B_MSG_KO_UNAFFECTED] = STRINGID_PKMNUNAFFECTED
};

const u16 gAttractUsedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNFELLINLOVE,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNSXINFATUATEDY
};

const u16 gAbsorbDrainStringIds[] =
{
    [B_MSG_ABSORB]      = STRINGID_PKMNENERGYDRAINED,
    [B_MSG_ABSORB_OOZE] = STRINGID_ITSUCKEDLIQUIDOOZE
};

const u16 gSportsUsedStringIds[] =
{
    [B_MSG_WEAKEN_ELECTRIC] = STRINGID_ELECTRICITYWEAKENED,
    [B_MSG_WEAKEN_FIRE]     = STRINGID_FIREWEAKENED
};

const u16 gPartyStatusHealStringIds[] =
{
    [B_MSG_BELL]                     = STRINGID_BELLCHIMED,
    [B_MSG_BELL_SOUNDPROOF_ATTACKER] = STRINGID_BELLCHIMED,
    [B_MSG_BELL_SOUNDPROOF_PARTNER]  = STRINGID_BELLCHIMED,
    [B_MSG_BELL_BOTH_SOUNDPROOF]     = STRINGID_BELLCHIMED,
    [B_MSG_SOOTHING_AROMA]           = STRINGID_SOOTHINGAROMA
};

const u16 gFutureMoveUsedStringIds[] =
{
    [B_MSG_FUTURE_SIGHT] = STRINGID_PKMNFORESAWATTACK,
    [B_MSG_DOOM_DESIRE]  = STRINGID_PKMNCHOSEXASDESTINY
};

const u16 gBallEscapeStringIds[] =
{
    [BALL_NO_SHAKES]     = STRINGID_PKMNBROKEFREE,
    [BALL_1_SHAKE]       = STRINGID_ITAPPEAREDCAUGHT,
    [BALL_2_SHAKES]      = STRINGID_AARGHALMOSTHADIT,
    [BALL_3_SHAKES_FAIL] = STRINGID_SHOOTSOCLOSE
};

// Overworld weathers that don't have an associated battle weather default to "It is raining."
const u16 gWeatherStartsStringIds[] =
{
    [WEATHER_NONE]               = STRINGID_ITISRAINING,
    [WEATHER_SUNNY_CLOUDS]       = STRINGID_ITISRAINING,
    [WEATHER_SUNNY]              = STRINGID_ITISRAINING,
    [WEATHER_RAIN]               = STRINGID_ITISRAINING,
    [WEATHER_SNOW]               = (B_OVERWORLD_SNOW >= GEN_9 ? STRINGID_ITISSNOWING : STRINGID_ITISHAILING),
    [WEATHER_RAIN_THUNDERSTORM]  = STRINGID_ITISRAINING,
    [WEATHER_FOG_HORIZONTAL]     = STRINGID_FOGISDEEP,
    [WEATHER_VOLCANIC_ASH]       = STRINGID_ITISRAINING,
    [WEATHER_SANDSTORM]          = STRINGID_SANDSTORMISRAGING,
    [WEATHER_FOG_DIAGONAL]       = STRINGID_FOGISDEEP,
    [WEATHER_UNDERWATER]         = STRINGID_ITISRAINING,
    [WEATHER_SHADE]              = STRINGID_ITISRAINING,
    [WEATHER_DROUGHT]            = STRINGID_SUNLIGHTISHARSH,
    [WEATHER_DOWNPOUR]           = STRINGID_ITISRAINING,
    [WEATHER_UNDERWATER_BUBBLES] = STRINGID_ITISRAINING,
    [WEATHER_ABNORMAL]           = STRINGID_ITISRAINING
};

const u16 gTerrainStartsStringIds[] =
{
    [B_MSG_TERRAIN_SET_MISTY]    = STRINGID_MISTSWIRLSAROUND,
    [B_MSG_TERRAIN_SET_ELECTRIC] = STRINGID_ELECTRICCURRENTISRUNNING,
    [B_MSG_TERRAIN_SET_PSYCHIC]  = STRINGID_SEEMSWEIRD,
    [B_MSG_TERRAIN_SET_GRASSY]   = STRINGID_ISCOVEREDWITHGRASS,
};

const u16 gPrimalWeatherBlocksStringIds[] =
{
    [B_MSG_PRIMAL_WEATHER_FIZZLED_BY_RAIN]      = STRINGID_MOVEFIZZLEDOUTINTHEHEAVYRAIN,
    [B_MSG_PRIMAL_WEATHER_EVAPORATED_IN_SUN]    = STRINGID_MOVEEVAPORATEDINTHEHARSHSUNLIGHT,
};

const u16 gInobedientStringIds[] =
{
    [B_MSG_LOAFING]            = STRINGID_PKMNLOAFING,
    [B_MSG_WONT_OBEY]          = STRINGID_PKMNWONTOBEY,
    [B_MSG_TURNED_AWAY]        = STRINGID_PKMNTURNEDAWAY,
    [B_MSG_PRETEND_NOT_NOTICE] = STRINGID_PKMNPRETENDNOTNOTICE,
    [B_MSG_INCAPABLE_OF_POWER] = STRINGID_PKMNINCAPABLEOFPOWER
};

const u16 gSafariReactionStringIds[NUM_SAFARI_REACTIONS] =
{
    [B_MSG_MON_WATCHING] = STRINGID_PKMNWATCHINGCAREFULLY,
    [B_MSG_MON_ANGRY]    = STRINGID_PKMNANGRY,
    [B_MSG_MON_EATING]   = STRINGID_PKMNEATING
};

const u16 gSafariGetNearStringIds[] =
{
    [B_MSG_CREPT_CLOSER]    = STRINGID_CREPTCLOSER,
    [B_MSG_CANT_GET_CLOSER] = STRINGID_CANTGETCLOSER
};

const u16 gSafariPokeblockResultStringIds[] =
{
    [B_MSG_MON_CURIOUS]    = STRINGID_PKMNCURIOUSABOUTX,
    [B_MSG_MON_ENTHRALLED] = STRINGID_PKMNENTHRALLEDBYX,
    [B_MSG_MON_IGNORED]    = STRINGID_PKMNIGNOREDX
};

const u16 CureStatusBerryEffectStringID[] =
{
    [B_MSG_CURED_PARALYSIS] = STRINGID_PKMNSITEMCUREDPARALYSIS,
    [B_MSG_CURED_POISON] = STRINGID_PKMNSITEMCUREDPOISON,
    [B_MSG_CURED_BURN] = STRINGID_PKMNSITEMHEALEDBURN,
    [B_MSG_CURED_FREEZE] = STRINGID_PKMNSITEMDEFROSTEDIT,
    [B_MSG_CURED_FROSTBITE] = STRINGID_PKMNSITEMHEALEDFROSTBITE,
    [B_MSG_CURED_SLEEP] = STRINGID_PKMNSITEMWOKEIT,
    [B_MSG_CURED_CONFUSION] = STRINGID_PKMNSITEMSNAPPEDOUT,
};

const u16 gItemSwapStringIds[] =
{
    [B_MSG_ITEM_SWAP_TAKEN] = STRINGID_PKMNOBTAINEDX,
    [B_MSG_ITEM_SWAP_GIVEN] = STRINGID_PKMNOBTAINEDX2,
    [B_MSG_ITEM_SWAP_BOTH]  = STRINGID_PKMNOBTAINEDXYOBTAINEDZ
};

const u16 gFlashFireStringIds[] =
{
    [B_MSG_FLASH_FIRE_BOOST]    = STRINGID_PKMNRAISEDFIREPOWERWITH,
    [B_MSG_FLASH_FIRE_NO_BOOST] = STRINGID_PKMNSXMADEITINEFFECTIVE
};

const u16 gCaughtMonStringIds[] =
{
    [B_MSG_SENT_SOMEONES_PC]   = STRINGID_PKMNTRANSFERREDSOMEONESPC,
    [B_MSG_SENT_LANETTES_PC]   = STRINGID_PKMNTRANSFERREDLANETTESPC,
    [B_MSG_SOMEONES_BOX_FULL]  = STRINGID_PKMNBOXSOMEONESPCFULL,
    [B_MSG_LANETTES_BOX_FULL]  = STRINGID_PKMNBOXLANETTESPCFULL,
    [B_MSG_SWAPPED_INTO_PARTY] = STRINGID_PKMNSENTTOPCAFTERCATCH,
};

const u16 gRoomsStringIds[] =
{
    STRINGID_PKMNTWISTEDDIMENSIONS, STRINGID_TRICKROOMENDS,
    STRINGID_SWAPSDEFANDSPDEFOFALLPOKEMON, STRINGID_WONDERROOMENDS,
    STRINGID_HELDITEMSLOSEEFFECTS, STRINGID_MAGICROOMENDS,
    STRINGID_EMPTYSTRING3
};

const u16 gStatusConditionsStringIds[] =
{
    STRINGID_PKMNWASPOISONED, STRINGID_PKMNBADLYPOISONED, STRINGID_PKMNWASBURNED, STRINGID_PKMNWASPARALYZED, STRINGID_PKMNFELLASLEEP, STRINGID_PKMNGOTFROSTBITE
};

const u16 gDamageNonTypesStartStringIds[] =
{
    [B_MSG_TRAPPED_WITH_VINES]  = STRINGID_TEAMTRAPPEDWITHVINES,
    [B_MSG_CAUGHT_IN_VORTEX]    = STRINGID_TEAMCAUGHTINVORTEX,
    [B_MSG_SURROUNDED_BY_FIRE]  = STRINGID_TEAMSURROUNDEDBYFIRE,
    [B_MSG_SURROUNDED_BY_ROCKS] = STRINGID_TEAMSURROUNDEDBYROCKS,
};

const u16 gDamageNonTypesDmgStringIds[] =
{
    [B_MSG_HURT_BY_VINES]        = STRINGID_PKMNHURTBYVINES,
    [B_MSG_HURT_BY_VORTEX]       = STRINGID_PKMNHURTBYVORTEX,
    [B_MSG_BURNING_UP]           = STRINGID_PKMNBURNINGUP,
    [B_MSG_HURT_BY_ROCKS_THROWN] = STRINGID_PKMNHURTBYROCKSTHROWN,
};

const u16 gRemoveHazardsStringIds[] =
{
    [HAZARDS_SPIKES] = STRINGID_SPIKESDISAPPEAREDFROMTEAM,
    [HAZARDS_STICKY_WEB] = STRINGID_STICKYWEBDISAPPEAREDFROMTEAM,
    [HAZARDS_TOXIC_SPIKES] = STRINGID_TOXICSPIKESDISAPPEAREDFROMTEAM,
    [HAZARDS_STEALTH_ROCK] = STRINGID_STEALTHROCKDISAPPEAREDFROMTEAM,
    [HAZARDS_STEELSURGE] = STRINGID_SHARPSTEELDISAPPEAREDFROMTEAM,
};

const u16 gZenModeStringIds[] =
{
    [B_MSG_ZEN_MODE_TRIGGERED] = STRINGID_ZENMODETRIGGERED,
    [B_MSG_ZEN_MODE_ENDED] = STRINGID_ZENMODEENDED
};

const u16 gCureStatusStringIds[] =
{
    [B_MSG_CURED_PARALYSIS] = STRINGID_SCRCUREDPARALYSIS,
    [B_MSG_CURED_POISON] = STRINGID_SCRCUREDPOISON,
    [B_MSG_CURED_BURN] = STRINGID_SCRCUREDBURN,
    [B_MSG_CURED_SLEEP] = STRINGID_SCRCUREDSLEEP,
    [B_MSG_CURED_FREEZE] = STRINGID_PKMNWASDEFROSTED,
    [B_MSG_CURED_FROSTBITE] = STRINGID_PKMNFROSTBITEHEALED,
    [B_MSG_CURED_CONFUSION] = STRINGID_SCRCUREDCONFUSION,
    [B_MSG_CURED_INFATUATION] = STRINGID_PKMNGOTOVERITSINFATUATION,
    [B_MSG_CURED_TAUNT] = STRINGID_PKMNSHOOKOFFTHETAUNT,
};

const u16 gPartyCureStatusStringIds[] =
{
    [B_MSG_CURED_PARALYSIS] = STRINGID_PARTYCUREDPARALYSIS,
    [B_MSG_CURED_POISON] = STRINGID_PARTYCUREDPOISON,
    [B_MSG_CURED_BURN] = STRINGID_PARTYCUREDBURN,
    [B_MSG_CURED_SLEEP] = STRINGID_PARTYCUREDSLEEP,
    [B_MSG_CURED_FREEZE] = STRINGID_PARTYCUREDFREEZE,
    [B_MSG_CURED_FROSTBITE] = STRINGID_PARTYCUREDFROSTBITE,
    [B_MSG_CURED_CONFUSION] = STRINGID_SCRCUREDCONFUSION,
    [B_MSG_CURED_INFATUATION] = STRINGID_PKMNGOTOVERITSINFATUATION,
    [B_MSG_CURED_TAUNT] = STRINGID_PKMNSHOOKOFFTHETAUNT,
};

const u16 gHurtByStringIds[] =
{
    [B_MSG_HURT] = STRINGID_PKMNWASHURT,
    [B_MSG_HURT_BY_ITEM] = STRINGID_PKMNHURTSWITH,
};

const u16 gBreakScreensStringIds[] =
{
    [B_MSG_BREAK_REFLECT] = STRINGID_REFLECTWOREOFF,
    [B_MSG_BREAK_LIGHT_SCREEN] = STRINGID_LIGHTSCREENWOREOFF,
    [B_MSG_BREAK_AURORA_VEIL] = STRINGID_AURORAVEILWOREOFF,
};

const u8 gText_PkmnIsEvolving[] = _(BATTLE_TEXT("What?\n{STR_VAR_1} is evolving!", "Cože?\n{STR_VAR_1} se vyvíjí!"));
const u8 gText_CongratsPkmnEvolved[] = _(BATTLE_TEXT("Congratulations! Your {STR_VAR_1}\nevolved into {STR_VAR_2}!{WAIT_SE}\p", "Gratulujeme! Tvůj {STR_VAR_1}\nse vyvinul v {STR_VAR_2}!{WAIT_SE}\p"));
const u8 gText_PkmnStoppedEvolving[] = _(BATTLE_TEXT("Huh? {STR_VAR_1}\nstopped evolving!\p", "Cože? {STR_VAR_1}\npřestal s vývojem!\p"));
const u8 gText_EllipsisQuestionMark[] = _("……?\p");
const u8 gText_WhatWillPkmnDo[] = _(BATTLE_TEXT("What will\n{B_BUFF1} do?", "Co udělá\n{B_BUFF1}?"));
const u8 gText_WhatWillPkmnDo2[] = _(BATTLE_TEXT("What will\n{B_PLAYER_NAME} do?", "Co udělá\n{B_PLAYER_NAME}?"));
const u8 gText_WhatWillWallyDo[] = _(BATTLE_TEXT("What will\nWALLY do?", "Co udělá\nWALLY?"));
const u8 gText_LinkStandby[] = _(BATTLE_TEXT("{PAUSE 16}Link standby…", "{PAUSE 16}Čekání na spojení…"));
const u8 gText_BattleMenu[] = _("Boj{CLEAR_TO 56}Batoh\nPokémon{CLEAR_TO 56}Útěk");
const u8 gText_SafariZoneMenu[] = _("Ball{CLEAR_TO 56}{POKEBLOCK}\nBlíž{CLEAR_TO 56}Útěk");
const u8 gText_SafariZoneMenuFrlg[] = _(BATTLE_TEXT("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW 13 14 15}BALL{CLEAR_TO 56}BAIT\nROCK{CLEAR_TO 56}RUN", "{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW 13 14 15}BALL{CLEAR_TO 56}NÁVN.\nKÁMEN{CLEAR_TO 56}ÚTĚK"));
const u8 gText_MoveInterfacePP[] = _("PP ");
const u8 gText_MoveInterfaceType[] = _(BATTLE_TEXT("TYPE/", "TYP/"));
const u8 gText_MoveInterfacePPType[] = _(BATTLE_TEXT("{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}PP\nTYPE/", "{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}PP\nTYP/"));
const u8 gText_MoveInterfaceDynamicColors[] = _("{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}");
const u8 gText_WhichMoveToForget4[] = _(BATTLE_TEXT("{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}Which move should\nbe forgotten?", "{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}Který útok\nzapomenout?"));
const u8 gText_BattleCatchOrNot[] = _(BATTLE_TEXT("{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}Catch\nDon't catch", "{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}Chytit\nNechytit"));
const u8 gText_BattleYesNoChoice[] = _(BATTLE_TEXT("{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}Yes\nNo", "{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}Ano\nNe"));
const u8 gText_BattleSwitchWhich[] = _(BATTLE_TEXT("{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}Switch\nwhich?", "{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}Koho\nvyměnit?"));
const u8 gText_BattleSwitchWhich2[] = _("{PALETTE 5}{BACKGROUND DYNAMIC_COLOR5}{TEXT_COLORS DYNAMIC_COLOR4 DYNAMIC_COLOR6 DYNAMIC_COLOR5}");
const u8 gText_BattleSwitchWhich3[] = _("{UP_ARROW}");
const u8 gText_BattleSwitchWhich4[] = _("{ESCAPE 4}");
const u8 gText_BattleSwitchWhich5[] = _("-");
const u8 gText_SafariBalls[] = _(BATTLE_TEXT("Safari Balls", "Safari Bally"));
const u8 gText_SafariBallLeft[] = _(BATTLE_TEXT("Left: $", "Zbývá $"));
const u8 gText_Sleep[] = _(BATTLE_TEXT("sleep", "spánek"));
const u8 gText_Poison[] = _(BATTLE_TEXT("poison", "otrava"));
const u8 gText_Burn[] = _(BATTLE_TEXT("burn", "popálení"));
const u8 gText_Paralysis[] = _(BATTLE_TEXT("paralysis", "ochrnutí"));
const u8 gText_Ice[] = _(BATTLE_TEXT("ice", "led"));
const u8 gText_Confusion[] = _(BATTLE_TEXT("confusion", "zmatení"));
const u8 gText_Love[] = _(BATTLE_TEXT("love", "láska"));
const u8 gText_SpaceAndSpace[] = _(BATTLE_TEXT(" and ", " a "));
const u8 gText_CommaSpace[] = _(", ");
const u8 gText_Space2[] = _(" ");
const u8 gText_LineBreak[] = _("\l");
const u8 gText_NewLine[] = _("\n");
const u8 gText_Are[] = _(BATTLE_TEXT("are", "jsou"));
const u8 gText_Are2[] = _(BATTLE_TEXT("are", "jsou"));
const u8 gText_BadEgg[] = _(BATTLE_TEXT("Bad Egg", "Vadné vej."));
const u8 gText_BattleWallyName[] = _("WALLY");
const u8 gText_Win[] = _(BATTLE_TEXT("{BACKGROUND TRANSPARENT}{ACCENT TRANSPARENT}Win", "{BACKGROUND TRANSPARENT}{ACCENT TRANSPARENT}Výhra"));
const u8 gText_Loss[] = _(BATTLE_TEXT("{BACKGROUND TRANSPARENT}{ACCENT TRANSPARENT}Loss", "{BACKGROUND TRANSPARENT}{ACCENT TRANSPARENT}Prohra"));
const u8 gText_Draw[] = _(BATTLE_TEXT("{BACKGROUND TRANSPARENT}{ACCENT TRANSPARENT}Draw", "{BACKGROUND TRANSPARENT}{ACCENT TRANSPARENT}Remíza"));
static const u8 sText_SpaceIs[] = _(BATTLE_TEXT(" is", " je"));
static const u8 sText_ApostropheS[] = _(BATTLE_TEXT("'s", ":"));
const u8 gText_BattleTourney[] = _(BATTLE_TEXT("BATTLE TOURNEY", "BOJOVÝ TURNAJ"));

const u8 *const gRoundsStringTable[DOME_ROUNDS_COUNT] =
{
    [DOME_ROUND1]    = COMPOUND_STRING(BATTLE_TEXT("Round 1", "1. kolo")),
    [DOME_ROUND2]    = COMPOUND_STRING(BATTLE_TEXT("Round 2", "2. kolo")),
    [DOME_SEMIFINAL] = COMPOUND_STRING(BATTLE_TEXT("Semifinal", "Semifinále")),
    [DOME_FINAL]     = COMPOUND_STRING(BATTLE_TEXT("Final", "Finále")),
};

const u8 gText_TheGreatNewHope[] = _(BATTLE_TEXT("The great new hope!\p", "Velká nová naděje!\p"));
const u8 gText_WillChampionshipDreamComeTrue[] = _(BATTLE_TEXT("Will the championship dream come true?!\p", "Splní se sen o mistrovském titulu?!\p"));
const u8 gText_AFormerChampion[] = _(BATTLE_TEXT("A former champion!\p", "Bývalý šampion!\p"));
const u8 gText_ThePreviousChampion[] = _(BATTLE_TEXT("The previous champion!\p", "Předchozí šampion!\p"));
const u8 gText_TheUnbeatenChampion[] = _(BATTLE_TEXT("The unbeaten champion!\p", "Neporažený šampion!\p"));
const u8 gText_PlayerMon1Name[] = _("{B_PLAYER_MON1_NAME}");
const u8 gText_Vs[] = _("VS");
const u8 gText_OpponentMon1Name[] = _("{B_OPPONENT_MON1_NAME}");
const u8 gText_Mind[] = _(BATTLE_TEXT("Mind", "Duch"));
const u8 gText_Skill[] = _(BATTLE_TEXT("Skill", "Technika"));
const u8 gText_Body[] = _(BATTLE_TEXT("Body", "Tělo"));
const u8 gText_Judgment[] = _(BATTLE_TEXT("{B_BUFF1}{CLEAR 13}Judgment{CLEAR 13}{B_BUFF2}", "{B_BUFF1}{CLEAR 13}Verdikt{CLEAR 13}{B_BUFF2}"));
static const u8 sText_TwoTrainersSentPkmn[] = _(BATTLE_TEXT("{B_TRAINER1_NAME_WITH_CLASS} sent out {B_OPPONENT_MON1_NAME}!\p{B_TRAINER2_NAME_WITH_CLASS} sent out {B_OPPONENT_MON2_NAME}!", "{B_TRAINER1_NAME_WITH_CLASS} posílá do boje {B_OPPONENT_MON1_NAME}!\p{B_TRAINER2_NAME_WITH_CLASS} posílá do boje {B_OPPONENT_MON2_NAME}!"));
static const u8 sText_Trainer2SentOutPkmn[] = _(BATTLE_TEXT("{B_TRAINER2_NAME_WITH_CLASS} sent out {B_BUFF1}!", "{B_TRAINER2_NAME_WITH_CLASS} posílá do boje {B_BUFF1}!"));
static const u8 sText_TwoTrainersWantToBattle[] = _(BATTLE_TEXT("You are challenged by {B_TRAINER1_NAME_WITH_CLASS} and {B_TRAINER2_NAME_WITH_CLASS}!\p", "K souboji tě vyzývají {B_TRAINER1_NAME_WITH_CLASS} a {B_TRAINER2_NAME_WITH_CLASS}!\p"));
static const u8 sText_InGamePartnerSentOutZGoN[] = _(BATTLE_TEXT("{B_PARTNER_NAME_WITH_CLASS} sent out {B_PLAYER_MON2_NAME}! Go, {B_PLAYER_MON1_NAME}!", "{B_PARTNER_NAME_WITH_CLASS} posílá do boje {B_PLAYER_MON2_NAME}! Do toho, {B_PLAYER_MON1_NAME}!"));
static const u8 sText_InGamePartnerSentOutNGoZ[] = _(BATTLE_TEXT("{B_PARTNER_NAME_WITH_CLASS} sent out {B_PLAYER_MON1_NAME}! Go, {B_PLAYER_MON2_NAME}!", "{B_PARTNER_NAME_WITH_CLASS} posílá do boje {B_PLAYER_MON1_NAME}! Do toho, {B_PLAYER_MON2_NAME}!"));
static const u8 sText_InGamePartnerSentOutPkmn1[] = _(BATTLE_TEXT("{B_PARTNER_NAME_WITH_CLASS} sent out {B_PLAYER_MON1_NAME}!", "{B_PARTNER_NAME_WITH_CLASS} posílá do boje {B_PLAYER_MON1_NAME}!"));
static const u8 sText_InGamePartnerSentOutPkmn2[] = _(BATTLE_TEXT("{B_PARTNER_NAME_WITH_CLASS} sent out {B_PLAYER_MON2_NAME}!", "{B_PARTNER_NAME_WITH_CLASS} posílá do boje {B_PLAYER_MON2_NAME}!"));
static const u8 sText_InGamePartnerWithdrewPkmn1[] = _(BATTLE_TEXT("{B_PARTNER_NAME_WITH_CLASS} withdrew {B_PLAYER_MON1_NAME}!", "{B_PARTNER_NAME_WITH_CLASS} stahuje z boje {B_PLAYER_MON1_NAME}!"));
static const u8 sText_InGamePartnerWithdrewPkmn2[] = _(BATTLE_TEXT("{B_PARTNER_NAME_WITH_CLASS} withdrew {B_PLAYER_MON2_NAME}!", "{B_PARTNER_NAME_WITH_CLASS} stahuje z boje {B_PLAYER_MON2_NAME}!"));

const u16 gBattlePalaceFlavorTextTable[] =
{
    [B_MSG_GLINT_IN_EYE]   = STRINGID_GLINTAPPEARSINEYE,
    [B_MSG_GETTING_IN_POS] = STRINGID_PKMNGETTINGINTOPOSITION,
    [B_MSG_GROWL_DEEPLY]   = STRINGID_PKMNBEGANGROWLINGDEEPLY,
    [B_MSG_EAGER_FOR_MORE] = STRINGID_PKMNEAGERFORMORE,
};

const u8 *const gRefereeStringsTable[] =
{
    [B_MSG_REF_NOTHING_IS_DECIDED] = COMPOUND_STRING(BATTLE_TEXT("REFEREE: If nothing is decided in 3 turns, we will go to judging!", "ROZHODČÍ: Pokud se nerozhodne do 3 kol, přejdeme k hodnocení!")),
    [B_MSG_REF_THATS_IT]           = COMPOUND_STRING(BATTLE_TEXT("REFEREE: That's it! We will now go to judging to determine the winner!", "ROZHODČÍ: Konec! Vítěze teď určíme hodnocením!")),
    [B_MSG_REF_JUDGE_MIND]         = COMPOUND_STRING(BATTLE_TEXT("REFEREE: Judging category 1, Mind! The POKéMON showing the most guts!\p", "ROZHODČÍ: První kategorie, duch! Vyhrává Pokémon s největší odvahou!\p")),
    [B_MSG_REF_JUDGE_SKILL]        = COMPOUND_STRING(BATTLE_TEXT("REFEREE: Judging category 2, Skill! The POKéMON using moves the best!\p", "ROZHODČÍ: Druhá kategorie, technika! Vyhrává Pokémon, který nejlépe používá útoky!\p")),
    [B_MSG_REF_JUDGE_BODY]         = COMPOUND_STRING(BATTLE_TEXT("REFEREE: Judging category 3, Body! The POKéMON with the most vitality!\p", "ROZHODČÍ: Třetí kategorie, tělo! Vyhrává Pokémon s nejvíce silami!\p")),
    [B_MSG_REF_PLAYER_WON]         = COMPOUND_STRING(BATTLE_TEXT("REFEREE: Judgment: {B_BUFF1} to {B_BUFF2}! The winner is {B_PLAYER_NAME}'s {B_PLAYER_MON1_NAME}!\p", "ROZHODČÍ: Verdikt: {B_BUFF1} ku {B_BUFF2}! Vítězí tým {B_PLAYER_NAME}: {B_PLAYER_MON1_NAME}!\p")),
    [B_MSG_REF_OPPONENT_WON]       = COMPOUND_STRING(BATTLE_TEXT("REFEREE: Judgment: {B_BUFF1} to {B_BUFF2}! The winner is {B_TRAINER1_NAME}'s {B_OPPONENT_MON1_NAME}!\p", "ROZHODČÍ: Verdikt: {B_BUFF1} ku {B_BUFF2}! Vítězí tým {B_TRAINER1_NAME}: {B_OPPONENT_MON1_NAME}!\p")),
    [B_MSG_REF_DRAW]               = COMPOUND_STRING(BATTLE_TEXT("REFEREE: Judgment: 3 to 3! We have a draw!\p", "ROZHODČÍ: Verdikt: 3 ku 3! Je to remíza!\p")),
    [B_MSG_REF_COMMENCE_BATTLE]    = COMPOUND_STRING(BATTLE_TEXT("REFEREE: {B_PLAYER_MON1_NAME} VS {B_OPPONENT_MON1_NAME}! Commence battling!", "ROZHODČÍ: {B_PLAYER_MON1_NAME} VS {B_OPPONENT_MON1_NAME}! Začněte bojovat!")),
};

static const u8 sText_Trainer1Fled[] = _(BATTLE_TEXT("{PLAY_SE SE_FLEE}{B_TRAINER1_NAME_WITH_CLASS} fled!", "{PLAY_SE SE_FLEE}{B_TRAINER1_NAME_WITH_CLASS} utekl!"));
static const u8 sText_PlayerLostAgainstTrainer1[] = _(BATTLE_TEXT("You lost to {B_TRAINER1_NAME_WITH_CLASS}!", "Prohráváš! Vítěz: {B_TRAINER1_NAME_WITH_CLASS}!"));
static const u8 sText_PlayerBattledToDrawTrainer1[] = _(BATTLE_TEXT("You battled to a draw against {B_TRAINER1_NAME_WITH_CLASS}!", "Souboj skončil remízou! Soupeř: {B_TRAINER1_NAME_WITH_CLASS}!"));
const u8 gText_RecordBattleToPass[] = _(BATTLE_TEXT("Would you like to record your battle\non your Frontier Pass?", "Chceš zaznamenat svůj souboj\ndo Frontier Pass?"));
const u8 gText_BattleRecordedOnPass[] = _(BATTLE_TEXT("{B_PLAYER_NAME}'s battle result was recorded\non the Frontier Pass.", "{B_PLAYER_NAME}: výsledek souboje\nbyl zaznamenán do Frontier Pass."));
static const u8 sText_LinkTrainerWantsToBattlePause[] = _(BATTLE_TEXT("You are challenged by {B_LINK_OPPONENT1_NAME}!\p", "K souboji tě vyzývá {B_LINK_OPPONENT1_NAME}!\p"));
static const u8 sText_TwoLinkTrainersWantToBattlePause[] = _(BATTLE_TEXT("You are challenged by {B_LINK_OPPONENT1_NAME} and {B_LINK_OPPONENT2_NAME}!\p", "K souboji tě vyzývají {B_LINK_OPPONENT1_NAME} a {B_LINK_OPPONENT2_NAME}!\p"));
static const u8 sText_Your1[] = _(BATTLE_TEXT("Your", "Tvůj"));
static const u8 sText_Opposing1[] = _(BATTLE_TEXT("The opposing", "Soupeřův"));
static const u8 sText_Your2[] = _(BATTLE_TEXT("your", "tvůj"));
static const u8 sText_Opposing2[] = _(BATTLE_TEXT("the opposing", "soupeřův"));
static const u8 sText_EmptyStatus[] = _("$$$$$$$");

static const struct BattleWindowText sTextOnWindowsInfo_Normal[] =
{
    [B_WIN_MSG] = {
        .fillValue = PIXEL_FILL(0xF),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 1,
        .color.foreground = 1,
        .color.background = 15,
        .color.accent = 15,
        .color.shadow = 6,
    },
    [B_WIN_ACTION_PROMPT] = {
        .fillValue = PIXEL_FILL(0xF),
        .fontId = FONT_NORMAL,
        .x = 1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.background = 15,
        .color.accent = 15,
        .color.shadow = 6,
    },
    [B_WIN_ACTION_MENU] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_1] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_2] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_3] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_4] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_PP] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = B_SHOW_EFFECTIVENESS != SHOW_EFFECTIVENESS_NEVER ? 13 : 12,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = B_SHOW_EFFECTIVENESS != SHOW_EFFECTIVENESS_NEVER ? 15 : 11,
    },
    [B_WIN_DUMMY] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_PP_REMAINING] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 2,
        .y = 1,
        .speed = 0,
        .color.foreground = 12,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 11,
    },
    [B_WIN_MOVE_TYPE] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_SWITCH_PROMPT] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_YESNO] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_LEVEL_UP_BOX] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_LEVEL_UP_BANNER] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = 32,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.shadow = 2,
    },
    [B_WIN_VS_PLAYER] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_OPPONENT] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_1] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_2] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_3] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_4] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_OUTCOME_DRAW] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.shadow = 6,
    },
    [B_WIN_VS_OUTCOME_LEFT] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.shadow = 6,
    },
    [B_WIN_VS_OUTCOME_RIGHT] = {
        .fillValue = PIXEL_FILL(0x0),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.shadow = 6,
    },
    [B_WIN_MOVE_DESCRIPTION] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .color.foreground = TEXT_DYNAMIC_COLOR_4,
        .color.background = TEXT_DYNAMIC_COLOR_5,
        .color.accent = TEXT_DYNAMIC_COLOR_5,
        .color.shadow = TEXT_DYNAMIC_COLOR_6,
    },
    [B_CATCH_OR_NOT] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
};

static const struct BattleWindowText sTextOnWindowsInfo_KantoTutorial[] =
{
    [B_WIN_MSG] = {
        .fillValue = PIXEL_FILL(0xF),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 1,
        .color.foreground = 1,
        .color.background = 15,
        .color.accent = 15,
        .color.shadow = 6,
    },
    [B_WIN_ACTION_PROMPT] = {
        .fillValue = PIXEL_FILL(0xF),
        .fontId = FONT_NORMAL,
        .x = 1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.background = 15,
        .color.accent = 15,
        .color.shadow = 6,
    },
    [B_WIN_ACTION_MENU] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_1] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_2] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_3] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_4] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_PP] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = B_SHOW_EFFECTIVENESS != SHOW_EFFECTIVENESS_NEVER ? 13 : 12,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = B_SHOW_EFFECTIVENESS != SHOW_EFFECTIVENESS_NEVER ? 15 : 11,
    },
    [B_WIN_DUMMY] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_PP_REMAINING] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 2,
        .y = 1,
        .speed = 0,
        .color.foreground = 12,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 11,
    },
    [B_WIN_MOVE_TYPE] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_SWITCH_PROMPT] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_YESNO] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_LEVEL_UP_BOX] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_LEVEL_UP_BANNER] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = 32,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.shadow = 2,
    },
    [B_WIN_VS_PLAYER] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_OPPONENT] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_1] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_2] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_3] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_4] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_VS_OUTCOME_DRAW] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.shadow = 6,
    },
    [B_WIN_VS_OUTCOME_LEFT] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.shadow = 6,
    },
    [B_WIN_VS_OUTCOME_RIGHT] = {
        .fillValue = PIXEL_FILL(0x0),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.shadow = 6,
    },
    [B_WIN_MOVE_DESCRIPTION] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .color.foreground = TEXT_DYNAMIC_COLOR_4,
        .color.background = TEXT_DYNAMIC_COLOR_5,
        .color.accent = TEXT_DYNAMIC_COLOR_5,
        .color.shadow = TEXT_DYNAMIC_COLOR_6,
    },
    [B_WIN_OAK_OLD_MAN] = {
        .fillValue = PIXEL_FILL(0x1),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 1,
        .speed = 1,
        .fgColor = 2,
        .bgColor = 1,
        .shadowColor = 3,
    },
};

static const struct BattleWindowText sTextOnWindowsInfo_Arena[] =
{
    [B_WIN_MSG] = {
        .fillValue = PIXEL_FILL(0xF),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 1,
        .color.foreground = 1,
        .color.background = 15,
        .color.accent = 15,
        .color.shadow = 6,
    },
    [B_WIN_ACTION_PROMPT] = {
        .fillValue = PIXEL_FILL(0xF),
        .fontId = FONT_NORMAL,
        .x = 1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.background = 15,
        .color.accent = 15,
        .color.shadow = 6,
    },
    [B_WIN_ACTION_MENU] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_1] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_2] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_3] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_MOVE_NAME_4] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_PP] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = B_SHOW_EFFECTIVENESS != SHOW_EFFECTIVENESS_NEVER ? 13 : 12,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = B_SHOW_EFFECTIVENESS != SHOW_EFFECTIVENESS_NEVER ? 15 : 11,
    },
    [B_WIN_DUMMY] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_PP_REMAINING] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 2,
        .y = 1,
        .speed = 0,
        .color.foreground = 12,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 11,
    },
    [B_WIN_MOVE_TYPE] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_SWITCH_PROMPT] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_YESNO] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_LEVEL_UP_BOX] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [B_WIN_LEVEL_UP_BANNER] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = 32,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.shadow = 2,
    },
    [ARENA_WIN_PLAYER_NAME] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 1,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [ARENA_WIN_VS] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [ARENA_WIN_OPPONENT_NAME] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [ARENA_WIN_MIND] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [ARENA_WIN_SKILL] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [ARENA_WIN_BODY] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [ARENA_WIN_JUDGMENT_TITLE] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .color.foreground = 13,
        .color.background = 14,
        .color.accent = 14,
        .color.shadow = 15,
    },
    [ARENA_WIN_JUDGMENT_TEXT] = {
        .fillValue = PIXEL_FILL(0x1),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 1,
        .color.foreground = 2,
        .color.background = 1,
        .color.accent = 1,
        .color.shadow = 3,
    },
    [B_WIN_MOVE_DESCRIPTION] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .color.foreground = TEXT_DYNAMIC_COLOR_4,
        .color.background = TEXT_DYNAMIC_COLOR_5,
        .color.accent = TEXT_DYNAMIC_COLOR_5,
        .color.shadow = TEXT_DYNAMIC_COLOR_6,
    },
};

static const struct BattleWindowText *const sBattleTextOnWindowsInfo[] =
{
    [B_WIN_TYPE_NORMAL] = sTextOnWindowsInfo_Normal,
    [B_WIN_TYPE_ARENA]  = sTextOnWindowsInfo_Arena,
    [B_WIN_TYPE_KANTO_TUTORIAL] = sTextOnWindowsInfo_KantoTutorial,
};

static const u8 sRecordedBattleTextSpeeds[] = {8, 4, 1, 0};

void BufferStringBattle(enum StringID stringID, enum BattlerId battler)
{
    s32 i;
    const u8 *stringPtr = NULL;

    gBattleMsgDataPtr = (struct BattleMsgData *)(&gBattleResources->bufferA[battler][4]);
    gLastUsedItem = gBattleMsgDataPtr->lastItem;
    gLastUsedAbility = gBattleMsgDataPtr->lastAbility;
    gBattleScripting.battler = gBattleMsgDataPtr->scrActive;
    gBattleStruct->scriptPartyIdx = gBattleMsgDataPtr->bakScriptPartyIdx;
    gBattleStruct->hpScale = gBattleMsgDataPtr->hpScale;
    gPotentialItemEffectBattler = gBattleMsgDataPtr->itemEffectBattler;
    gBattleStruct->stringMoveType = gBattleMsgDataPtr->moveType;

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        sBattlerAbilities[i] = gBattleMsgDataPtr->abilities[i];
    }
    for (i = 0; i < TEXT_BUFF_ARRAY_COUNT; i++)
    {
        gBattleTextBuff1[i] = gBattleMsgDataPtr->textBuffs[0][i];
        gBattleTextBuff2[i] = gBattleMsgDataPtr->textBuffs[1][i];
        gBattleTextBuff3[i] = gBattleMsgDataPtr->textBuffs[2][i];
    }

    switch (stringID)
    {
    case STRINGID_INTROMSG: // first battle msg
        if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        {
            if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
            {
                if (gBattleTypeFlags & BATTLE_TYPE_TOWER_LINK_MULTI)
                {
                    stringPtr = sText_TwoTrainersWantToBattle;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS)
                {
                    stringPtr = sText_TwoLinkTrainersWantToBattle;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
                {
                    if (gBattleTypeFlags & BATTLE_TYPE_RECORDED)
                    {
                        if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK)
                            stringPtr = sText_LinkTrainerWantsToBattle;
                        else if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS)
                            stringPtr = sText_TwoTrainersWantToBattle;
                        else if (!(gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS))
                            stringPtr = sText_LinkTrainerWantsToBattlePause;
                        else
                            stringPtr = sText_TwoLinkTrainersWantToBattlePause;
                    }
                    else
                    {
                        stringPtr = sText_TwoLinkTrainersWantToBattle;
                    }
                }
                else
                {
                    if (TRAINER_BATTLE_PARAM.opponentA == TRAINER_UNION_ROOM)
                        stringPtr = sText_Trainer1WantsToBattle;
                    else if (gBattleTypeFlags & BATTLE_TYPE_RECORDED)
                        stringPtr = sText_LinkTrainerWantsToBattlePause;
                    else
                        stringPtr = sText_LinkTrainerWantsToBattle;
                }
            }
            else
            {
                if (BATTLE_TWO_VS_ONE_OPPONENT)
                    stringPtr = sText_Trainer1WantsToBattle;
                else if (gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER))
                    stringPtr = sText_TwoTrainersWantToBattle;
                else if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS)
                    stringPtr = sText_TwoTrainersWantToBattle;
                else
                    stringPtr = sText_Trainer1WantsToBattle;
            }
        }
        else
        {
            if (IsGhostBattleWithoutScope())
                stringPtr = sText_GhostAppearedCantId;
            else if (gBattleTypeFlags & BATTLE_TYPE_GHOST)
                stringPtr = sText_TheGhostAppeared;
            else if (gBattleTypeFlags & BATTLE_TYPE_LEGENDARY)
                stringPtr = sText_LegendaryPkmnAppeared;
            else if (IsDoubleBattle() && IsValidForBattle(GetBattlerMon(GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT))))
                stringPtr = sText_TwoWildPkmnAppeared;
            else if (gBattleTypeFlags & BATTLE_TYPE_CATCH_TUTORIAL)
                stringPtr = sText_WildPkmnAppearedPause;
            else
                stringPtr = sText_WildPkmnAppeared;
        }
        break;
    case STRINGID_INTROSENDOUT: // poke first send-out
        if (IsOnPlayerSide(battler))
        {
            if (IsDoubleBattle() && IsValidForBattle(GetBattlerMon(GetPartnerBattler(battler))))
            {
                if (gBattleTypeFlags & BATTLE_TYPE_MULTI && (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK || gBattleTypeFlags & BATTLE_TYPE_LINK))
                {
                    if (BattlerIsPlayer(battler))
                        stringPtr = sText_LinkPartnerSentOutPkmn2GoPkmn; // Player is on left
                    else
                        stringPtr = sText_LinkPartnerSentOutPkmn1GoPkmn; // Link Partner on left
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
                {
                    if (BattlerIsPlayer(battler) && (battler & BIT_FLANK) == B_FLANK_LEFT)
                        stringPtr = sText_InGamePartnerSentOutZGoN; // Player is on left
                    else
                        stringPtr = sText_InGamePartnerSentOutNGoZ; // Partner on left
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_RAID)
                {
                    stringPtr = sText_BeCarefulPkmn;
                }
                else
                {
                    stringPtr = sText_GoTwoPkmn;
                }
            }
            else
            {
                stringPtr = sText_GoPkmn;
            }
        }
        else
        {
            if (IsDoubleBattle() && IsValidForBattle(GetBattlerMon(GetPartnerBattler(battler))))
            {
                if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK && BATTLE_TWO_VS_ONE_OPPONENT)
                    stringPtr = sText_LinkTrainerSentOutTwoPkmn;
                else if (BATTLE_TWO_VS_ONE_OPPONENT)
                    stringPtr = sText_Trainer1SentOutTwoPkmn;
                else if ((gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK && gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) || (gBattleTypeFlags & BATTLE_TYPE_MULTI && (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK || gBattleTypeFlags & BATTLE_TYPE_LINK)))
                    stringPtr = sText_TwoLinkTrainersIntroSendOutPkmn;
                else if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS || gBattleTypeFlags & BATTLE_TYPE_TOWER_LINK_MULTI)
                    stringPtr = sText_TwoTrainersSentPkmn;
                else if (BattlerIsLink(battler) || (BattlerIsRecorded(battler) && BattlerIsOpponent(battler))) // Link Opponent doubles and test opponent
                    stringPtr = sText_LinkTrainerSentOutTwoPkmn;
                else
                    stringPtr = sText_Trainer1SentOutTwoPkmn;
            }
            else if (BattlerIsLink(battler) || (BattlerIsRecorded(battler) && BattlerIsOpponent(battler)))
            {
                if (TRAINER_BATTLE_PARAM.opponentA == TRAINER_UNION_ROOM)
                    stringPtr = sText_Trainer1SentOutPkmn;
                else
                    stringPtr = sText_LinkTrainerIntroSendOutPkmn;
            }
            else
            {
                stringPtr = sText_Trainer1SentOutPkmn;
            }
        }
        break;
    case STRINGID_RETURNMON: // sending poke to ball msg
        if ((GetBattlerPosition(battler) & BIT_FLANK) == B_FLANK_LEFT) // battler 0 and 1
        {
            if (BattlerIsPlayer(battler) || BattlerIsWally(battler)) // Player
            {
                if (*(&gBattleStruct->hpScale) == 0)
                    stringPtr = sText_PkmnThatsEnough;
                else if (*(&gBattleStruct->hpScale) == 1 || IsDoubleBattle())
                    stringPtr = sText_PkmnComeBack;
                else if (*(&gBattleStruct->hpScale) == 2)
                    stringPtr = sText_PkmnOkComeBack;
                else
                    stringPtr = sText_PkmnGoodComeBack;
            }
            else if (BattlerIsPartner(battler)) // Link or Ingame Partner
            {
                if (BattlerIsLink(battler) || gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK)
                    stringPtr = sText_LinkPartnerWithdrewPkmn1;
                else
                    stringPtr = sText_InGamePartnerWithdrewPkmn1;
            }
            else if (TRAINER_BATTLE_PARAM.opponentA == TRAINER_LINK_OPPONENT
             || gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK) // Link Opponent A and test opponent
            {
                stringPtr = sText_LinkTrainer1WithdrewPkmn;
            }
            else // Opponent A
            {
                stringPtr = sText_Trainer1WithdrewPkmn;
            }
        }
        else // battler 2 and 3
        {
            if (BattlerIsPlayer(battler)) // Player
            {
                if (*(&gBattleStruct->hpScale) == 0)
                    stringPtr = sText_PkmnThatsEnough;
                else if (*(&gBattleStruct->hpScale) == 1 || IsDoubleBattle())
                    stringPtr = sText_PkmnComeBack;
                else if (*(&gBattleStruct->hpScale) == 2)
                    stringPtr = sText_PkmnOkComeBack;
                else
                    stringPtr = sText_PkmnGoodComeBack;
            }
            else if (BattlerIsPartner(battler)) // Link or Ingame Partner
            {
                if (BattlerIsLink(battler) || gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK)
                    stringPtr = sText_LinkPartnerWithdrewPkmn2;
                else
                    stringPtr = sText_InGamePartnerWithdrewPkmn2;
            }
            else if (TRAINER_BATTLE_PARAM.opponentA == TRAINER_LINK_OPPONENT
             || gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK) // Link Opponent B and test opponent
            {
                if (BattleSideHasTwoTrainers(B_SIDE_OPPONENT))
                    stringPtr = sText_LinkTrainer2WithdrewPkmn;
                else
                    stringPtr = sText_LinkTrainer1WithdrewPkmn;
            }
            else if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) // Opponent B
            {
                stringPtr = sText_Trainer2WithdrewPkmn;
            }
            else // Opponent A
            {
                stringPtr = sText_Trainer1WithdrewPkmn;
            }
        }
        break;
    case STRINGID_SWITCHINMON: // switch-in msg
        if ((GetBattlerPosition(gBattleScripting.battler) & BIT_FLANK) == B_FLANK_LEFT) // battler 0 and 1
        {
            if (BattlerIsPlayer(gBattleScripting.battler)) // Player
            {
                if (*(&gBattleStruct->hpScale) == 0)
                    stringPtr = sText_GoPkmn2;
                else if (*(&gBattleStruct->hpScale) == 1 || IsDoubleBattle())
                    stringPtr = sText_DoItPkmn;
                else if (*(&gBattleStruct->hpScale) == 2)
                    stringPtr = sText_GoForItPkmn;
                else
                    stringPtr = sText_YourFoesWeakGetEmPkmn;
            }
            else if (BattlerIsPartner(gBattleScripting.battler)) // Link or Ingame Partner
            {
                if (BattlerIsLink(gBattleScripting.battler) || gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK)
                    stringPtr = sText_LinkPartnerSentOutPkmn1;
                else
                    stringPtr = sText_InGamePartnerSentOutPkmn1;
            }
            else if (TRAINER_BATTLE_PARAM.opponentA == TRAINER_LINK_OPPONENT
             || gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK) // Link Opponent A and test opponent
            {
                stringPtr = sText_LinkTrainerSentOutPkmn;
            }
            else // Opponent A
            {
                stringPtr = sText_Trainer1SentOutPkmn;
            }
        }
        else // battler 2 and 3
        {
            if (BattlerIsPlayer(gBattleScripting.battler)) // Player
            {
                if (*(&gBattleStruct->hpScale) == 0)
                    stringPtr = sText_GoPkmn2;
                else if (*(&gBattleStruct->hpScale) == 1 || IsDoubleBattle())
                    stringPtr = sText_DoItPkmn;
                else if (*(&gBattleStruct->hpScale) == 2)
                    stringPtr = sText_GoForItPkmn;
                else
                    stringPtr = sText_YourFoesWeakGetEmPkmn;
            }
            else if (BattlerIsPartner(gBattleScripting.battler)) // Link or Ingame Partner
            {
                if (BattlerIsLink(gBattleScripting.battler) || gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK)
                    stringPtr = sText_LinkPartnerSentOutPkmn2;
                else
                    stringPtr = sText_InGamePartnerSentOutPkmn2;
            }
            else if (TRAINER_BATTLE_PARAM.opponentA == TRAINER_LINK_OPPONENT
             || gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK) // Link Opponent B and test opponent
            {
                if (BattleSideHasTwoTrainers(B_SIDE_OPPONENT))
                    stringPtr = sText_LinkTrainer2SentOutPkmn2;
                else
                    stringPtr = sText_LinkTrainerSentOutPkmn2;
            }
            else if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) // Opponent B
            {
                stringPtr = sText_Trainer2SentOutPkmn;
            }
            else // Single trainer double Opponent A
            {
                stringPtr = sText_Trainer1SentOutPkmn2;
            }
        }
        break;
    case STRINGID_USEDMOVE: // Pokémon used a move msg
        if (gBattleMsgDataPtr->currentMove >= MOVES_COUNT
         && !IsZMove(gBattleMsgDataPtr->currentMove)
         && !IsMaxMove(gBattleMsgDataPtr->currentMove))
            StringCopy(gBattleTextBuff3, gTypesInfo[*(&gBattleStruct->stringMoveType)].generic);
        else
            StringCopy(gBattleTextBuff3, GetMoveName(gBattleMsgDataPtr->currentMove));
        stringPtr = sText_AttackerUsedX;
        break;
    case STRINGID_BATTLEEND: // battle end
        if (gBattleTextBuff1[0] & B_OUTCOME_LINK_BATTLE_RAN)
        {
            gBattleTextBuff1[0] &= ~(B_OUTCOME_LINK_BATTLE_RAN);
            if (!(BattlerIsPlayer(battler) || BattlerIsPlayer(GetPartnerBattler(battler))) && gBattleTextBuff1[0] != B_OUTCOME_DREW)
                gBattleTextBuff1[0] ^= (B_OUTCOME_LOST | B_OUTCOME_WON);

            if (gBattleTextBuff1[0] == B_OUTCOME_LOST || gBattleTextBuff1[0] == B_OUTCOME_DREW)
                stringPtr = sText_GotAwaySafely;
            else if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
                stringPtr = sText_TwoWildFled;
            else
                stringPtr = sText_WildFled;
        }
        else
        {
            if (!(BattlerIsPlayer(battler) || BattlerIsPlayer(GetPartnerBattler(battler))) && gBattleTextBuff1[0] != B_OUTCOME_DREW)
                gBattleTextBuff1[0] ^= (B_OUTCOME_LOST | B_OUTCOME_WON);

            if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
            {
                switch (gBattleTextBuff1[0])
                {
                case B_OUTCOME_WON:
                    if (gBattleTypeFlags & BATTLE_TYPE_TOWER_LINK_MULTI)
                        stringPtr = sText_TwoInGameTrainersDefeated;
                    else
                        stringPtr = sText_TwoLinkTrainersDefeated;
                    break;
                case B_OUTCOME_LOST:
                    stringPtr = sText_PlayerLostToTwo;
                    break;
                case B_OUTCOME_DREW:
                    stringPtr = sText_PlayerBattledToDrawVsTwo;
                    break;
                }
            }
            else if (TRAINER_BATTLE_PARAM.opponentA == TRAINER_UNION_ROOM)
            {
                switch (gBattleTextBuff1[0])
                {
                case B_OUTCOME_WON:
                    stringPtr = sText_PlayerDefeatedLinkTrainerTrainer1;
                    break;
                case B_OUTCOME_LOST:
                    stringPtr = sText_PlayerLostAgainstTrainer1;
                    break;
                case B_OUTCOME_DREW:
                    stringPtr = sText_PlayerBattledToDrawTrainer1;
                    break;
                }
            }
            else
            {
                switch (gBattleTextBuff1[0])
                {
                case B_OUTCOME_WON:
                    stringPtr = sText_PlayerDefeatedLinkTrainer;
                    break;
                case B_OUTCOME_LOST:
                    stringPtr = sText_PlayerLostAgainstLinkTrainer;
                    break;
                case B_OUTCOME_DREW:
                    stringPtr = sText_PlayerBattledToDrawLinkTrainer;
                    break;
                }
            }
        }
        break;
    case STRINGID_TRAINERSLIDE:
        stringPtr = gBattleStruct->trainerSlideMsg;
        break;
    default: // load a string from the table
        if (stringID >= STRINGID_COUNT)
        {
            gDisplayedStringBattle[0] = EOS;
            return;
        }
        else
        {
            stringPtr = gBattleStringsTable[stringID];
        }
        break;
    }

    BattleStringExpandPlaceholdersToDisplayedString(stringPtr);
}

u32 BattleStringExpandPlaceholdersToDisplayedString(const u8 *src)
{
#ifndef NDEBUG
    u32 j, strWidth;
    u32 dstID = BattleStringExpandPlaceholders(src, gDisplayedStringBattle, sizeof(gDisplayedStringBattle));
    for (j = 1;; j++)
    {
        strWidth = GetStringLineWidth(0, gDisplayedStringBattle, 0, j, sizeof(gDisplayedStringBattle));
        if (strWidth == 0)
            break;
    }
    return dstID;
#else
    return BattleStringExpandPlaceholders(src, gDisplayedStringBattle, sizeof(gDisplayedStringBattle));
#endif
}

static const u8 *TryGetStatusString(u8 *src)
{
    u32 i;
    u8 status[8];
    u32 chars1, chars2;
    u8 *statusPtr;

    memcpy(status, sText_EmptyStatus, min(ARRAY_COUNT(status), ARRAY_COUNT(sText_EmptyStatus)));

    statusPtr = status;
    for (i = 0; i < ARRAY_COUNT(status); i++)
    {
        if (*src == EOS) break; // one line required to match -g
        *statusPtr = *src;
        src++;
        statusPtr++;
    }

    chars1 = *(u32 *)(&status[0]);
    chars2 = *(u32 *)(&status[4]);

    for (i = 0; i < ARRAY_COUNT(gStatusConditionStringsTable); i++)
    {
        if (chars1 == *(u32 *)(&gStatusConditionStringsTable[i][0][0])
            && chars2 == *(u32 *)(&gStatusConditionStringsTable[i][0][4]))
            return gStatusConditionStringsTable[i][1];
    }
    return NULL;
}

static void GetBattlerNick(enum BattlerId battler, u8 *dst)
{
    struct Pokemon *illusionMon = GetIllusionMonPtr(battler);
    struct Pokemon *mon = GetBattlerMon(battler);

    if (illusionMon != NULL)
        mon = illusionMon;
    GetMonData(mon, MON_DATA_NICKNAME, dst);
    StringGet_Nickname(dst);
}

#define HANDLE_NICKNAME_STRING_CASE(battler)                            \
    if (!IsOnPlayerSide(battler))                                       \
    {                                                                   \
        if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)                     \
            toCpy = sText_FoePkmnPrefix;                                \
        else                                                            \
            toCpy = sText_WildPkmnPrefix;                               \
        while (*toCpy != EOS)                                           \
        {                                                               \
            dst[dstID] = *toCpy;                                        \
            dstID++;                                                    \
            toCpy++;                                                    \
        }                                                               \
    }                                                                   \
    GetBattlerNick(battler, text);                                      \
    toCpy = text;

#define HANDLE_NICKNAME_STRING_LOWERCASE(battler)                       \
    if (!IsOnPlayerSide(battler))                       \
    {                                                                   \
        if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)                     \
            toCpy = sText_FoePkmnPrefixLower;                           \
        else                                                            \
            toCpy = sText_WildPkmnPrefixLower;                          \
        while (*toCpy != EOS)                                           \
        {                                                               \
            dst[dstID] = *toCpy;                                        \
            dstID++;                                                    \
            toCpy++;                                                    \
        }                                                               \
    }                                                                   \
    GetBattlerNick(battler, text);                                      \
    toCpy = text;

static const u8 *BattleStringGetOpponentNameByTrainerId(u16 trainerId, u8 *text, u8 multiplayerId, enum BattlerId battler)
{
    const u8 *toCpy = NULL;

    if (gBattleTypeFlags & BATTLE_TYPE_SECRET_BASE)
    {
        u32 i;
        for (i = 0; i < ARRAY_COUNT(gBattleResources->secretBase->trainerName); i++)
            text[i] = gBattleResources->secretBase->trainerName[i];
        text[i] = EOS;
        ConvertInternationalString(text, gBattleResources->secretBase->language);
        toCpy = text;
    }
    else if (trainerId == TRAINER_UNION_ROOM)
    {
        toCpy = gLinkPlayers[multiplayerId ^ BIT_SIDE].name;
    }
    else if (trainerId == TRAINER_LINK_OPPONENT)
    {
        if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
            toCpy = gLinkPlayers[GetBattlerMultiplayerId(battler)].name;
        else
            toCpy = gLinkPlayers[GetBattlerMultiplayerId(battler) & BIT_SIDE].name;
    }
    else if (trainerId == TRAINER_FRONTIER_BRAIN)
    {
        CopyFrontierBrainTrainerName(text);
        toCpy = text;
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
    {
        GetFrontierTrainerName(text, trainerId);
        toCpy = text;
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER && gMapHeader.regionMapSectionId == MAPSEC_TRAINER_TOWER_2)
    {
        GetTrainerTowerOpponentName(text);
        toCpy = text;
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL)
    {
        GetTrainerHillTrainerName(text, trainerId);
        toCpy = text;
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_EREADER_TRAINER)
    {
        GetEreaderTrainerName(text);
        toCpy = text;
    }
    else
    {
        enum TrainerClassID trainerClass = GetTrainerClassFromId(TRAINER_BATTLE_PARAM.opponentA);

        if (trainerClass == TRAINER_CLASS_RIVAL_EARLY_FRLG || trainerClass == TRAINER_CLASS_RIVAL_LATE_FRLG || trainerClass == TRAINER_CLASS_CHAMPION_FRLG)
            toCpy = GetExpandedPlaceholder(PLACEHOLDER_ID_RIVAL);
        else
            toCpy = GetTrainerNameFromId(trainerId);
    }

    assertf(DoesStringProperlyTerminate(toCpy, TRAINER_NAME_LENGTH + 1),"Opponent needs a valid name")
    {
        return sText_EmptyString4;
    }

    return toCpy;
}

static const u8 *BattleStringGetOpponentName(u8 *text, u8 multiplayerId, enum BattlerId battler)
{
    const u8 *toCpy = NULL;

    switch (GetBattlerPosition(battler))
    {
    case B_POSITION_OPPONENT_LEFT:
        toCpy = BattleStringGetOpponentNameByTrainerId(TRAINER_BATTLE_PARAM.opponentA, text, multiplayerId, battler);
        break;
    case B_POSITION_OPPONENT_RIGHT:
        if (gBattleTypeFlags & (BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_MULTI) && !BATTLE_TWO_VS_ONE_OPPONENT)
            toCpy = BattleStringGetOpponentNameByTrainerId(TRAINER_BATTLE_PARAM.opponentB, text, multiplayerId, battler);
        else
            toCpy = BattleStringGetOpponentNameByTrainerId(TRAINER_BATTLE_PARAM.opponentA, text, multiplayerId, battler);
        break;
    default:
        break;
    }

    return toCpy;
}

static const u8 *BattleStringGetPlayerName(u8 *text, enum BattlerId battler)
{
    const u8 *toCpy = NULL;

    switch (GetBattlerPosition(battler))
    {
    case B_POSITION_PLAYER_LEFT:
        if (gBattleTypeFlags & BATTLE_TYPE_RECORDED)
            toCpy = gLinkPlayers[0].name;
        else
            toCpy = gSaveBlock2Ptr->playerName;
        break;
    case B_POSITION_PLAYER_RIGHT:
        if (((gBattleTypeFlags & BATTLE_TYPE_RECORDED) && !(gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER)))
            || gTestRunnerEnabled)
        {
            toCpy = gLinkPlayers[0].name;
        }
        else if ((gBattleTypeFlags & BATTLE_TYPE_LINK) && gBattleTypeFlags & (BATTLE_TYPE_RECORDED | BATTLE_TYPE_MULTI))
        {
            toCpy = gLinkPlayers[2].name;
        }
        else if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
        {
            GetFrontierTrainerName(text, gPartnerTrainerId);
            toCpy = text;
        }
        else
        {
            toCpy = gSaveBlock2Ptr->playerName;
        }
        break;
    default:
        break;
    }

    return toCpy;
}

static const u8 *BattleStringGetTrainerName(u8 *text, u8 multiplayerId, enum BattlerId battler)
{
    if (IsOnPlayerSide(battler))
        return BattleStringGetPlayerName(text, battler);
    else
        return BattleStringGetOpponentName(text, multiplayerId, battler);
}

static const u8 *BattleStringGetOpponentClassByTrainerId(u16 trainerId)
{
    const u8 *toCpy;

    if (gBattleTypeFlags & BATTLE_TYPE_SECRET_BASE)
        toCpy = gTrainerClasses[GetSecretBaseTrainerClass()].name;
    else if (trainerId == TRAINER_UNION_ROOM)
        toCpy = gTrainerClasses[GetUnionRoomTrainerClass()].name;
    else if (trainerId == TRAINER_FRONTIER_BRAIN)
        toCpy = gTrainerClasses[GetFrontierBrainTrainerClass()].name;
    else if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
        toCpy = gTrainerClasses[GetFrontierOpponentClass(trainerId)].name;
    else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER && gMapHeader.regionMapSectionId == MAPSEC_TRAINER_TOWER_2)
        toCpy = gTrainerClasses[GetTrainerTowerOpponentClass()].name;
    else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL)
        toCpy = gTrainerClasses[GetTrainerHillOpponentClass(trainerId)].name;
    else if (gBattleTypeFlags & BATTLE_TYPE_EREADER_TRAINER)
        toCpy = gTrainerClasses[GetEreaderTrainerClassId()].name;
    else if (trainerId == TRAINER_LINK_OPPONENT)
        toCpy = gTrainerClasses[TRAINER_NONE].name;
    else
        toCpy = gTrainerClasses[GetTrainerClassFromId(trainerId)].name;

    return toCpy;
}

// Ensure the defined length for an item name can contain the full defined length of a berry name.
// This ensures that custom Enigma Berry names will fit in the text buffer at the top of BattleStringExpandPlaceholders.
STATIC_ASSERT(BERRY_NAME_LENGTH + ARRAY_COUNT(sText_BerrySuffix) <= ITEM_NAME_LENGTH, BerryNameTooLong);

u32 BattleStringExpandPlaceholders(const u8 *src, u8 *dst, u32 dstSize)
{
    u32 dstID = 0; // if they used dstID, why not use srcID as well?
    const u8 *toCpy = NULL;
    u8 text[max(max(max(32, TRAINER_NAME_LENGTH + 1), POKEMON_NAME_LENGTH + 1), ITEM_NAME_LENGTH)];
    u8 *textStart = &text[0];
    u8 multiplayerId;
    u8 fontId = FONT_NORMAL;

    if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK)
        multiplayerId = gRecordedBattleMultiplayerId;
    else
        multiplayerId = GetMultiplayerId();

    // Clear destination first
    while (dstID < dstSize)
    {
        dst[dstID] = EOS;
        dstID++;
    }

    dstID = 0;
    while (*src != EOS)
    {
        toCpy = NULL;

        if (*src == PLACEHOLDER_BEGIN)
        {
            src++;
            u32 classLength = 0;
            u32 nameLength = 0;
            const u8 *classString;
            const u8 *nameString;
            switch (*src)
            {
            case B_TXT_BUFF1:
                if (gBattleTextBuff1[0] == B_BUFF_PLACEHOLDER_BEGIN)
                {
                    ExpandBattleTextBuffPlaceholders(gBattleTextBuff1, gStringVar1);
                    toCpy = gStringVar1;
                }
                else
                {
                    toCpy = TryGetStatusString(gBattleTextBuff1);
                    if (toCpy == NULL)
                        toCpy = gBattleTextBuff1;
                }
                break;
            case B_TXT_BUFF2:
                if (gBattleTextBuff2[0] == B_BUFF_PLACEHOLDER_BEGIN)
                {
                    ExpandBattleTextBuffPlaceholders(gBattleTextBuff2, gStringVar2);
                    toCpy = gStringVar2;
                }
                else
                {
                    toCpy = gBattleTextBuff2;
                }
                break;
            case B_TXT_BUFF3:
                if (gBattleTextBuff3[0] == B_BUFF_PLACEHOLDER_BEGIN)
                {
                    ExpandBattleTextBuffPlaceholders(gBattleTextBuff3, gStringVar3);
                    toCpy = gStringVar3;
                }
                else
                {
                    toCpy = gBattleTextBuff3;
                }
                break;
            case B_TXT_COPY_VAR_1:
                toCpy = gStringVar1;
                break;
            case B_TXT_COPY_VAR_2:
                toCpy = gStringVar2;
                break;
            case B_TXT_COPY_VAR_3:
                toCpy = gStringVar3;
                break;
            case B_TXT_PLAYER_MON1_NAME: // first player poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), text);
                toCpy = text;
                break;
            case B_TXT_OPPONENT_MON1_NAME: // first enemy poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), text);
                toCpy = text;
                break;
            case B_TXT_PLAYER_MON2_NAME: // second player poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT), text);
                toCpy = text;
                break;
            case B_TXT_OPPONENT_MON2_NAME: // second enemy poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT), text);
                toCpy = text;
                break;
            case B_TXT_LINK_PLAYER_MON1_NAME: // link first player poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), text);
                toCpy = text;
                break;
            case B_TXT_LINK_OPPONENT_MON1_NAME: // link first opponent poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), text);
                toCpy = text;
                break;
            case B_TXT_LINK_PLAYER_MON2_NAME: // link second player poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT), text);
                toCpy = text;
                break;
            case B_TXT_LINK_OPPONENT_MON2_NAME: // link second opponent poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT), text);
                toCpy = text;
                break;
            case B_TXT_ATK_NAME_WITH_PREFIX_MON1: // Unused, to change into sth else.
                break;
            case B_TXT_ATK_PARTNER_NAME: // attacker partner name
                GetBattlerNick(GetPartnerBattler(gBattlerAttacker), text);
                toCpy = text;
                break;
            case B_TXT_ATK_NAME_WITH_PREFIX: // attacker name with prefix
                HANDLE_NICKNAME_STRING_CASE(gBattlerAttacker)
                break;
            case B_TXT_DEF_NAME_WITH_PREFIX: // target name with prefix
                HANDLE_NICKNAME_STRING_CASE(gBattlerTarget)
                break;
            case B_TXT_DEF_NAME: // target name
                GetBattlerNick(gBattlerTarget, text);
                toCpy = text;
                break;
            case B_TXT_DEF_PARTNER_NAME: // partner target name
                GetBattlerNick(GetPartnerBattler(gBattlerTarget), text);
                toCpy = text;
                break;
            case B_TXT_EFF_NAME_WITH_PREFIX: // effect battler name with prefix
                HANDLE_NICKNAME_STRING_CASE(gEffectBattler)
                break;
            case B_TXT_SCR_ACTIVE_NAME_WITH_PREFIX: // scripting active battler name with prefix
                HANDLE_NICKNAME_STRING_CASE(gBattleScripting.battler)
                break;
            case B_TXT_CURRENT_MOVE: // current move name
                if (gBattleMsgDataPtr->currentMove >= MOVES_COUNT
                 && !IsZMove(gBattleMsgDataPtr->currentMove)
                 && !IsMaxMove(gBattleMsgDataPtr->currentMove))
                    toCpy = gTypesInfo[gBattleStruct->stringMoveType].generic;
                else
                    toCpy = GetMoveName(gBattleMsgDataPtr->currentMove);
                break;
            case B_TXT_LAST_MOVE: // originally used move name
                if (gBattleMsgDataPtr->originallyUsedMove >= MOVES_COUNT
                 && !IsZMove(gBattleMsgDataPtr->currentMove)
                 && !IsMaxMove(gBattleMsgDataPtr->currentMove))
                    toCpy = gTypesInfo[gBattleStruct->stringMoveType].generic;
                else
                    toCpy = GetMoveName(gBattleMsgDataPtr->originallyUsedMove);
                break;
            case B_TXT_LAST_ITEM: // last used item
                if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
                {
                    if (gLastUsedItem == ITEM_ENIGMA_BERRY_E_READER)
                    {
                        if (!(gBattleTypeFlags & BATTLE_TYPE_MULTI))
                        {
                            if ((gBattleScripting.multiplayerId != 0 && (gPotentialItemEffectBattler & BIT_SIDE))
                                || (gBattleScripting.multiplayerId == 0 && !(gPotentialItemEffectBattler & BIT_SIDE)))
                            {
                                StringCopy(text, gEnigmaBerries[gPotentialItemEffectBattler].name);
                                StringAppend(text, sText_BerrySuffix);
                                toCpy = text;
                            }
                            else
                            {
                                toCpy = sText_EnigmaBerry;
                            }
                        }
                        else
                        {
                            if (gLinkPlayers[gBattleScripting.multiplayerId].id == gPotentialItemEffectBattler)
                            {
                                StringCopy(text, gEnigmaBerries[gPotentialItemEffectBattler].name);
                                StringAppend(text, sText_BerrySuffix);
                                toCpy = text;
                            }
                            else
                            {
                                toCpy = sText_EnigmaBerry;
                            }
                        }
                    }
                    else
                    {
                        CopyItemName(gLastUsedItem, text);
                        toCpy = text;
                    }
                }
                else
                {
                    CopyItemName(gLastUsedItem, text);
                    toCpy = text;
                }
                break;
            case B_TXT_LAST_ABILITY: // last used ability
                toCpy = gAbilitiesInfo[gLastUsedAbility].name;
                break;
            case B_TXT_ATK_ABILITY: // attacker ability
                toCpy = gAbilitiesInfo[sBattlerAbilities[gBattlerAttacker]].name;
                break;
            case B_TXT_DEF_ABILITY: // target ability
                toCpy = gAbilitiesInfo[sBattlerAbilities[gBattlerTarget]].name;
                break;
            case B_TXT_SCR_ACTIVE_ABILITY: // scripting active ability
                toCpy = gAbilitiesInfo[sBattlerAbilities[gBattleScripting.battler]].name;
                break;
            case B_TXT_EFF_ABILITY: // effect battler ability
                toCpy = gAbilitiesInfo[sBattlerAbilities[gEffectBattler]].name;
                break;
            case B_TXT_TRAINER1_CLASS: // trainer class name
                toCpy = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentA);
                break;
            case B_TXT_TRAINER1_NAME: // trainer1 name
                toCpy = BattleStringGetOpponentNameByTrainerId(TRAINER_BATTLE_PARAM.opponentA, text, multiplayerId, GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT));
                break;
            case B_TXT_TRAINER1_NAME_WITH_CLASS: // trainer1 name with trainer class
                toCpy = textStart;
                classString = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentA);
                while (classString[classLength] != EOS)
                {
                    textStart[classLength] = classString[classLength];
                    classLength++;
                }
                textStart[classLength] = CHAR_SPACE;
                textStart += classLength + 1;
                nameString = BattleStringGetOpponentNameByTrainerId(TRAINER_BATTLE_PARAM.opponentA, textStart, multiplayerId, GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT));
                if (nameString != textStart)
                {
                    while (nameString[nameLength] != EOS)
                    {
                        textStart[nameLength] = nameString[nameLength];
                        nameLength++;
                    }
                    textStart[nameLength] = EOS;
                }
                break;
            case B_TXT_LINK_PLAYER_NAME: // link player name
                toCpy = gLinkPlayers[multiplayerId].name;
                break;
            case B_TXT_LINK_PARTNER_NAME: // link partner name
                toCpy = gLinkPlayers[GetBattlerMultiplayerId(GetPartnerPosition(gLinkPlayers[multiplayerId].id))].name;
                break;
            case B_TXT_LINK_OPPONENT1_NAME: // link opponent 1 name
                toCpy = gLinkPlayers[GetBattlerMultiplayerId(GetBattlerLeftFoe(gLinkPlayers[multiplayerId].id))].name;
                break;
            case B_TXT_LINK_OPPONENT2_NAME: // link opponent 2 name
                toCpy = gLinkPlayers[GetBattlerMultiplayerId(GetBattlerRightFoe(gLinkPlayers[multiplayerId].id))].name;
                break;
            case B_TXT_LINK_SCR_TRAINER_NAME: // link scripting active name
                toCpy = gLinkPlayers[GetBattlerMultiplayerId(gBattleScripting.battler)].name;
                break;
            case B_TXT_PLAYER_NAME: // player name
                toCpy = BattleStringGetPlayerName(text, GetBattlerAtPosition(B_POSITION_PLAYER_LEFT));
                break;
            case B_TXT_TRAINER1_LOSE_TEXT: // trainerA lose text
                if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
                {
                    CopyFrontierTrainerText(FRONTIER_PLAYER_WON_TEXT, TRAINER_BATTLE_PARAM.opponentA);
                    toCpy = gStringVar4;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER && gMapHeader.regionMapSectionId == MAPSEC_TRAINER_TOWER_2)
                {
                    GetTrainerTowerOpponentLoseText(gStringVar4, 0);
                    toCpy = gStringVar4;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL)
                {
                    CopyTrainerHillTrainerText(TRAINER_HILL_TEXT_PLAYER_WON, TRAINER_BATTLE_PARAM.opponentA);
                    toCpy = gStringVar4;
                }
                else
                {
                    toCpy = GetTrainerALoseText();
                }
                break;
            case B_TXT_TRAINER1_WIN_TEXT: // trainerA win text
                if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
                {
                    CopyFrontierTrainerText(FRONTIER_PLAYER_LOST_TEXT, TRAINER_BATTLE_PARAM.opponentA);
                    toCpy = gStringVar4;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER && gMapHeader.regionMapSectionId == MAPSEC_TRAINER_TOWER_2)
                {
                    GetTrainerTowerOpponentWinText(gStringVar4, 0);
                    toCpy = gStringVar4;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL)
                {
                    CopyTrainerHillTrainerText(TRAINER_HILL_TEXT_PLAYER_LOST, TRAINER_BATTLE_PARAM.opponentA);
                    toCpy = gStringVar4;
                }
                else
                {
                    toCpy = GetTrainerWonSpeech();
                }
                break;
            case B_TXT_26: // ?
                if (!IsOnPlayerSide(gBattleScripting.battler))
                {
                    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
                        toCpy = sText_FoePkmnPrefix;
                    else
                        toCpy = sText_WildPkmnPrefix;
                    while (*toCpy != EOS)
                    {
                        dst[dstID] = *toCpy;
                        dstID++;
                        toCpy++;
                    }
                }
                GetMonData(&GetBattlerParty(gBattleScripting.battler)[gBattleStruct->scriptPartyIdx], MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_PC_CREATOR_NAME: // lanette pc
                if (FlagGet(FLAG_SYS_PC_LANETTE))
                    toCpy = IS_FRLG ? sText_Bills : sText_Lanettes;
                else
                    toCpy = sText_Someones;
                break;
            case B_TXT_ATK_PREFIX2:
                if (IsOnPlayerSide(gBattlerAttacker))
                    toCpy = sText_AllyPkmnPrefix2;
                else
                    toCpy = sText_FoePkmnPrefix3;
                break;
            case B_TXT_DEF_PREFIX2:
                if (IsOnPlayerSide(gBattlerTarget))
                    toCpy = sText_AllyPkmnPrefix2;
                else
                    toCpy = sText_FoePkmnPrefix3;
                break;
            case B_TXT_ATK_PREFIX1:
                if (IsOnPlayerSide(gBattlerAttacker))
                    toCpy = sText_AllyPkmnPrefix;
                else
                    toCpy = sText_FoePkmnPrefix2;
                break;
            case B_TXT_DEF_PREFIX1:
                if (IsOnPlayerSide(gBattlerTarget))
                    toCpy = sText_AllyPkmnPrefix;
                else
                    toCpy = sText_FoePkmnPrefix2;
                break;
            case B_TXT_ATK_PREFIX3:
                if (IsOnPlayerSide(gBattlerAttacker))
                    toCpy = sText_AllyPkmnPrefix3;
                else
                    toCpy = sText_FoePkmnPrefix4;
                break;
            case B_TXT_DEF_PREFIX3:
                if (IsOnPlayerSide(gBattlerTarget))
                    toCpy = sText_AllyPkmnPrefix3;
                else
                    toCpy = sText_FoePkmnPrefix4;
                break;
            case B_TXT_TRAINER2_CLASS:
                toCpy = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentB);
                break;
            case B_TXT_TRAINER2_NAME:
                toCpy = BattleStringGetOpponentNameByTrainerId(TRAINER_BATTLE_PARAM.opponentB, text, multiplayerId, GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT));
                break;
            case B_TXT_TRAINER2_NAME_WITH_CLASS:
                toCpy = textStart;
                classString = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentB);
                while (classString[classLength] != EOS)
                {
                    textStart[classLength] = classString[classLength];
                    classLength++;
                }
                textStart[classLength] = CHAR_SPACE;
                textStart += classLength + 1;
                nameString = BattleStringGetOpponentNameByTrainerId(TRAINER_BATTLE_PARAM.opponentB, textStart, multiplayerId, GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT));
                if (nameString != textStart)
                {
                    while (nameString[nameLength] != EOS)
                    {
                        textStart[nameLength] = nameString[nameLength];
                        nameLength++;
                    }
                    textStart[nameLength] = EOS;
                }
                break;
            case B_TXT_TRAINER2_LOSE_TEXT:
                if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
                {
                    CopyFrontierTrainerText(FRONTIER_PLAYER_WON_TEXT, TRAINER_BATTLE_PARAM.opponentB);
                    toCpy = gStringVar4;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER && gMapHeader.regionMapSectionId == MAPSEC_TRAINER_TOWER_2)
                {
                    GetTrainerTowerOpponentLoseText(gStringVar4, 1);
                    toCpy = gStringVar4;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL)
                {
                    CopyTrainerHillTrainerText(TRAINER_HILL_TEXT_PLAYER_WON, TRAINER_BATTLE_PARAM.opponentB);
                    toCpy = gStringVar4;
                }
                else
                {
                    toCpy = GetTrainerBLoseText();
                }
                break;
            case B_TXT_TRAINER2_WIN_TEXT:
                if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
                {
                    CopyFrontierTrainerText(FRONTIER_PLAYER_LOST_TEXT, TRAINER_BATTLE_PARAM.opponentB);
                    toCpy = gStringVar4;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER && gMapHeader.regionMapSectionId == MAPSEC_TRAINER_TOWER_2)
                {
                    GetTrainerTowerOpponentWinText(gStringVar4, 1);
                    toCpy = gStringVar4;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL)
                {
                    CopyTrainerHillTrainerText(TRAINER_HILL_TEXT_PLAYER_LOST, TRAINER_BATTLE_PARAM.opponentB);
                    toCpy = gStringVar4;
                }
                break;
            case B_TXT_PARTNER_CLASS:
                toCpy = gTrainerClasses[GetFrontierOpponentClass(gPartnerTrainerId)].name;
                break;
            case B_TXT_PARTNER_NAME:
                toCpy = BattleStringGetPlayerName(text, GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT));
                break;
            case B_TXT_PARTNER_NAME_WITH_CLASS:
                toCpy = textStart;
                classString = gTrainerClasses[GetFrontierOpponentClass(gPartnerTrainerId)].name;
                while (classString[classLength] != EOS)
                {
                    textStart[classLength] = classString[classLength];
                    classLength++;
                }
                textStart[classLength] = CHAR_SPACE;
                textStart += classLength + 1;
                nameString = BattleStringGetPlayerName(textStart, GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT));
                if (nameString != textStart)
                {
                    while (nameString[nameLength] != EOS)
                    {
                        textStart[nameLength] = nameString[nameLength];
                        nameLength++;
                    }
                    textStart[nameLength] = EOS;
                }
                break;
            case B_TXT_ATK_TRAINER_NAME:
                toCpy = BattleStringGetTrainerName(text, multiplayerId, gBattlerAttacker);
                break;
            case B_TXT_ATK_TRAINER_CLASS:
                switch (GetBattlerPosition(gBattlerAttacker))
                {
                case B_POSITION_PLAYER_RIGHT:
                    if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
                        toCpy = gTrainerClasses[GetFrontierOpponentClass(gPartnerTrainerId)].name;
                    break;
                case B_POSITION_OPPONENT_LEFT:
                    toCpy = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentA);
                    break;
                case B_POSITION_OPPONENT_RIGHT:
                    if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS && !BATTLE_TWO_VS_ONE_OPPONENT)
                        toCpy = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentB);
                    else
                        toCpy = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentA);
                    break;
                default:
                    break;
                }
                break;
            case B_TXT_ATK_TRAINER_NAME_WITH_CLASS:
                toCpy = textStart;
                if (gBattleTypeFlags & BATTLE_TYPE_CATCH_TUTORIAL)
                {
                    if (IS_FRLG)
                        textStart = StringCopy(textStart, COMPOUND_STRING(BATTLE_TEXT("The old man", "Stařík")));
                    else
                        textStart = StringCopy(textStart, COMPOUND_STRING("WALLY"));
                }
                else if (GetBattlerPosition(gBattlerAttacker) == B_POSITION_PLAYER_LEFT)
                {
                    textStart = StringCopy(textStart, BattleStringGetTrainerName(textStart, multiplayerId, gBattlerAttacker));
                }
                else
                {
                    classString = NULL;
                    switch (GetBattlerPosition(gBattlerAttacker))
                    {
                    case B_POSITION_PLAYER_RIGHT:
                        if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
                            classString = gTrainerClasses[GetFrontierOpponentClass(gPartnerTrainerId)].name;
                        break;
                    case B_POSITION_OPPONENT_LEFT:
                        classString = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentA);
                        break;
                    case B_POSITION_OPPONENT_RIGHT:
                        if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS && !BATTLE_TWO_VS_ONE_OPPONENT)
                            classString = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentB);
                        else
                            classString = BattleStringGetOpponentClassByTrainerId(TRAINER_BATTLE_PARAM.opponentA);
                        break;
                    default:
                        break;
                    }
                    classLength = 0;
                    nameLength = 0;
                    while (classString[classLength] != EOS)
                    {
                        textStart[classLength] = classString[classLength];
                        classLength++;
                    }
                    textStart[classLength] = CHAR_SPACE;
                    textStart += 1 + classLength;
                    nameString = BattleStringGetTrainerName(textStart, multiplayerId, gBattlerAttacker);
                    if (nameString != textStart)
                    {
                        while (nameString[nameLength] != EOS)
                        {
                            textStart[nameLength] = nameString[nameLength];
                            nameLength++;
                        }
                        textStart[nameLength] = EOS;
                    }
                }
                break;
            case B_TXT_ATK_TEAM1:
                if (IsOnPlayerSide(gBattlerAttacker))
                    toCpy = sText_Your1;
                else
                    toCpy = sText_Opposing1;
                break;
            case B_TXT_ATK_TEAM2:
                if (IsOnPlayerSide(gBattlerAttacker))
                    toCpy = sText_Your2;
                else
                    toCpy = sText_Opposing2;
                break;
            case B_TXT_DEF_TEAM1:
                if (IsOnPlayerSide(gBattlerTarget))
                    toCpy = sText_Your1;
                else
                    toCpy = sText_Opposing1;
                break;
            case B_TXT_DEF_TEAM2:
                if (IsOnPlayerSide(gBattlerTarget))
                    toCpy = sText_Your2;
                else
                    toCpy = sText_Opposing2;
                break;
            case B_TXT_EFF_TEAM1:
                if (IsOnPlayerSide(gEffectBattler))
                    toCpy = sText_Your1;
                else
                    toCpy = sText_Opposing1;
                break;
            case B_TXT_EFF_TEAM2:
                if (IsOnPlayerSide(gEffectBattler))
                    toCpy = sText_Your2;
                else
                    toCpy = sText_Opposing2;
                break;
            case B_TXT_ATK_NAME_WITH_PREFIX2:
                HANDLE_NICKNAME_STRING_LOWERCASE(gBattlerAttacker)
                break;
            case B_TXT_DEF_NAME_WITH_PREFIX2:
                HANDLE_NICKNAME_STRING_LOWERCASE(gBattlerTarget)
                break;
            case B_TXT_EFF_NAME_WITH_PREFIX2:
                HANDLE_NICKNAME_STRING_LOWERCASE(gEffectBattler)
                break;
            case B_TXT_SCR_ACTIVE_NAME_WITH_PREFIX2:
                HANDLE_NICKNAME_STRING_LOWERCASE(gBattleScripting.battler)
                break;
            }

            if (toCpy != NULL)
            {
                while (*toCpy != EOS)
                {
                    if (*toCpy == CHAR_SPACE)
                        dst[dstID] = CHAR_NBSP;
                    else
                        dst[dstID] = *toCpy;
                    dstID++;
                    toCpy++;
                }
            }

            if (*src == B_TXT_TRAINER1_LOSE_TEXT || *src == B_TXT_TRAINER2_LOSE_TEXT
                || *src == B_TXT_TRAINER1_WIN_TEXT || *src == B_TXT_TRAINER2_WIN_TEXT)
            {
                dst[dstID] = EXT_CTRL_CODE_BEGIN;
                dstID++;
                dst[dstID] = EXT_CTRL_CODE_PAUSE_UNTIL_PRESS;
                dstID++;
            }
        }
        else
        {
            dst[dstID] = *src;
            dstID++;
        }
        src++;
    }

    dst[dstID] = *src;
    dstID++;

    BreakStringAutomatic(dst, BATTLE_MSG_MAX_WIDTH, BATTLE_MSG_MAX_LINES, fontId, SHOW_SCROLL_PROMPT);

    return dstID;
}

static void IllusionNickHack(enum BattlerId battler, u32 partyId, u8 *dst)
{
    u32 id = PARTY_SIZE;
    struct Pokemon *party = GetBattlerParty(battler);
    struct Pokemon *mon = &party[partyId], *partnerMon;

    if (GetMonAbility(mon) == ABILITY_ILLUSION)
    {
        if (IsBattlerAlive(GetPartnerBattler(battler)))
            partnerMon = GetBattlerMon(GetPartnerBattler(battler));
        else
            partnerMon = mon;

        id = GetIllusionMonPartyId(party, mon, partnerMon, battler);
    }

    if (id != PARTY_SIZE)
        GetMonData(&party[id], MON_DATA_NICKNAME, dst);
    else
        GetMonData(mon, MON_DATA_NICKNAME, dst);
}

void ExpandBattleTextBuffPlaceholders(const u8 *src, u8 *dst)
{
    u32 srcID = 1;
    u32 value = 0;
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u16 hword;

    *dst = EOS;
    while (src[srcID] != B_BUFF_EOS)
    {
        switch (src[srcID])
        {
        case B_BUFF_STRING: // battle string
            hword = T1_READ_16(&src[srcID + 1]);
            StringAppend(dst, gBattleStringsTable[hword]);
            srcID += 3;
            break;
        case B_BUFF_NUMBER: // int to string
            switch (src[srcID + 1])
            {
            case 1:
                value = src[srcID + 3];
                break;
            case 2:
                value = T1_READ_16(&src[srcID + 3]);
                break;
            case 4:
                value = T1_READ_32(&src[srcID + 3]);
                break;
            }
            ConvertIntToDecimalStringN(dst, value, STR_CONV_MODE_LEFT_ALIGN, src[srcID + 2]);
            srcID += src[srcID + 1] + 3;
            break;
        case B_BUFF_MOVE: // move name
            StringAppend(dst, GetMoveName(T1_READ_16(&src[srcID + 1])));
            srcID += 3;
            break;
        case B_BUFF_TYPE: // type name
            StringAppend(dst, gTypesInfo[src[srcID + 1]].name);
            srcID += 2;
            break;
        case B_BUFF_MON_NICK_WITH_PREFIX: // poke nick with prefix
        case B_BUFF_MON_NICK_WITH_PREFIX_LOWER: // poke nick with lowercase prefix
            if (!IsOnPlayerSide(src[srcID + 1]))
            {
                if (src[srcID] == B_BUFF_MON_NICK_WITH_PREFIX_LOWER)
                {
                    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
                        StringAppend(dst, sText_FoePkmnPrefixLower);
                    else
                        StringAppend(dst, sText_WildPkmnPrefixLower);
                }
                else
                {
                    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
                        StringAppend(dst, sText_FoePkmnPrefix);
                    else
                        StringAppend(dst, sText_WildPkmnPrefix);
                }
            }
            GetMonData(&GetBattlerParty(src[srcID + 1])[src[srcID + 2]], MON_DATA_NICKNAME, nickname);
            StringGet_Nickname(nickname);
            StringAppend(dst, nickname);
            srcID += 3;
            break;
        case B_BUFF_STAT: // stats
            StringAppend(dst, gStatNamesTable[src[srcID + 1]]);
            srcID += 2;
            break;
        case B_BUFF_SPECIES: // species name
            StringCopy(dst, GetSpeciesName(T1_READ_16(&src[srcID + 1])));
            srcID += 3;
            break;
        case B_BUFF_MON_NICK: // poke nick without prefix
            if (src[srcID + 2] == gBattlerPartyIndexes[src[srcID + 1]])
            {
                GetBattlerNick(src[srcID + 1], dst);
            }
            else if (gBattleScripting.illusionNickHack) // for STRINGID_ENEMYABOUTTOSWITCHPKMN
            {
                gBattleScripting.illusionNickHack = 0;
                IllusionNickHack(src[srcID + 1], src[srcID + 2], dst);
                StringGet_Nickname(dst);
            }
            else
            {
                GetMonData(&GetBattlerParty(src[srcID + 1])[src[srcID + 2]], MON_DATA_NICKNAME, dst);
                StringGet_Nickname(dst);
            }
            srcID += 3;
            break;
        case B_BUFF_NEGATIVE_FLAVOR: // flavor table
            StringAppend(dst, gPokeblockWasTooXStringTable[src[srcID + 1]]);
            srcID += 2;
            break;
        case B_BUFF_ABILITY: // ability names
            StringAppend(dst, gAbilitiesInfo[T1_READ_16(&src[srcID + 1])].name);
            srcID += 3;
            break;
        case B_BUFF_ITEM: // item name
            hword = T1_READ_16(&src[srcID + 1]);
            if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
            {
                if (hword == ITEM_ENIGMA_BERRY_E_READER)
                {
                    if (gLinkPlayers[gBattleScripting.multiplayerId].id == gPotentialItemEffectBattler)
                    {
                        StringCopy(dst, gEnigmaBerries[gPotentialItemEffectBattler].name);
                        StringAppend(dst, sText_BerrySuffix);
                    }
                    else
                    {
                        StringAppend(dst, sText_EnigmaBerry);
                    }
                }
                else
                {
                    CopyItemName(hword, dst);
                }
            }
            else
            {
                CopyItemName(hword, dst);
            }
            srcID += 3;
            break;
        }
    }
}

void BattlePutTextOnWindow(const u8 *text, u8 windowId)
{
    const struct BattleWindowText *textInfo = sBattleTextOnWindowsInfo[gBattleScripting.windowsType];
    bool32 copyToVram;
    struct TextPrinterTemplate printerTemplate;
    u8 speed;

    if (windowId & B_WIN_COPYTOVRAM)
    {
        windowId &= ~B_WIN_COPYTOVRAM;
        copyToVram = FALSE;
    }
    else
    {
        FillWindowPixelBuffer(windowId, textInfo[windowId].fillValue);
        copyToVram = TRUE;
    }

    printerTemplate.currentChar = text;
    printerTemplate.type = WINDOW_TEXT_PRINTER;
    printerTemplate.windowId = windowId;
    printerTemplate.fontId = textInfo[windowId].fontId;
    printerTemplate.x = textInfo[windowId].x;
    printerTemplate.y = textInfo[windowId].y;
    printerTemplate.currentX = printerTemplate.x;
    printerTemplate.currentY = printerTemplate.y;
    printerTemplate.letterSpacing = textInfo[windowId].letterSpacing;
    printerTemplate.lineSpacing = textInfo[windowId].lineSpacing;
    printerTemplate.color = textInfo[windowId].color;

    if (B_WIN_MOVE_NAME_1 <= windowId && windowId <= B_WIN_MOVE_NAME_4)
    {
        // We cannot check the actual width of the window because
        // B_WIN_MOVE_NAME_1 and B_WIN_MOVE_NAME_3 are 16 wide for
        // Z-move details.
        if (gBattleStruct->zmove.viewing && windowId == B_WIN_MOVE_NAME_1)
            printerTemplate.fontId = GetFontIdToFit(text, printerTemplate.fontId, printerTemplate.letterSpacing, 16 * TILE_WIDTH);
        else
            printerTemplate.fontId = GetFontIdToFit(text, printerTemplate.fontId, printerTemplate.letterSpacing, 8 * TILE_WIDTH);
    }

    if (printerTemplate.x == 0xFF)
    {
        u32 width = GetBattleWindowTemplatePixelWidth(gBattleScripting.windowsType, windowId);
        s32 alignX = GetStringCenterAlignXOffsetWithLetterSpacing(printerTemplate.fontId, printerTemplate.currentChar, width, printerTemplate.letterSpacing);
        printerTemplate.x = printerTemplate.currentX = alignX;
    }

    if (windowId == ARENA_WIN_JUDGMENT_TEXT || windowId == B_WIN_OAK_OLD_MAN)
        gTextFlags.useAlternateDownArrow = FALSE;
    else
        gTextFlags.useAlternateDownArrow = TRUE;

    if ((gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED)) || gTestRunnerEnabled || ((gBattleTypeFlags & BATTLE_TYPE_POKEDUDE) && windowId != B_WIN_OAK_OLD_MAN))
        gTextFlags.autoScroll = TRUE;
    else
        gTextFlags.autoScroll = FALSE;

    if (windowId == B_WIN_MSG || windowId == ARENA_WIN_JUDGMENT_TEXT || windowId == B_WIN_OAK_OLD_MAN)
    {
        if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
            speed = 1;
        else if (gBattleTypeFlags & BATTLE_TYPE_RECORDED)
            speed = sRecordedBattleTextSpeeds[GetTextSpeedInRecordedBattle()];
        else
            speed = GetPlayerTextSpeedDelay();

        gTextFlags.canABSpeedUpPrint = 1;
    }
    else
    {
        speed = textInfo[windowId].speed;
        gTextFlags.canABSpeedUpPrint = 0;
    }

    AddTextPrinter(&printerTemplate, speed, NULL);

    if (copyToVram)
    {
        PutWindowTilemap(windowId);
        CopyWindowToVram(windowId, COPYWIN_FULL);
    }
}

void SetPPNumbersPaletteInMoveSelection(enum BattlerId battler)
{
    struct ChooseMoveStruct *chooseMoveStruct = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    const u16 *palPtr = gPPTextPalette;
    u8 var;

    if (!gBattleStruct->zmove.viewing)
        var = GetCurrentPPToMaxPPState(chooseMoveStruct->currentPP[gMoveSelectionCursor[battler]],
                         chooseMoveStruct->maxPP[gMoveSelectionCursor[battler]]);
    else
        var = 3;

    gPlttBufferUnfaded[BG_PLTT_ID(5) + 12] = palPtr[(var * 2) + 0];
    gPlttBufferUnfaded[BG_PLTT_ID(5) + 11] = palPtr[(var * 2) + 1];

    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(5) + 12], &gPlttBufferFaded[BG_PLTT_ID(5) + 12], PLTT_SIZEOF(1));
    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(5) + 11], &gPlttBufferFaded[BG_PLTT_ID(5) + 11], PLTT_SIZEOF(1));
}

u8 GetCurrentPPToMaxPPState(u8 currentPP, u8 maxPP)
{
    if (maxPP == currentPP)
    {
        return 3;
    }
    else if (maxPP <= 2)
    {
        if (currentPP > 1)
            return 3;
        else
            return 2 - currentPP;
    }
    else if (maxPP <= 7)
    {
        if (currentPP > 2)
            return 3;
        else
            return 2 - currentPP;
    }
    else
    {
        if (currentPP == 0)
            return 2;
        if (currentPP <= maxPP / 4)
            return 1;
        if (currentPP > maxPP / 2)
            return 3;
    }

    return 0;
}
