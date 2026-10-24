#include "global.h"
#include "agb_flash.h"
#include "battle.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "new_game.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "region_map.h"
#include "regions.h"
#include "save.h"
#include "string_util.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "wild_encounter.h"
#include "constants/map_types.h"

// Virelia uses Emerald map metadata. FRLG intentionally omits these headers.
#if !IS_FRLG

static const u16 sVireliaMaps[] =
{
    MAP_VIRELIA_MAPLEBROOK_TOWN,
    MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY,
    MAP_VIRELIA_ROUTE01,
    MAP_VIRELIA_SLATEFORD_CITY,
    MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE,
    MAP_VIRELIA_SLATEFORD_POKEMON_CENTER,
    MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE,
    MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS,
    MAP_VIRELIA_SLATEFORD_GYM,
    MAP_VIRELIA_SLATEFORD_QUARRY_GALLERY,
    MAP_VIRELIA_WHISPERWOOD_SANCTUARY,
};

static const struct MapHeader *WorldHeader(u16 map)
{
    return Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(map), MAP_NUM(map));
}

static void EnterWorldMap(u16 map, s8 x, s8 y)
{
    SetWarpDestination(MAP_GROUP(map), MAP_NUM(map), WARP_ID_NONE, x, y);
    WarpIntoMap();
}

TEST("Virelia world headers use distinct appended groups and native layouts")
{
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaMaps); i++)
        PARAMETRIZE { index = i; }

    u16 map = sVireliaMaps[index];
    const struct MapHeader *header = WorldHeader(map);
    EXPECT_NE(header, NULL);
    EXPECT_NE(header->mapLayout, NULL);
    EXPECT_NE(header->events, NULL);
    EXPECT_NE(header->mapScripts, NULL);
    EXPECT_EQ(header->mapLayout, GetMapLayout(header->mapLayoutId));
    EXPECT_GT(header->mapLayout->width, 0);
    EXPECT_GT(header->mapLayout->height, 0);
    EXPECT_LE((header->mapLayout->width + MAP_OFFSET_W)
              * (header->mapLayout->height + MAP_OFFSET_H), MAX_MAP_DATA_SIZE);
    EXPECT_EQ(header->mapLayout->isFrlg, FALSE);
    if (map == MAP_VIRELIA_WHISPERWOOD_SANCTUARY)
    {
        EXPECT_EQ(MAP_GROUP(map), MAP_GROUP(MAP_VIRELIA_ROUTE01));
        EXPECT_EQ(MAP_NUM(map), 3);
        EXPECT_EQ(header->mapType, MAP_TYPE_ROUTE);
    }
    EXPECT_GT(header->regionMapSectionId, MAPSEC_TRAINER_HILL);
    EXPECT_LT(header->regionMapSectionId, METLOC_SPECIAL_EGG);
    EXPECT_LT(header->regionMapSectionId, MAPSEC_NONE);
    EXPECT_EQ(GetRegionForSectionId(header->regionMapSectionId), REGION_HOENN);
    EXPECT_LT(MAP_GROUP(map), MAP_GROUP(MAP_DYNAMIC));
    EXPECT_NE(MAP_GROUP(map), MAP_GROUP(MAP_LITTLEROOT_TOWN));
    EXPECT_NE(MAP_GROUP(map), MAP_GROUP(MAP_BATTLE_FRONTIER_BATTLE_TOWER_LOBBY));
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaMaps); i++)
    {
        if (i == index)
            continue;
        EXPECT_NE(map, sVireliaMaps[i]);
        EXPECT_NE(header->mapLayoutId, WorldHeader(sVireliaMaps[i])->mapLayoutId);
    }
    EXPECT_NE(MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_TOWN),
              MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY));
    EXPECT_EQ(MAP_NUM(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER), 2);
    EXPECT_EQ(MAP_NUM(MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE), 3);
    EXPECT_EQ(MAP_NUM(MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS), 4);
    EXPECT_EQ(MAP_NUM(MAP_VIRELIA_SLATEFORD_GYM), 5);
    EXPECT_EQ(MAP_NUM(MAP_VIRELIA_SLATEFORD_QUARRY_GALLERY), 6);
}

