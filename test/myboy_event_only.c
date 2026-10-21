#include "global.h"
#include "bg.h"
#include "dma3.h"
#include "event_data.h"
#include "field_weather.h"
#include "malloc.h"
#include "script.h"
#include "script_menu.h"
#include "sprite.h"
#include "task.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "test/test.h"

#if MYBOY_DIAGNOSTIC_MODE == 2

extern bool8 ScrCmd_showmonpic(struct ScriptContext *ctx);
extern bool8 ScrCmd_hidemonpic(struct ScriptContext *ctx);

static const struct BgTemplate sPreviewBg = {.bg = 0, .mapBaseIndex = 31};
static const struct WindowTemplate sNoWindows[] = { DUMMY_WIN_TEMPLATE };
static EWRAM_DATA u16 sPreviewTilemap[32 * 32];

static void InitPictureTest(void)
{
    ResetTasks();
    ResetSpriteData();
    ResetAllPicSprites();
    FreeAllSpritePalettes();
    ResetPreservedPalettesInWeather();
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, &sPreviewBg, 1);
    SetBgTilemapBuffer(0, sPreviewTilemap);
    EXPECT(InitWindows(sNoWindows));
    ClearScheduledBgCopiesToVram();
}

static void FinishPictureTest(void)
{
    ClearDma3Requests();
    ClearSpriteCopyRequests();
    ClearScheduledBgCopiesToVram();
    FreeAllWindowBuffers();
    UnsetBgTilemapBuffer(0);
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetPreservedPalettesInWeather();
}

static u32 AllocatedBytes(void)
{
    const struct MemBlock *head = HeapHead();
    const struct MemBlock *block = head;
    u32 bytes = 0;
    do
    {
        if (block->allocated)
            bytes += block->size;
        block = block->next;
    } while (block != head);
    return bytes;
}

TEST("My Boy event-only consumes starter picture operands without resources or native waits")
{
    const enum Species species[] = {SPECIES_SPINARAK_VIRELIA, SPECIES_EKANS_VIRELIA, SPECIES_POOCHYENA_VIRELIA};
    const struct WarpData location = gSaveBlock1Ptr->location;
    const u16 savedResult = gSpecialVar_Result;
    struct ScriptContext ctx;

    InitPictureTest();
    const u32 bytes = AllocatedBytes();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY);
    gSpecialVar_Result = 0x1234;
    for (u32 iteration = 0; iteration < 20; iteration++)
    {
        for (u32 i = 0; i < ARRAY_COUNT(species); i++)
        {
            const u8 operands[] = {species[i] & 0xFF, species[i] >> 8, 10, 3, 0xA5};
            InitScriptContext(&ctx, NULL, NULL);
            SetupBytecodeScript(&ctx, operands);
            EXPECT_EQ(ScrCmd_showmonpic(&ctx), FALSE);
            EXPECT_EQ(ctx.scriptPtr, operands + 4);
            EXPECT_EQ(*ctx.scriptPtr, 0xA5);
            EXPECT_EQ(ScriptMenu_HidePokemonPic(), NULL);
            EXPECT_EQ(ScrCmd_hidemonpic(&ctx), FALSE);
            EXPECT_EQ(ctx.nativePtr, NULL);
            EXPECT_EQ(IndexOfSpritePaletteTag(species[i]), 0xFF);
            EXPECT_EQ(AllocatedBytes(), bytes);
            EXPECT_EQ(gSpecialVar_Result, 0x1234);
            for (u32 task = 0; task < NUM_TASKS; task++)
                EXPECT(!gTasks[task].isActive);
            for (u32 sprite = 0; sprite < MAX_SPRITES; sprite++)
                EXPECT(!gSprites[sprite].inUse);
            for (u32 window = 0; window < WINDOWS_MAX; window++)
                EXPECT_EQ(gWindows[window].window.bg, 0xFF);
        }
    }
    gSpecialVar_Result = savedResult;
    gSaveBlock1Ptr->location = location;
    FinishPictureTest();
}

TEST("My Boy event-only preserves nonstarter pictures and pictures outside the laboratory")
{
    enum Species species;
    bool32 laboratory;
    PARAMETRIZE { species = SPECIES_PIKACHU; laboratory = TRUE; }
    PARAMETRIZE { species = SPECIES_EKANS_VIRELIA; laboratory = FALSE; }
    const struct WarpData location = gSaveBlock1Ptr->location;
    struct ScriptContext ctx;

    InitPictureTest();
    const u32 bytes = AllocatedBytes();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY);
    gSaveBlock1Ptr->location.mapNum = laboratory ? MAP_NUM(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY) : MAP_NUM(MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE);
    const u8 operands[] = {species & 0xFF, species >> 8, 2, 2};
    InitScriptContext(&ctx, NULL, NULL);
    SetupBytecodeScript(&ctx, operands);
    EXPECT_EQ(ScrCmd_showmonpic(&ctx), FALSE);
    EXPECT_EQ(ctx.scriptPtr, operands + sizeof(operands));
    bool8 (*isClosed)(void) = ScriptMenu_HidePokemonPic();
    EXPECT_NE(isClosed, NULL);
    EXPECT(!isClosed());
    RunTasks();
    RunTasks();
    EXPECT(isClosed());
    EXPECT_EQ(AllocatedBytes(), bytes);
    gSaveBlock1Ptr->location = location;
    FinishPictureTest();
}

#endif
