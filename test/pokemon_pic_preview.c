#include "global.h"
#include "bg.h"
#include "dma3.h"
#include "field_effect.h"
#include "field_weather.h"
#include "malloc.h"
#include "menu.h"
#include "script.h"
#include "script_menu.h"
#include "sprite.h"
#include "task.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "test/test.h"

#if P_VIRELIA_STARTERS

extern bool8 ScrCmd_hidemonpic(struct ScriptContext *ctx);

static const struct BgTemplate sPreviewBg = {.bg = 0, .mapBaseIndex = 31};
static const struct WindowTemplate sNoWindows[] = { DUMMY_WIN_TEMPLATE };
static EWRAM_DATA u16 sPreviewTilemap[32 * 32];

static void InitPreviewTest(void)
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

static void FinishPreviewTest(void)
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

static u32 CountPreviewTasks(void)
{
    u32 count = 0;
    for (u32 i = 0; i < NUM_TASKS; i++)
        count += gTasks[i].isActive != FALSE;
    return count;
}

static u32 CountPreviewSprites(void)
{
    u32 count = 0;
    for (u32 i = 0; i < MAX_SPRITES; i++)
        count += gSprites[i].inUse != FALSE;
    return count;
}

static u32 CountPreviewWindows(void)
{
    u32 count = 0;
    for (u32 i = 0; i < WINDOWS_MAX; i++)
        count += gWindows[i].window.bg != 0xFF;
    return count;
}

static u32 AllocatedPreviewBytes(void)
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

static void ExpectFailedPreviewDoesNotWait(void)
{
    struct ScriptContext ctx;
    InitScriptContext(&ctx, NULL, NULL);
    EXPECT_EQ(ScriptMenu_HidePokemonPic(), NULL);
    // Test the actual script command: no task means no native wait is installed.
    EXPECT_EQ(ScrCmd_hidemonpic(&ctx), FALSE);
    EXPECT_EQ(ctx.nativePtr, NULL);
}

static void ClosePreview(bool32 immediate)
{
    bool8 (*isClosed)(void);
    if (!immediate)
        RunTasks();
    isClosed = ScriptMenu_HidePokemonPic();
    EXPECT_NE(isClosed, NULL);
    EXPECT(!isClosed());
    // Duplicate hide calls must not skip the sprite or the window cleanup.
    EXPECT_EQ(ScriptMenu_HidePokemonPic(), isClosed);
    RunTasks();
    EXPECT(!isClosed());
    EXPECT_EQ(ScriptMenu_HidePokemonPic(), isClosed);
    RunTasks();
    EXPECT(isClosed());
    EXPECT_EQ(CountPreviewTasks(), 0);
    EXPECT_EQ(CountPreviewSprites(), 0);
    EXPECT_EQ(CountPreviewWindows(), 0);
    ClearDma3Requests();
    ClearSpriteCopyRequests();
}

TEST("Pokemon picture preview closes repeatedly for every Virelia starter without resource leaks")
{
    enum Species species;
    bool32 immediate;
    PARAMETRIZE { species = SPECIES_POOCHYENA_VIRELIA; immediate = FALSE; }
    PARAMETRIZE { species = SPECIES_NATU_VIRELIA; immediate = FALSE; }
    PARAMETRIZE { species = SPECIES_EKANS_VIRELIA; immediate = FALSE; }
    PARAMETRIZE { species = SPECIES_POOCHYENA_VIRELIA; immediate = TRUE; }
    PARAMETRIZE { species = SPECIES_NATU_VIRELIA; immediate = TRUE; }
    PARAMETRIZE { species = SPECIES_EKANS_VIRELIA; immediate = TRUE; }

    InitPreviewTest();
    u32 bytes = AllocatedPreviewBytes();
    for (u32 i = 0; i < 20; i++)
    {
        EXPECT(ScriptMenu_ShowPokemonPic(species, 2, 2));
        EXPECT_EQ(CountPreviewTasks(), 1);
        EXPECT_EQ(CountPreviewSprites(), 1);
        EXPECT_EQ(CountPreviewWindows(), 1);
        // A second show cannot replace or leak the first picture.
        EXPECT(!ScriptMenu_ShowPokemonPic(species, 2, 2));
        ClosePreview(immediate);
        EXPECT_EQ(IndexOfSpritePaletteTag(species), 0xFF);
        EXPECT_EQ(AllocatedPreviewBytes(), bytes);
    }
    ExpectFailedPreviewDoesNotWait();
    FinishPreviewTest();
}