TEST("Virelia object graphics include hidden story actors in Emerald")
{
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaMaps); i++)
        PARAMETRIZE { index = i; }

    const struct MapEvents *events = WorldHeader(sVireliaMaps[index])->events;
    for (u32 i = 0; i < events->objectEventCount; i++)
    {
        const struct ObjectEventGraphicsInfo *info =
            GetObjectEventGraphicsInfo(events->objectEvents[i].graphicsId);
        EXPECT_NE(info, NULL);
        EXPECT_NE(info->images, NULL);
        EXPECT_NE(info->anims, NULL);
        EXPECT_NE(info->oam, NULL);
    }
}

TEST("Virelia building warps resolve through the native warp loader in both directions")
{
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaMaps); i++)
        if (WorldHeader(sVireliaMaps[i])->events->warpCount != 0)
            PARAMETRIZE { index = i; }

    const struct MapHeader *source = WorldHeader(sVireliaMaps[index]);
    EXPECT_GT(source->events->warpCount, 0);
    for (u32 i = 0; i < source->events->warpCount; i++)
    {
        const struct WarpEvent *warp = &source->events->warps[i];
        const struct MapHeader *target = Overworld_GetMapHeaderByGroupAndId(warp->mapGroup, warp->mapNum);
        EXPECT_LT(warp->warpId, target->events->warpCount);
        const struct WarpEvent *arrival = &target->events->warps[warp->warpId];
        EXPECT_EQ(arrival->mapGroup, MAP_GROUP(sVireliaMaps[index]));
        EXPECT_EQ(arrival->mapNum, MAP_NUM(sVireliaMaps[index]));
        EnterWorldMap(sVireliaMaps[index], warp->x, warp->y);
        SetWarpDestinationToMapWarp(warp->mapGroup, warp->mapNum, warp->warpId);
        WarpIntoMap();
        EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, warp->mapGroup);
        EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, warp->mapNum);
        EXPECT_EQ(gSaveBlock1Ptr->mapLayoutId, target->mapLayoutId);
        EXPECT_EQ(gSaveBlock1Ptr->pos.x, arrival->x);
        EXPECT_EQ(gSaveBlock1Ptr->pos.y, arrival->y);
    }
}

TEST("Virelia overworld connections have reciprocal passable seams")
{
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaMaps); i++)
        if (WorldHeader(sVireliaMaps[i])->connections != NULL)
            PARAMETRIZE { index = i; }

    u16 map = sVireliaMaps[index];
    const struct MapHeader *source = WorldHeader(map);
    EXPECT_NE(source->connections, NULL);
    EXPECT_GT(source->connections->count, 0);
    for (u32 i = 0; i < source->connections->count; i++)
    {
        const struct MapConnection *connection = &source->connections->connections[i];
        const struct MapHeader *target = GetMapHeaderFromConnection(connection);
        // Neighbor metatiles are cached and drawn with the current map's bank.
        EXPECT_EQ(source->mapLayout->primaryTileset, target->mapLayout->primaryTileset);
        EXPECT_EQ(source->mapLayout->secondaryTileset, target->mapLayout->secondaryTileset);
        bool32 reciprocal = FALSE, passable = FALSE;
        EXPECT_NE(target->connections, NULL);
        bool32 vertical = connection->direction == CONNECTION_NORTH || connection->direction == CONNECTION_SOUTH;
        s32 opposite = vertical
            ? (connection->direction == CONNECTION_NORTH ? CONNECTION_SOUTH : CONNECTION_NORTH)
            : (connection->direction == CONNECTION_WEST ? CONNECTION_EAST : CONNECTION_WEST);
        for (u32 j = 0; j < target->connections->count; j++)
        {
            const struct MapConnection *back = &target->connections->connections[j];
            if (back->mapGroup == MAP_GROUP(map) && back->mapNum == MAP_NUM(map)
                && back->direction == opposite
                && back->offset == -connection->offset)
                reciprocal = TRUE;
        }
        EXPECT(reciprocal);
        s32 limit = vertical ? source->mapLayout->width : source->mapLayout->height;
        for (s32 coordinate = 0; coordinate < limit; coordinate++)
        {
            s32 sourceX, sourceY, targetX, targetY;
            if (vertical)
            {
                sourceX = coordinate;
                targetX = coordinate - connection->offset;
                sourceY = connection->direction == CONNECTION_NORTH ? 0 : source->mapLayout->height - 1;
                targetY = connection->direction == CONNECTION_NORTH ? target->mapLayout->height - 1 : 0;
            }
            else
            {
                sourceX = connection->direction == CONNECTION_WEST ? 0 : source->mapLayout->width - 1;
                targetX = connection->direction == CONNECTION_WEST ? target->mapLayout->width - 1 : 0;
                sourceY = coordinate;
                targetY = coordinate - connection->offset;
            }
            if (targetX < 0 || targetX >= target->mapLayout->width
                || targetY < 0 || targetY >= target->mapLayout->height)
                continue;
            u16 a = source->mapLayout->map[sourceY * source->mapLayout->width + sourceX];
            u16 b = target->mapLayout->map[targetY * target->mapLayout->width + targetX];
            if (UNPACK_COLLISION(a) == 0 && UNPACK_COLLISION(b) == 0
                && UNPACK_ELEVATION(a) == UNPACK_ELEVATION(b))
                passable = TRUE;
        }
        EXPECT(passable);
    }
}

