#include "localization.h"

extern const u8 EventScript_FollowerIsShivering[];
extern const u8 EventScript_FollowerNostalgia[];
extern const u8 EventScript_FollowerHopping[];
extern const u8 EventScript_FollowerJumpOnPlayer[];
extern const u8 EventScript_FollowerCuddling[];
extern const u8 EventScript_FollowerShiverCuddling[];
extern const u8 EventScript_FollowerGetCloser[];
extern const u8 EventScript_FollowerPokingPlayer[];
extern const u8 EventScript_FollowerLookAround[];
extern const u8 EventScript_FollowerLookAway[];
extern const u8 EventScript_FollowerLookAwayBark[];
extern const u8 EventScript_FollowerLookAwayPoke[];
extern const u8 EventScript_FollowerPokeGround[];
extern const u8 EventScript_FollowerStartled[];
extern const u8 EventScript_FollowerFastHopping[];
extern const u8 EventScript_FollowerDizzy[];
extern const u8 EventScript_FollowerLookAroundScared[];
extern const u8 EventScript_FollowerDance[];
extern const u8 EventScript_FollowerLookUp[];

// 'Generic', unconditional happy messages
static const u8 sHappyMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} began poking you in the\nstomach.", "{STR_VAR_1} tě šťouchá do břicha."));
static const u8 sHappyMsg01[] = _(LOCALIZED_TEXT("{STR_VAR_1} is happy but shy.", "{STR_VAR_1} má radost, ale stydí se."));
static const u8 sHappyMsg02[] = _(LOCALIZED_TEXT("{STR_VAR_1} is coming along happily.", "{STR_VAR_1} tě vesele doprovází."));
static const u8 sHappyMsg03[] = _(LOCALIZED_TEXT("{STR_VAR_1} is composed.", "{STR_VAR_1} působí klidně."));
static const u8 sHappyMsg04[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems to be feeling\ngreat about walking with you!", "{STR_VAR_1} si procházku s tebou moc\nužívá!"));
static const u8 sHappyMsg05[] = _(LOCALIZED_TEXT("{STR_VAR_1} is glowing with health.", "{STR_VAR_1} jen září zdravím."));
static const u8 sHappyMsg06[] = _(LOCALIZED_TEXT("{STR_VAR_1} looks very happy.", "{STR_VAR_1} vypadá velmi šťastně."));
static const u8 sHappyMsg07[] = _(LOCALIZED_TEXT("{STR_VAR_1} put in extra effort.", "{STR_VAR_1} se snaží ještě víc."));
static const u8 sHappyMsg08[] = _(LOCALIZED_TEXT("{STR_VAR_1} is smelling the scents\nof the surrounding air.", "{STR_VAR_1} nasává vůně z okolí."));
static const u8 sHappyMsg09[] = _(LOCALIZED_TEXT("{STR_VAR_1} is jumping for joy!", "{STR_VAR_1} skáče radostí!"));
static const u8 sHappyMsg10[] = _(LOCALIZED_TEXT("{STR_VAR_1} is still feeling great!", "{STR_VAR_1} se pořád cítí skvěle!"));
static const u8 sHappyMsg11[] = _(LOCALIZED_TEXT("Your POKéMON has caught the scent of\nsmoke.", "Tvůj Pokémon ucítil kouř."));
static const u8 sHappyMsg12[] = _(LOCALIZED_TEXT("{STR_VAR_1} is poking at your belly.", "{STR_VAR_1} tě šťouchá do břicha."));
static const u8 sHappyMsg13[] = _(LOCALIZED_TEXT("Your POKéMON stretched out its body\nand is relaxing.", "Tvůj Pokémon se protáhl a odpočívá."));
static const u8 sHappyMsg14[] = _(LOCALIZED_TEXT("{STR_VAR_1} looks like it wants to\nlead!", "{STR_VAR_1} chce asi vést cestu!"));
static const u8 sHappyMsg15[] = _(LOCALIZED_TEXT("{STR_VAR_1} is doing its best to\nkeep up with you.", "{STR_VAR_1} se snaží držet s tebou\nkrok."));
static const u8 sHappyMsg16[] = _(LOCALIZED_TEXT("{STR_VAR_1} is happily cuddling up\nto you!", "{STR_VAR_1} se k tobě radostně tulí!"));
static const u8 sHappyMsg17[] = _(LOCALIZED_TEXT("{STR_VAR_1} is full of life!", "{STR_VAR_1} překypuje energií!"));
static const u8 sHappyMsg18[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems to be very happy!", "{STR_VAR_1} vypadá moc šťastně!"));
static const u8 sHappyMsg19[] = _(LOCALIZED_TEXT("{STR_VAR_1} is so happy that it\ncan't stand still!", "{STR_VAR_1} radostí nepostojí!"));
static const u8 sHappyMsg20[] = _(LOCALIZED_TEXT("{STR_VAR_1} nodded slowly.", "{STR_VAR_1} pomalu přikývl."));
static const u8 sHappyMsg21[] = _(LOCALIZED_TEXT("{STR_VAR_1} is very eager!", "{STR_VAR_1} je plný nadšení!"));
static const u8 sHappyMsg22[] = _(LOCALIZED_TEXT("{STR_VAR_1} is wandering around and\nlistening to the different sounds.", "{STR_VAR_1} se toulá a naslouchá\nokolním zvukům."));
static const u8 sHappyMsg23[] = _(LOCALIZED_TEXT("{STR_VAR_1} looks very interested.", "{STR_VAR_1} vypadá velmi zaujatě."));
static const u8 sHappyMsg24[] = _(LOCALIZED_TEXT("{STR_VAR_1} is somehow forcing\nitself to keep going.", "{STR_VAR_1} se nutí jít dál."));
static const u8 sHappyMsg25[] = _(LOCALIZED_TEXT("{STR_VAR_1} gave you a sunny look!", "{STR_VAR_1} na tebe vesele pohlédl!"));
static const u8 sHappyMsg26[] = _(LOCALIZED_TEXT("{STR_VAR_1} gives you a happy look\nand a smile.", "{STR_VAR_1} na tebe hledí s úsměvem."));
static const u8 sHappyMsg27[] = _(LOCALIZED_TEXT("Your POKéMON is smelling the scent\nof flowers.", "Tvůj Pokémon nasává vůni květin."));
static const u8 sHappyMsg28[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems very happy to see\nyou!", "{STR_VAR_1} má radost, že tě vidí!"));
static const u8 sHappyMsg29[] = _(LOCALIZED_TEXT("{STR_VAR_1} faced this way and\ngrinned.", "{STR_VAR_1} se otočil a zazubil."));
static const u8 sHappyMsg30[] = _(LOCALIZED_TEXT("{STR_VAR_1} happily cuddled up to\nyou!", "{STR_VAR_1} se k tobě vesele přitulil!"));
// Conditional messages begin here, index 31
static const u8 sHappyMsg31[] = _(LOCALIZED_TEXT("Your POKéMON seems happy about the\ngreat weather.", "Tvůj Pokémon má radost z krásného\npočasí."));
static const u8 sHappyMsg32[] = _(LOCALIZED_TEXT("{STR_VAR_1} is very composed and\nsure of itself!", "{STR_VAR_1} je klidný a sebejistý!"));

const struct FollowerMsgInfo gFollowerHappyMessages[] = {
    {sHappyMsg00, EventScript_FollowerPokingPlayer},
    {sHappyMsg01}, {sHappyMsg02}, {sHappyMsg03}, {sHappyMsg04}, {sHappyMsg05}, {sHappyMsg06}, {sHappyMsg07},
    {sHappyMsg08, EventScript_FollowerLookAround},
    {sHappyMsg09, EventScript_FollowerHopping},
    {sHappyMsg10}, {sHappyMsg11},
    {sHappyMsg12, EventScript_FollowerPokingPlayer},
    {sHappyMsg13, EventScript_FollowerLookAround},
    {sHappyMsg14}, {sHappyMsg15},
    {sHappyMsg16, EventScript_FollowerCuddling},
    {sHappyMsg17}, {sHappyMsg18},
    {sHappyMsg19, EventScript_FollowerFastHopping},
    {sHappyMsg20}, {sHappyMsg21}, {sHappyMsg22}, {sHappyMsg23}, {sHappyMsg24}, {sHappyMsg25}, {sHappyMsg26}, {sHappyMsg27}, {sHappyMsg28}, {sHappyMsg29},
    {sHappyMsg30, EventScript_FollowerCuddling},
    {sHappyMsg31}, {sHappyMsg32},
};

// Unconditional neutral messages
static const u8 sNeutralMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} is steadily poking at\nthe ground.", "{STR_VAR_1} vytrvale rýpe do země."));
static const u8 sNeutralMsg01[] = _(LOCALIZED_TEXT("{STR_VAR_1} is standing guard.", "{STR_VAR_1} stojí na stráži."));
static const u8 sNeutralMsg02[] = _(LOCALIZED_TEXT("{STR_VAR_1} is staring patiently at\nnothing at all.", "{STR_VAR_1} trpělivě hledí do\nprázdna."));
static const u8 sNeutralMsg03[] = _(LOCALIZED_TEXT("{STR_VAR_1} is wandering around.", "{STR_VAR_1} se toulá po okolí."));
static const u8 sNeutralMsg04[] = _(LOCALIZED_TEXT("Your POKéMON yawned loudly!", "Tvůj Pokémon hlasitě zívl!"));
static const u8 sNeutralMsg05[] = _(LOCALIZED_TEXT("Your POKéMON is looking around\nrestlessly.", "Tvůj Pokémon se neklidně rozhlíží."));
static const u8 sNeutralMsg06[] = _(LOCALIZED_TEXT("{STR_VAR_1} is looking this way and\nsmiling.", "{STR_VAR_1} na tebe hledí s úsměvem."));
static const u8 sNeutralMsg07[] = _(LOCALIZED_TEXT("{STR_VAR_1} is gazing around\nrestlessly.", "{STR_VAR_1} se neklidně rozhlíží."));
static const u8 sNeutralMsg08[] = _(LOCALIZED_TEXT("{STR_VAR_1} let out a battle cry.", "{STR_VAR_1} vydal bojový pokřik."));
static const u8 sNeutralMsg09[] = _(LOCALIZED_TEXT("{STR_VAR_1} danced a wonderful\ndance!", "{STR_VAR_1} předvedl nádherný tanec!"));
static const u8 sNeutralMsg10[] = _(LOCALIZED_TEXT("{STR_VAR_1} is very eager.", "{STR_VAR_1} je plný nadšení."));
static const u8 sNeutralMsg11[] = _(LOCALIZED_TEXT("{STR_VAR_1} is staring intently into\nthe distance.", "{STR_VAR_1} upřeně hledí do dálky."));
static const u8 sNeutralMsg12[] = _(LOCALIZED_TEXT("{STR_VAR_1} is on the lookout!", "{STR_VAR_1} je ve střehu!"));
static const u8 sNeutralMsg13[] = _(LOCALIZED_TEXT("{STR_VAR_1} looked off into the\ndistance and barked!", "{STR_VAR_1} pohlédl do dálky a\nzaštěkal!"));

const struct FollowerMsgInfo gFollowerNeutralMessages[] = {
    {sNeutralMsg00, EventScript_FollowerPokeGround},
    {sNeutralMsg01},
    {sNeutralMsg02, EventScript_FollowerLookAway},
    {sNeutralMsg03, EventScript_FollowerLookAround},
    {sNeutralMsg04},
    {sNeutralMsg05, EventScript_FollowerLookAround},
    {sNeutralMsg06}, {sNeutralMsg07}, {sNeutralMsg08},
    {sNeutralMsg09, EventScript_FollowerDance},
    {sNeutralMsg10},
    {sNeutralMsg11, EventScript_FollowerLookAway},
    {sNeutralMsg12},
    {sNeutralMsg13, EventScript_FollowerLookAwayBark},
};

// Unconditional sad messages
static const u8 sSadMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} is dizzy.", "{STR_VAR_1} má závrať."));
static const u8 sSadMsg01[] = _(LOCALIZED_TEXT("{STR_VAR_1} is stepping on your\nfeet!", "{STR_VAR_1} ti šlape na nohy!"));
static const u8 sSadMsg02[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems a little tired.", "{STR_VAR_1} vypadá trochu unaveně."));
// Conditional messages begin, index 3
static const u8 sSadMsg03[] = _(LOCALIZED_TEXT("{STR_VAR_1} is not happy.", "{STR_VAR_1} není šťastný."));
static const u8 sSadMsg04[] = _(LOCALIZED_TEXT("{STR_VAR_1} is going to fall down!\n", "{STR_VAR_1} každou chvíli upadne!"));
static const u8 sSadMsg05[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems to be about to\nfall over!", "{STR_VAR_1} se asi brzy svalí!"));
static const u8 sSadMsg06[] = _(LOCALIZED_TEXT("{STR_VAR_1} is trying very hard to\nkeep up with you…", "{STR_VAR_1} se ze všech sil snaží\ndržet krok…"));
static const u8 sSadMsg07[] = _(LOCALIZED_TEXT("{STR_VAR_1} is nervous.", "{STR_VAR_1} je nervózní."));

const struct FollowerMsgInfo gFollowerSadMessages[] = {
    {sSadMsg00, EventScript_FollowerDizzy},
    {sSadMsg01}, {sSadMsg02},
    {sSadMsg03}, {sSadMsg04}, {sSadMsg05}, {sSadMsg06}, {sSadMsg07},
};

// Unconditional upset messages
static const u8 sUpsetMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems unhappy somehow…", "{STR_VAR_1} vypadá nějak\nnešťastně…"));
static const u8 sUpsetMsg01[] = _(LOCALIZED_TEXT("{STR_VAR_1} is making an unhappy\nface.", "{STR_VAR_1} se nešťastně tváří."));
static const u8 sUpsetMsg02[] = _(LOCALIZED_TEXT("…Your POKéMON seems a little\ncold.", "…Tvému Pokémonovi je asi trochu zima."));
// Conditional messages, index 3
static const u8 sUpsetMsg03[] = _(LOCALIZED_TEXT("{STR_VAR_1} is taking shelter in the\ngrass from the rain.", "{STR_VAR_1} se v trávě schovává před\ndeštěm."));

const struct FollowerMsgInfo gFollowerUpsetMessages[] = {
    {sUpsetMsg00}, {sUpsetMsg01},
    {sUpsetMsg02, EventScript_FollowerIsShivering},
    {sUpsetMsg03},
};

// Unconditional angry messages
static const u8 sAngryMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} let out a roar!", "{STR_VAR_1} hlasitě zařval!"));
static const u8 sAngryMsg01[] = _(LOCALIZED_TEXT("{STR_VAR_1} is making a face like\nit's angry!", "{STR_VAR_1} se tváří rozzlobeně!"));
static const u8 sAngryMsg02[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems to be angry for\nsome reason.", "{STR_VAR_1} se z nějakého důvodu\nzlobí."));
static const u8 sAngryMsg03[] = _(LOCALIZED_TEXT("Your POKéMON turned to face the\nother way, showing a defiant face.", "Tvůj Pokémon se vzdorovitě odvrátil."));
static const u8 sAngryMsg04[] = _(LOCALIZED_TEXT("{STR_VAR_1} cried out.", "{STR_VAR_1} vykřikl."));

const struct FollowerMsgInfo gFollowerAngryMessages[] = {
    {sAngryMsg00}, {sAngryMsg01}, {sAngryMsg02},
    {sAngryMsg03, EventScript_FollowerLookAway},
    {sAngryMsg04},
};

// Unconditional pensive messages
static const u8 sPensiveMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} is looking down\nsteadily.", "{STR_VAR_1} vytrvale hledí k zemi."));
static const u8 sPensiveMsg01[] = _(LOCALIZED_TEXT("{STR_VAR_1} is surveying the area.", "{STR_VAR_1} zkoumá okolí."));
static const u8 sPensiveMsg02[] = _(LOCALIZED_TEXT("{STR_VAR_1} is peering down.", "{STR_VAR_1} se dívá dolů."));
static const u8 sPensiveMsg03[] = _(LOCALIZED_TEXT("{STR_VAR_1} is somehow fighting off\nsleep…", "{STR_VAR_1} se snaží neusnout…"));
static const u8 sPensiveMsg04[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems to be wandering\naround.", "{STR_VAR_1} se asi toulá po okolí."));
static const u8 sPensiveMsg05[] = _(LOCALIZED_TEXT("{STR_VAR_1} is looking around\nabsentmindedly.", "{STR_VAR_1} se nepřítomně rozhlíží."));
static const u8 sPensiveMsg06[] = _(LOCALIZED_TEXT("{STR_VAR_1} yawned very loudly!", "{STR_VAR_1} velmi hlasitě zívl!"));
static const u8 sPensiveMsg07[] = _(LOCALIZED_TEXT("{STR_VAR_1} is relaxing comfortably.", "{STR_VAR_1} spokojeně odpočívá."));
static const u8 sPensiveMsg08[] = _(LOCALIZED_TEXT("{STR_VAR_1} is staring steadfastly\nat your face.", "{STR_VAR_1} ti vytrvale hledí do\ntváře."));
static const u8 sPensiveMsg09[] = _(LOCALIZED_TEXT("{STR_VAR_1} is staring intently at\nyour face.", "{STR_VAR_1} ti upřeně hledí do tváře."));
static const u8 sPensiveMsg10[] = _(LOCALIZED_TEXT("{STR_VAR_1} is focusing its\nattention on you.", "{STR_VAR_1} ti věnuje plnou\npozornost."));
static const u8 sPensiveMsg11[] = _(LOCALIZED_TEXT("{STR_VAR_1} is staring into the\ndepths.", "{STR_VAR_1} hledí do hlubin."));
static const u8 sPensiveMsg12[] = _(LOCALIZED_TEXT("{STR_VAR_1} is sniffing at the\nground.", "{STR_VAR_1} očichává zem."));
static const u8 sPensiveMsg13[] = _(LOCALIZED_TEXT("Your POKéMON is staring intently at\nnothing.", "Tvůj Pokémon upřeně hledí do prázdna."));
static const u8 sPensiveMsg14[] = _(LOCALIZED_TEXT("{STR_VAR_1} focused with a sharp\ngaze!", "{STR_VAR_1} zaostřil bystrý pohled!"));
static const u8 sPensiveMsg15[] = _(LOCALIZED_TEXT("{STR_VAR_1} is concentrating.", "{STR_VAR_1} se soustředí."));
static const u8 sPensiveMsg16[] = _(LOCALIZED_TEXT("{STR_VAR_1} faced this way and\nnodded.", "{STR_VAR_1} se otočil a přikývl."));
static const u8 sPensiveMsg17[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems a bit nervous…", "{STR_VAR_1} vypadá trochu nervózně…"));
static const u8 sPensiveMsg18[] = _(LOCALIZED_TEXT("{STR_VAR_1} is looking at your\nfootprints.", "{STR_VAR_1} si prohlíží tvé stopy."));
static const u8 sPensiveMsg19[] = _(LOCALIZED_TEXT("{STR_VAR_1} is staring straight into\nyour eyes.", "{STR_VAR_1} ti hledí přímo do očí."));

const struct FollowerMsgInfo gFollowerPensiveMessages[] = {
    {sPensiveMsg00},
    {sPensiveMsg01, EventScript_FollowerLookAround},
    {sPensiveMsg02}, {sPensiveMsg03}, {sPensiveMsg04},
    {sPensiveMsg05, EventScript_FollowerLookAround},
    {sPensiveMsg06}, {sPensiveMsg07}, {sPensiveMsg08}, {sPensiveMsg09}, {sPensiveMsg10},
    {sPensiveMsg11, EventScript_FollowerLookAway},
    {sPensiveMsg12, EventScript_FollowerPokeGround},
    {sPensiveMsg13, EventScript_FollowerLookAway},
    {sPensiveMsg14}, {sPensiveMsg15}, {sPensiveMsg16}, {sPensiveMsg17}, {sPensiveMsg18}, {sPensiveMsg19},
};

// All 'love' messages are unconditional
static const u8 sLoveMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} suddenly started walking\ncloser!", "{STR_VAR_1} k tobě náhle přišel blíž!"));
static const u8 sLoveMsg01[] = _(LOCALIZED_TEXT("{STR_VAR_1} cheeks are becoming\nrosy!", "{STR_VAR_1} má růžové tváře!"));
static const u8 sLoveMsg02[] = _(LOCALIZED_TEXT("Woah! {STR_VAR_1} suddenly hugged\nyou!", "Jé! {STR_VAR_1} tě náhle objal!"));
static const u8 sLoveMsg03[] = _(LOCALIZED_TEXT("Woah! {STR_VAR_1} is suddenly\nplayful!", "Jé! {STR_VAR_1} si najednou chce\nhrát!"));
static const u8 sLoveMsg04[] = _(LOCALIZED_TEXT("{STR_VAR_1} is rubbing against your\nlegs!", "{STR_VAR_1} se ti otírá o nohy!"));
static const u8 sLoveMsg05[] = _(LOCALIZED_TEXT("{STR_VAR_1} blushes.", "{STR_VAR_1} se červená."));
static const u8 sLoveMsg06[] = _(LOCALIZED_TEXT("Ah! {STR_VAR_1} cuddles you!", "Jé! {STR_VAR_1} se k tobě tulí!"));
static const u8 sLoveMsg07[] = _(LOCALIZED_TEXT("{STR_VAR_1} is regarding you with\nadoration!", "{STR_VAR_1} na tebe hledí s láskou!"));
static const u8 sLoveMsg08[] = _(LOCALIZED_TEXT("{STR_VAR_1} got closer to you.", "{STR_VAR_1} k tobě přišel blíž."));
static const u8 sLoveMsg09[] = _(LOCALIZED_TEXT("{STR_VAR_1} is keeping close to your\nfeet.", "{STR_VAR_1} se ti drží u nohou."));

const struct FollowerMsgInfo gFollowerLoveMessages[] = {
    {sLoveMsg00, EventScript_FollowerGetCloser},
    {sLoveMsg01},
    {sLoveMsg02, EventScript_FollowerCuddling},
    {sLoveMsg03},
    {sLoveMsg04, EventScript_FollowerCuddling},
    {sLoveMsg05},
    {sLoveMsg06, EventScript_FollowerCuddling},
    {sLoveMsg07},
    {sLoveMsg08, EventScript_FollowerGetCloser},
    {sLoveMsg09},
};

// Unconditional surprised messages
static const u8 sSurpriseMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} is in danger of falling\nover!", "{STR_VAR_1} každou chvíli upadne!"));
static const u8 sSurpriseMsg01[] = _(LOCALIZED_TEXT("{STR_VAR_1} bumped into you!", "{STR_VAR_1} do tebe vrazil!"));
static const u8 sSurpriseMsg02[] = _(LOCALIZED_TEXT("{STR_VAR_1} doesn't seem to be used\nto its own name yet.", "{STR_VAR_1} si ještě nezvykl na své\njméno."));
static const u8 sSurpriseMsg03[] = _(LOCALIZED_TEXT("{STR_VAR_1} is peering down.", "{STR_VAR_1} se dívá dolů."));
static const u8 sSurpriseMsg04[] = _(LOCALIZED_TEXT("Your POKéMON stumbled and nearly\nfell!", "Tvůj Pokémon klopýtl a málem upadl!"));
static const u8 sSurpriseMsg05[] = _(LOCALIZED_TEXT("{STR_VAR_1} feels something and is\nhowling!", "{STR_VAR_1} něco cítí a vyje!"));
static const u8 sSurpriseMsg06[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems refreshed!", "{STR_VAR_1} vypadá svěže!"));
static const u8 sSurpriseMsg07[] = _(LOCALIZED_TEXT("{STR_VAR_1} suddenly turned around\nand started barking!", "{STR_VAR_1} se náhle otočil a\nzaštěkal!"));
static const u8 sSurpriseMsg08[] = _(LOCALIZED_TEXT("{STR_VAR_1} suddenly turned around!", "{STR_VAR_1} se náhle otočil!"));
static const u8 sSurpriseMsg09[] = _(LOCALIZED_TEXT("Your POKéMON was surprised that you\nsuddenly spoke to it!", "Tvého Pokémona překvapilo, že na něj\nnáhle mluvíš!"));
static const u8 sSurpriseMsg10[] = _(LOCALIZED_TEXT("Sniff sniff, something smells really\ngood!", "Čmuch, čmuch! Tady něco krásně voní!"));
static const u8 sSurpriseMsg11[] = _(LOCALIZED_TEXT("{STR_VAR_1} feels refreshed.", "{STR_VAR_1} se cítí svěže."));
static const u8 sSurpriseMsg12[] = _(LOCALIZED_TEXT("{STR_VAR_1} is wobbling and seems\nabout to fall over.", "{STR_VAR_1} se kymácí a málem padá."));
static const u8 sSurpriseMsg13[] = _(LOCALIZED_TEXT("{STR_VAR_1} is in danger of falling\nover.", "{STR_VAR_1} každou chvíli upadne."));
static const u8 sSurpriseMsg14[] = _(LOCALIZED_TEXT("{STR_VAR_1} is walking along\ncautiously.", "{STR_VAR_1} kráčí opatrně."));
static const u8 sSurpriseMsg15[] = _(LOCALIZED_TEXT("{STR_VAR_1} is getting tense with\nnervous energy.", "{STR_VAR_1} je čím dál nervóznější."));
static const u8 sSurpriseMsg16[] = _(LOCALIZED_TEXT("{STR_VAR_1} sensed something strange\nand was surprised!", "{STR_VAR_1} ucítil něco zvláštního a\npolekal se!"));
static const u8 sSurpriseMsg17[] = _(LOCALIZED_TEXT("{STR_VAR_1} is scared and snuggled\nup to you!", "{STR_VAR_1} se bojí a přitulil se k\ntobě!"));
static const u8 sSurpriseMsg18[] = _(LOCALIZED_TEXT("{STR_VAR_1} is feeling an unusual\npresence…", "{STR_VAR_1} cítí zvláštní\npřítomnost…"));
static const u8 sSurpriseMsg19[] = _(LOCALIZED_TEXT("{STR_VAR_1} is getting tense with\nnervous energy.", "{STR_VAR_1} je čím dál nervóznější."));
// Conditional messages, index 20
static const u8 sSurpriseMsg20[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems to be very\nsurprised that it is raining!", "{STR_VAR_1} se moc diví, že prší!"));

const struct FollowerMsgInfo gFollowerSurpriseMessages[] = {
    {sSurpriseMsg00},
    {sSurpriseMsg01, EventScript_FollowerPokingPlayer},
    {sSurpriseMsg02}, {sSurpriseMsg03}, {sSurpriseMsg04}, {sSurpriseMsg05}, {sSurpriseMsg06},
    {sSurpriseMsg07, EventScript_FollowerLookAwayBark},
    {sSurpriseMsg08, EventScript_FollowerLookAway},
    {sSurpriseMsg09},
    {sSurpriseMsg10, EventScript_FollowerLookAround},
    {sSurpriseMsg11}, {sSurpriseMsg12}, {sSurpriseMsg13}, {sSurpriseMsg14}, {sSurpriseMsg15}, {sSurpriseMsg16},
    {sSurpriseMsg17, EventScript_FollowerCuddling},
    {sSurpriseMsg18},
    {sSurpriseMsg19, EventScript_FollowerLookAround},
    {sSurpriseMsg20},
};

// Unconditional curious messages
static const u8 sCuriousMsg00[] = _(LOCALIZED_TEXT("Your POKéMON is looking around\nrestlessly for something.", "Tvůj Pokémon se neklidně rozhlíží a\nněco hledá."));
static const u8 sCuriousMsg01[] = _(LOCALIZED_TEXT("Your POKéMON wasn't watching where\nit was going and ran into you!", "Tvůj Pokémon nedával pozor na cestu a\nvrazil do tebe!"));
static const u8 sCuriousMsg02[] = _(LOCALIZED_TEXT("Sniff, sniff! Is there something\nnearby?", "Čmuch, čmuch! Je tu něco poblíž?"));
static const u8 sCuriousMsg03[] = _(LOCALIZED_TEXT("{STR_VAR_1} is rolling a pebble\naround playfully.", "{STR_VAR_1} si hraje s oblázkem."));
static const u8 sCuriousMsg04[] = _(LOCALIZED_TEXT("{STR_VAR_1} is wandering around and\nsearching for something.", "{STR_VAR_1} se toulá a něco hledá."));
static const u8 sCuriousMsg05[] = _(LOCALIZED_TEXT("{STR_VAR_1} is sniffing at you.", "{STR_VAR_1} tě očichává."));
static const u8 sCuriousMsg06[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems to be a little\nhesitant…", "{STR_VAR_1} trochu váhá…"));

const struct FollowerMsgInfo gFollowerCuriousMessages[] = {
    {sCuriousMsg00, EventScript_FollowerLookAround},
    {sCuriousMsg01, EventScript_FollowerPokingPlayer},
    {sCuriousMsg02}, {sCuriousMsg03},
    {sCuriousMsg04, EventScript_FollowerLookAround},
    {sCuriousMsg05}, {sCuriousMsg06},
};

// Unconditional music messages
static const u8 sMusicMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} is showing off its\nagility!", "{STR_VAR_1} předvádí svou hbitost!"));
static const u8 sMusicMsg01[] = _(LOCALIZED_TEXT("{STR_VAR_1} is moving around\nhappily!", "{STR_VAR_1} se vesele pohybuje!"));
static const u8 sMusicMsg02[] = _(LOCALIZED_TEXT("Woah! {STR_VAR_1} suddenly started\ndancing in happiness!", "Jé! {STR_VAR_1} náhle tančí radostí!"));
static const u8 sMusicMsg03[] = _(LOCALIZED_TEXT("{STR_VAR_1} is steadily keeping up\nwith you!", "{STR_VAR_1} s tebou drží krok!"));
static const u8 sMusicMsg04[] = _(LOCALIZED_TEXT("{STR_VAR_1} seems to want to play\nwith you.", "{STR_VAR_1} si s tebou chce asi hrát."));
static const u8 sMusicMsg05[] = _(LOCALIZED_TEXT("{STR_VAR_1} is happy skipping about.", "{STR_VAR_1} vesele poskakuje."));
static const u8 sMusicMsg06[] = _(LOCALIZED_TEXT("{STR_VAR_1} is singing and humming.", "{STR_VAR_1} zpívá a brouká si."));
static const u8 sMusicMsg07[] = _(LOCALIZED_TEXT("{STR_VAR_1} is nipping at your feet!", "{STR_VAR_1} tě okusuje do nohou!"));
static const u8 sMusicMsg08[] = _(LOCALIZED_TEXT("{STR_VAR_1} turns around and looks\nat you.", "{STR_VAR_1} se otáčí a dívá se na\ntebe."));
static const u8 sMusicMsg09[] = _(LOCALIZED_TEXT("{STR_VAR_1} is working hard to show\noff its mighty power!", "{STR_VAR_1} se snaží předvést svou\nohromnou sílu!"));
static const u8 sMusicMsg10[] = _(LOCALIZED_TEXT("Whoa! {STR_VAR_1} suddenly danced in\nhappiness!", "Jé! {STR_VAR_1} náhle zatančil\nradostí!"));
static const u8 sMusicMsg11[] = _(LOCALIZED_TEXT("{STR_VAR_1} is cheerful!", "{STR_VAR_1} je veselý!"));
static const u8 sMusicMsg12[] = _(LOCALIZED_TEXT("{STR_VAR_1} is jumping around in a\ncarefree way!", "{STR_VAR_1} bezstarostně poskakuje!"));
static const u8 sMusicMsg13[] = _(LOCALIZED_TEXT("Your POKéMON seems to be smelling a\nnostalgically familiar scent…", "Tvůj Pokémon asi cítí vůni, která mu\nněco připomíná…"));
// Conditional music messages, index 14
static const u8 sMusicMsg14[] = _(LOCALIZED_TEXT("{STR_VAR_1} is very happy about the\nrain.", "{STR_VAR_1} má velkou radost z deště."));

const struct FollowerMsgInfo gFollowerMusicMessages[] = {
    {sMusicMsg00, EventScript_FollowerLookAround},
    {sMusicMsg01},
    {sMusicMsg02, EventScript_FollowerDance},
    {sMusicMsg03},
    {sMusicMsg04, EventScript_FollowerHopping},
    {sMusicMsg05, EventScript_FollowerHopping},
    {sMusicMsg06}, {sMusicMsg07}, {sMusicMsg08}, {sMusicMsg09},
    {sMusicMsg10, EventScript_FollowerDance},
    {sMusicMsg11},
    {sMusicMsg12, EventScript_FollowerHopping},
    {sMusicMsg13, EventScript_FollowerNostalgia},
    {sMusicMsg14}
};


static const u8 sPoisonedMsg00[] = _(LOCALIZED_TEXT("{STR_VAR_1} is shivering with the\neffects of being poisoned.", "{STR_VAR_1} se třese následkem\notravy."));

const struct FollowerMsgInfo gFollowerPoisonedMessages[] = {
    {sPoisonedMsg00, EventScript_FollowerIsShivering},
};