TEST("Pokemon picture preview leaves a full task table unchanged")
{
    struct Task original[NUM_TASKS];
    InitPreviewTest();
    for (u32 i = 0; i < NUM_TASKS; i++)
    {
        EXPECT_EQ(CreateTask(TaskDummy, i), i);
        gTasks[i].data[0] = 0x1234;
    }
    memcpy(original, gTasks, sizeof(original));
    u32 bytes = AllocatedPreviewBytes();
    EXPECT(!ScriptMenu_ShowPokemonPic(SPECIES_POOCHYENA_VIRELIA, 2, 2));
    EXPECT_EQ(memcmp(original, gTasks, sizeof(original)), 0);
    EXPECT_EQ(CountPreviewSprites(), 0);
    EXPECT_EQ(CountPreviewWindows(), 0);
    EXPECT_EQ(AllocatedPreviewBytes(), bytes);
    ExpectFailedPreviewDoesNotWait();
    FinishPreviewTest();
}

TEST("Pokemon picture preview rolls back its task when all window slots are occupied")
{
    struct WindowTemplate window = {.bg = 0, .width = 1, .height = 1};
    InitPreviewTest();
    for (u32 i = 0; i < WINDOWS_MAX; i++)
        EXPECT_EQ(AddWindow(&window), i);
    u32 bytes = AllocatedPreviewBytes();
    EXPECT(!ScriptMenu_ShowPokemonPic(SPECIES_POOCHYENA_VIRELIA, 2, 2));
    EXPECT_EQ(CountPreviewTasks(), 0);
    EXPECT_EQ(CountPreviewSprites(), 0);
    EXPECT_EQ(CountPreviewWindows(), WINDOWS_MAX);
    EXPECT_EQ(AllocatedPreviewBytes(), bytes);
    ExpectFailedPreviewDoesNotWait();
    FinishPreviewTest();
}

TEST("Pokemon picture preview and field move handle sprite exhaustion before indexing or preserving a palette")
{
    enum Species species;
    PARAMETRIZE { species = SPECIES_POOCHYENA_VIRELIA; }
    PARAMETRIZE { species = SPECIES_NATU_VIRELIA; }
    PARAMETRIZE { species = SPECIES_EKANS_VIRELIA; }
    struct Sprite sentinel;
    u8 weatherBefore[32];
    InitPreviewTest();
    for (u32 i = 0; i < MAX_SPRITES; i++)
        EXPECT_EQ(CreateSprite(&gDummySpriteTemplate, 0, 0, 0), i);
    // The sprite engine has an extra sentinel slot; failure must not edit it.
    sentinel = gSprites[MAX_SPRITES];
    const u8 *weather = SetPaletteColorMapType(31, COLOR_MAP_CONTRAST);
    memcpy(weatherBefore, weather, sizeof(weatherBefore));
    u32 bytes = AllocatedPreviewBytes();
    EXPECT(!ScriptMenu_ShowPokemonPic(species, 2, 2));
    EXPECT_EQ(CreateMonSprite_FieldMove(species, FALSE, 0x8000, 40, 40, 0), MAX_SPRITES);
    EXPECT_EQ(memcmp(&sentinel, &gSprites[MAX_SPRITES], sizeof(sentinel)), 0);
    EXPECT_EQ(memcmp(weatherBefore, weather, sizeof(weatherBefore)), 0);
    EXPECT_EQ(IndexOfSpritePaletteTag(species), 0xFF);
    EXPECT_EQ(CountPreviewTasks(), 0);
    EXPECT_EQ(CountPreviewWindows(), 0);
    EXPECT_EQ(CountPreviewSprites(), MAX_SPRITES);
    EXPECT_EQ(AllocatedPreviewBytes(), bytes);
    ExpectFailedPreviewDoesNotWait();
    FinishPreviewTest();
}