TEST("Virelia Route 01 has native grass behavior and generates a valid wild encounter")
{
    EnterWorldMap(MAP_VIRELIA_ROUTE01, 14, 1);
    InitMap();
    u32 grassCount = 0;
    for (s32 y = 0; y < gMapHeader.mapLayout->height; y++)
        for (s32 x = 0; x < gMapHeader.mapLayout->width; x++)
            if (MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET) == 0
                && MetatileBehavior_IsTallGrass(MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET)))
                grassCount++;
    EXPECT_GT(grassCount, 0);

    u16 headerId = GetCurrentMapWildMonHeaderId();
    EXPECT_NE(headerId, HEADER_NONE);
    const struct WildPokemonInfo *info =
        gWildMonHeaders[headerId].encounterTypes[GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND)].landMonsInfo;
    EXPECT_NE(info, NULL);
    EXPECT_GT(info->encounterRate, 0);
    EXPECT_NE(info->wildPokemon, NULL);
    for (u32 i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
    {
        EXPECT(IsSpeciesEnabled(info->wildPokemon[i].species));
        EXPECT_GT(info->wildPokemon[i].minLevel, 0);
        EXPECT_GE(info->wildPokemon[i].maxLevel, info->wildPokemon[i].minLevel);
        EXPECT_LE(info->wildPokemon[i].maxLevel, MAX_LEVEL);
    }
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_TREECKO, 10, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    EXPECT(TryGenerateWildMon(info, WILD_AREA_LAND, 0));
    enum Species species = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES);
    u32 level = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL);
    bool32 matchesTable = FALSE;
    for (u32 i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
        if (species == info->wildPokemon[i].species
            && level >= info->wildPokemon[i].minLevel && level <= info->wildPokemon[i].maxLevel)
            matchesTable = TRUE;
    EXPECT(matchesTable);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SANITY_IS_BAD_EGG), FALSE);
    EXPECT_GT(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HP), 0);
}

TEST("Virelia map locations and regional starter survive native flash save and load")
{
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaMaps); i++)
        PARAMETRIZE { index = i; }

    // Choose an actual walkable tile, so restoring the position also exercises
    // native map-layout lookup instead of merely retaining arbitrary bytes.
    const struct MapHeader *header = WorldHeader(sVireliaMaps[index]);
    s32 x = -1, y = -1;
    for (s32 row = 1; row < header->mapLayout->height - 1 && x < 0; row++)
        for (s32 col = 1; col < header->mapLayout->width - 1; col++)
            if (UNPACK_COLLISION(header->mapLayout->map[row * header->mapLayout->width + col]) == 0)
            {
                x = col;
                y = row;
                break;
            }
    EXPECT_GE(x, 0);
    EnterWorldMap(sVireliaMaps[index], x, y);
    InitMap();
    SetTrainerId(0x1234ABCD, gSaveBlock2Ptr->playerTrainerId);
    StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Virelia"));
    ZeroPlayerPartyMons();
#if P_VIRELIA_STARTERS
    enum Species species = SPECIES_POOCHYENA_VIRELIA;
#else
    enum Species species = SPECIES_TREECKO;
#endif
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], species, 10, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    FlagSet(FLAG_ITEM_VIRELIA_ROUTE01_POTION);

    // Flash normally uses the test runner's Timer 2 watchdog. Borrow Timer 3
    // and restore handlers/registers before an assertion may exit nonlocally.
    IntrFunc originalTimer3Handler, unusedFlashHandler;
    u16 originalIme = REG_IME;
    REG_IME = 0;
    u16 originalIe = REG_IE;
    originalTimer3Handler = gIntrTable[2];
    u16 originalTimer3Low = REG_TM3CNT_L, originalTimer3High = REG_TM3CNT_H;
    REG_TM3CNT_H = 0;
    REG_IF = INTR_FLAG_TIMER3;
    SetFlashTimerIntr(3, &gIntrTable[2]);
    REG_IME = originalIme;
    ClearSaveData();
    Save_ResetSaveCounters();
    u8 saveStatus = TrySavingData(SAVE_NORMAL), loadStatus = SAVE_STATUS_ERROR;
    if (saveStatus == SAVE_STATUS_OK)
    {
        EnterWorldMap(MAP_BATTLE_FRONTIER_BATTLE_TOWER_LOBBY, 4, 4);
        ZeroPlayerPartyMons();
        FlagClear(FLAG_ITEM_VIRELIA_ROUTE01_POTION);
        loadStatus = LoadGameSave(SAVE_NORMAL);
    }
    REG_IME = 0;
    REG_TM3CNT_H = 0;
    REG_IF = INTR_FLAG_TIMER3;
    SetFlashTimerIntr(2, &unusedFlashHandler);
    gIntrTable[2] = originalTimer3Handler;
    REG_TM3CNT_L = originalTimer3Low;
    REG_TM3CNT_H = originalTimer3High;
    REG_IE = originalIe;
    REG_IME = originalIme;

    EXPECT_EQ(saveStatus, SAVE_STATUS_OK);
    EXPECT_EQ(loadStatus, SAVE_STATUS_OK);
    EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(sVireliaMaps[index]));
    EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(sVireliaMaps[index]));
    EXPECT_EQ(gSaveBlock1Ptr->mapLayoutId, header->mapLayoutId);
    EXPECT_EQ(gSaveBlock1Ptr->pos.x, x);
    EXPECT_EQ(gSaveBlock1Ptr->pos.y, y);
    EXPECT(FlagGet(FLAG_ITEM_VIRELIA_ROUTE01_POTION));
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), species);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SANITY_IS_BAD_EGG), FALSE);
    EnterWorldMap(sVireliaMaps[index], x, y);
    InitMap();
    EXPECT_EQ(MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET), 0);

    // Hoenn and the existing developer test destination remain addressable.
    EnterWorldMap(MAP_BATTLE_FRONTIER_BATTLE_TOWER_LOBBY, 4, 4);
    EXPECT_EQ(gMapHeader.regionMapSectionId, MAPSEC_BATTLE_FRONTIER);
    EnterWorldMap(MAP_LITTLEROOT_TOWN, 10, 10);
    EXPECT_EQ(gMapHeader.regionMapSectionId, MAPSEC_LITTLEROOT_TOWN);
}

#if VIRELIA_WORLD_TEST
TEST("Virelia development gateway requires a healthy non-egg Pokemon without modifying the party")
{
    u32 state = 0;
    for (u32 i = 0; i < 6; i++)
        PARAMETRIZE { state = i; }

    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    if (state == 5)
    {
        struct Pokemon boxed;
        CreateRandomMonWithIVs(&boxed, SPECIES_TREECKO, 10, 10);
        VarSet(VAR_PC_BOX_TO_SEND_MON, 0);
        EXPECT_EQ(CopyMonToPC(&boxed), MON_GIVEN_TO_PC);
    }
    if (state != 0 && state != 5)
    {
        CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_WOBBUFFET, 10, 10);
        gPartiesCount[B_TRAINER_PLAYER] = 1;
        if (state == 1)
        {
            u32 egg = TRUE;
            SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_IS_EGG, &egg);
        }
        if (state == 2 || state == 4)
        {
            u32 hp = 0;
            SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
        }
        if (state == 4)
        {
            CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_TREECKO, 10, 10);
            gPartiesCount[B_TRAINER_PLAYER] = 2;
        }
    }
    struct Pokemon original[PARTY_SIZE];
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));
    u32 originalCount = gPartiesCount[B_TRAINER_PLAYER];
    RUN_OVERWORLD_SCRIPT(call VireliaWorldTest_EventScript_CheckEntry;);
    EXPECT_EQ(gSpecialVar_Result, state == 3 || state == 4);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], originalCount);
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], sizeof(original)), 0);
}
#endif

#endif // !IS_FRLG