TEST("Pokemon picture preview cleans up and can retry after sprite palette exhaustion")
{
    enum Species species;
    PARAMETRIZE { species = SPECIES_POOCHYENA_VIRELIA; }
    PARAMETRIZE { species = SPECIES_NATU_VIRELIA; }
    PARAMETRIZE { species = SPECIES_EKANS_VIRELIA; }
    InitPreviewTest();
    for (u32 i = 0; i < 16; i++)
        EXPECT_EQ(AllocSpritePalette(0x7000 + i), i);
    u32 bytes = AllocatedPreviewBytes();
    EXPECT(!ScriptMenu_ShowPokemonPic(species, 2, 2));
    EXPECT_EQ(CountPreviewTasks(), 0);
    EXPECT_EQ(CountPreviewSprites(), 0);
    EXPECT_EQ(CountPreviewWindows(), 0);
    EXPECT_EQ(AllocatedPreviewBytes(), bytes);
    for (u32 i = 0; i < 16; i++)
        EXPECT_EQ(IndexOfSpritePaletteTag(0x7000 + i), i);
    ExpectFailedPreviewDoesNotWait();
    FreeAllSpritePalettes();
    EXPECT(ScriptMenu_ShowPokemonPic(species, 2, 2));
    ClosePreview(FALSE);
    EXPECT_EQ(AllocatedPreviewBytes(), bytes);
    FinishPreviewTest();
}

TEST("Pokemon picture preview handles heap exhaustion at the window, background and picture allocation stages")
{
    u32 allocationStage;
    PARAMETRIZE { allocationStage = 0; } // Window pixel buffer.
    PARAMETRIZE { allocationStage = 1; } // Picture after the window succeeds.
    PARAMETRIZE { allocationStage = 2; } // Window-owned background tilemap.
    PARAMETRIZE { allocationStage = 3; } // Pixels after the background succeeds.
    PARAMETRIZE { allocationStage = 4; } // Picture after owned background and pixels.
    void *pressure[64];
    void *windowBuffer = NULL;
    u32 count = 0;
    InitPreviewTest();
    u32 initialBytes = AllocatedPreviewBytes();
    if (allocationStage >= 2)
    {
        UnsetBgTilemapBuffer(0);
        EXPECT(InitWindows(sNoWindows));
    }
    if (allocationStage == 1 || allocationStage == 3)
        windowBuffer = Alloc(8 * 8 * 32);
    else if (allocationStage == 4)
        windowBuffer = Alloc(2 * 8 * 8 * 32 + sizeof(struct MemBlock));
    // Occupy every free heap block using the real allocator. A reserved 8x8
    // buffer lets the second case allocate its window but not its picture.
    while (TRUE)
    {
        const struct MemBlock *head = HeapHead();
        const struct MemBlock *block = head;
        do
        {
            if (!block->allocated)
                break;
            block = block->next;
        } while (block != head);
        if (block->allocated)
            break;
        EXPECT_LT(count, ARRAY_COUNT(pressure));
        pressure[count] = AllocUnchecked(block->size);
        EXPECT_NE(pressure[count], NULL);
        count++;
    }
    if (windowBuffer != NULL)
        Free(windowBuffer);
    u32 pressureBytes = AllocatedPreviewBytes();
    EXPECT(!ScriptMenu_ShowPokemonPic(SPECIES_POOCHYENA_VIRELIA, 2, 2));
    EXPECT_EQ(CountPreviewTasks(), 0);
    EXPECT_EQ(CountPreviewSprites(), 0);
    EXPECT_EQ(CountPreviewWindows(), 0);
    EXPECT_EQ(IndexOfSpritePaletteTag(SPECIES_POOCHYENA_VIRELIA), 0xFF);
    EXPECT_EQ(AllocatedPreviewBytes(), pressureBytes);
    if (allocationStage >= 2)
    {
        EXPECT_EQ(GetBgTilemapBuffer(0), NULL);
        EXPECT_EQ(gWindowBgTilemapBuffers[0], NULL);
    }
    ExpectFailedPreviewDoesNotWait();
    for (u32 i = 0; i < count; i++)
        Free(pressure[i]);
    EXPECT_EQ(AllocatedPreviewBytes(), initialBytes);
    EXPECT(ScriptMenu_ShowPokemonPic(SPECIES_POOCHYENA_VIRELIA, 2, 2));
    ClosePreview(FALSE);
    EXPECT_EQ(AllocatedPreviewBytes(), initialBytes);
    FinishPreviewTest();
}

#endif // P_VIRELIA_STARTERS
