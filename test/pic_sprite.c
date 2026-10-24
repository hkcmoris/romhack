#include "global.h"
#include "data.h"
#include "malloc.h"
#include "palette.h"
#include "pokemon.h"
#include "sprite.h"
#include "trainer_pokemon_sprites.h"
#include "test/test.h"

#define PIC_FAILURE 0xFFFF
#define TEST_PIC_COUNT 8
#define TEST_PALETTE_TAG 0xF100
#define OBJ_PALETTE_COUNT 16

extern u32 gOamMatrixAllocBitmap;

struct HeapUsage
{
    u32 blocks;
    u32 bytes;
};

static struct HeapUsage GetHeapUsage(void)
{
    const struct MemBlock *head = HeapHead();
    const struct MemBlock *block = head;
    struct HeapUsage usage = {0};

    do
    {
        if (block->allocated)
        {
            usage.blocks++;
            usage.bytes += block->size;
        }
        block = block->next;
    } while (block != head);
    return usage;
}

static void ExpectSameHeapUsage(struct HeapUsage expected)
{
    struct HeapUsage actual = GetHeapUsage();
    EXPECT_EQ(actual.blocks, expected.blocks);
    EXPECT_EQ(actual.bytes, expected.bytes);
}

static void InitPicTest(void)
{
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetAllPicSprites();
}

static u16 CreateTestPic(bool32 affine, u16 paletteTag)
{
    if (affine)
        return CreateMonPicSprite_Affine(SPECIES_POOCHYENA, FALSE, 0, MON_PIC_AFFINE_FRONT, 40, 40, 3, paletteTag);
    return CreateMonPicSprite(SPECIES_POOCHYENA, FALSE, 0, TRUE, 40, 40, 3, paletteTag);
}

static void ExpectNoActiveSprites(void)
{
    for (u32 i = 0; i < MAX_SPRITES; i++)
        EXPECT(!gSprites[i].inUse);
}

#if P_VIRELIA_STARTERS
TEST("Pic sprites: all Virelia starters create and release front and back pictures")
{
    const enum Species starters[] = { SPECIES_POOCHYENA_VIRELIA, SPECIES_NATU_VIRELIA, SPECIES_EKANS_VIRELIA };
    enum Species species = SPECIES_NONE;
    bool32 affine = FALSE, front = FALSE, tagged = FALSE, shiny = FALSE;

    for (u32 i = 0; i < ARRAY_COUNT(starters); i++)
        for (u32 a = 0; a < 2; a++)
            for (u32 f = 0; f < 2; f++)
                for (u32 p = 0; p < 2; p++)
                    for (u32 s = 0; s < 2; s++)
                        PARAMETRIZE_LABEL("species:%d affine:%d front:%d tagged:%d shiny:%d", starters[i], a, f, p, s)
                        { species = starters[i]; affine = a; front = f; tagged = p; shiny = s; }

    InitPicTest();
    // Force reused heap bytes to be nonzero, as on a real overworld with
    // previous allocations. Every SpriteFrameImage field must be initialized.
    u32 poisonBytes = max(MON_PIC_SIZE, TRAINER_PIC_SIZE) * max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES)
                    + sizeof(struct SpriteFrameImage) * max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES)
                    + sizeof(struct MemBlock);
    void *poison = AllocUnchecked(poisonBytes);
    EXPECT_NE(poison, NULL);
    memset(poison, 0xA5, poisonBytes);
    Free(poison);
    struct HeapUsage before = GetHeapUsage();
    u16 tag = tagged ? TEST_PALETTE_TAG : TAG_NONE;
    u16 spriteId;
    if (affine)
        spriteId = CreateMonPicSprite_Affine(species, shiny, 0x12345678, front ? MON_PIC_AFFINE_FRONT : MON_PIC_AFFINE_BACK, 40, 40, 3, tag);
    else
        spriteId = CreateMonPicSprite(species, shiny, 0x12345678, front, 40, 40, 3, tag);
    EXPECT_LT(spriteId, MAX_SPRITES);
    EXPECT(gSprites[spriteId].inUse);
    EXPECT_NE(gSprites[spriteId].images, NULL);
    EXPECT_EQ(gSprites[spriteId].images[0].size, MON_PIC_SIZE);
    EXPECT_NE(gSprites[spriteId].images[0].data, NULL);
    EXPECT_EQ(gSprites[spriteId].images[1].size, MON_PIC_SIZE);
    EXPECT_EQ((const u8 *)gSprites[spriteId].images[1].data - (const u8 *)gSprites[spriteId].images[0].data, MON_PIC_SIZE);
    for (u32 j = 0; j < (affine ? MAX_MON_PIC_FRAMES : max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES)); j++)
        EXPECT_EQ(gSprites[spriteId].images[j].relativeFrames, FALSE);
    if (tagged)
    {
        EXPECT_LT(IndexOfSpritePaletteTag(tag), OBJ_PALETTE_COUNT);
        EXPECT_EQ((u32)gSprites[spriteId].oam.paletteNum, IndexOfSpritePaletteTag(tag));
        // The caller's palette tag is independent of the species identifier.
        EXPECT_EQ(IndexOfSpritePaletteTag(species), 0xFF);
    }
    else
        EXPECT_EQ((u32)gSprites[spriteId].oam.paletteNum, 3);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(spriteId), 0);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(spriteId), PIC_FAILURE);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG), 0xFF);
    ExpectNoActiveSprites();
    EXPECT_EQ(gOamMatrixAllocBitmap, 0);
    ExpectSameHeapUsage(before);
}
#endif

TEST("Pic sprites: picture slot exhaustion preserves all eight live pictures and supports retry")
{
    bool32 affine = FALSE;
    PARAMETRIZE { affine = FALSE; }
    PARAMETRIZE { affine = TRUE; }
    u16 ids[TEST_PIC_COUNT];
    const struct SpriteFrameImage *images[TEST_PIC_COUNT];

    InitPicTest();
    struct HeapUsage before = GetHeapUsage();
    for (u32 i = 0; i < TEST_PIC_COUNT; i++)
    {
        ids[i] = CreateTestPic(affine, TEST_PALETTE_TAG + i);
        EXPECT_LT(ids[i], MAX_SPRITES);
        images[i] = gSprites[ids[i]].images;
    }
    struct HeapUsage full = GetHeapUsage();
    EXPECT_EQ(CreateTestPic(affine, TEST_PALETTE_TAG + TEST_PIC_COUNT), PIC_FAILURE);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG + TEST_PIC_COUNT), 0xFF);
    ExpectSameHeapUsage(full);
    for (u32 i = 0; i < TEST_PIC_COUNT; i++)
    {
        EXPECT(gSprites[ids[i]].inUse);
        EXPECT_EQ(gSprites[ids[i]].images, images[i]);
    }
    EXPECT_EQ(FreeAndDestroyMonPicSprite(ids[3]), 0);
    ids[3] = CreateTestPic(affine, TEST_PALETTE_TAG + 3);
    EXPECT_LT(ids[3], MAX_SPRITES);
    for (u32 i = 0; i < TEST_PIC_COUNT; i++)
        EXPECT_EQ(FreeAndDestroyMonPicSprite(ids[i]), 0);
    ExpectNoActiveSprites();
    ExpectSameHeapUsage(before);
}

TEST("Pic sprites: full sprite array leaves the MAX_SPRITES sentinel and resources untouched")
{
    bool32 affine = FALSE, borrowed = FALSE;
    PARAMETRIZE { affine = FALSE; borrowed = FALSE; }
    PARAMETRIZE { affine = TRUE; borrowed = FALSE; }
    PARAMETRIZE { affine = FALSE; borrowed = TRUE; }
    PARAMETRIZE { affine = TRUE; borrowed = TRUE; }

    InitPicTest();
    struct Sprite *original = Alloc(sizeof(gSprites));
    for (u32 i = 0; i < MAX_SPRITES; i++)
        EXPECT_EQ(CreateSprite(&gDummySpriteTemplate, i, i, 0), i);
    if (borrowed)
        EXPECT_LT(AllocSpritePalette(TEST_PALETTE_TAG), OBJ_PALETTE_COUNT);
    // The extra array entry is a sentinel, not a usable sprite. The old
    // failure path changed its paletteNum after CreateSprite returned 64.
    gSprites[MAX_SPRITES].oam.paletteNum = 11;
    memcpy(original, gSprites, sizeof(gSprites));
    struct HeapUsage before = GetHeapUsage();
    EXPECT_EQ(CreateTestPic(affine, TAG_NONE), PIC_FAILURE);
    EXPECT_EQ(memcmp(original, gSprites, sizeof(gSprites)), 0);
    ExpectSameHeapUsage(before);
    EXPECT_EQ(CreateTestPic(affine, TEST_PALETTE_TAG), PIC_FAILURE);
    EXPECT_EQ(memcmp(original, gSprites, sizeof(gSprites)), 0);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG) != 0xFF, borrowed);
    ExpectSameHeapUsage(before);
    for (u32 i = 0; i < MAX_SPRITES; i++)
        DestroySprite(&gSprites[i]);
    Free(original);
    FreeAllSpritePalettes();
    u16 spriteId = CreateTestPic(affine, TEST_PALETTE_TAG);
    EXPECT_LT(spriteId, MAX_SPRITES);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(spriteId), 0);
}

TEST("Pic sprites: exhausted OBJ tiles release heap and the newly allocated palette")
{
    bool32 affine = FALSE;
    PARAMETRIZE { affine = FALSE; }
    PARAMETRIZE { affine = TRUE; }

    InitPicTest();
    // Exhaust the allocator through its public API. A reserved count equal
    // to the array capacity is not a supported state: its search starts by
    // reading that tile before checking the upper bound.
    EXPECT_EQ(AllocSpriteTiles(TOTAL_OBJ_TILE_COUNT), 0);
    EXPECT(!CanAllocSpriteTiles(MON_PIC_SIZE / TILE_SIZE_4BPP));
    struct HeapUsage before = GetHeapUsage();
    EXPECT_EQ(CreateTestPic(affine, TEST_PALETTE_TAG), PIC_FAILURE);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG), 0xFF);
    ExpectNoActiveSprites();
    ExpectSameHeapUsage(before);
    EXPECT_EQ(AllocSpriteTiles(0), 0);
    EXPECT(CanAllocSpriteTiles(MON_PIC_SIZE / TILE_SIZE_4BPP));
    u16 spriteId = CreateTestPic(affine, TEST_PALETTE_TAG);
    EXPECT_LT(spriteId, MAX_SPRITES);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(spriteId), 0);
    ExpectSameHeapUsage(before);
}

TEST("Pic sprites: exhausted palette slots release heap and never create palette 15 by truncating 0xFF")
{
    bool32 affine = FALSE;
    PARAMETRIZE { affine = FALSE; }
    PARAMETRIZE { affine = TRUE; }

    InitPicTest();
    struct HeapUsage before = GetHeapUsage();
    for (u32 i = 0; i < OBJ_PALETTE_COUNT; i++)
        EXPECT_EQ(AllocSpritePalette(TEST_PALETTE_TAG + 1 + i), i);
    EXPECT_EQ(CreateTestPic(affine, TEST_PALETTE_TAG), PIC_FAILURE);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG), 0xFF);
    for (u32 i = 0; i < OBJ_PALETTE_COUNT; i++)
        EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG + 1 + i), i);
    ExpectNoActiveSprites();
    ExpectSameHeapUsage(before);
    FreeAllSpritePalettes();
    u16 spriteId = CreateTestPic(affine, TEST_PALETTE_TAG);
    EXPECT_LT(spriteId, MAX_SPRITES);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(spriteId), 0);
    ExpectSameHeapUsage(before);
}

TEST("Pic sprites: first and second heap allocation failures are recoverable without leaks")
{
    bool32 affine = FALSE, failImages = FALSE;
    PARAMETRIZE { affine = FALSE; failImages = FALSE; }
    PARAMETRIZE { affine = TRUE; failImages = FALSE; }
    PARAMETRIZE { affine = FALSE; failImages = TRUE; }
    PARAMETRIZE { affine = TRUE; failImages = TRUE; }

    InitPicTest();
    // The runner owns its configuration/state blocks. Leave its allocations
    // intact and consume the only free tail block, leaving either no memory
    // or exactly the frame buffer (and none for the SpriteFrameImage array).
    const struct MemBlock *head = HeapHead();
    const struct MemBlock *block = head;
    u32 reserveBytes = 0, freeBlocks = 0;
    do
    {
        if (!block->allocated)
        {
            reserveBytes = block->size;
            freeBlocks++;
        }
        block = block->next;
    } while (block != head);
    EXPECT_EQ(freeBlocks, 1);
    struct HeapUsage initial = GetHeapUsage();
    u32 frameBytes = affine ? MON_PIC_SIZE * MAX_MON_PIC_FRAMES
                            : max(MON_PIC_SIZE, TRAINER_PIC_SIZE) * max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES);
    if (failImages)
        reserveBytes -= frameBytes + sizeof(struct MemBlock);
    void *reservation = AllocUnchecked(reserveBytes);
    EXPECT_NE(reservation, NULL);
    struct HeapUsage before = GetHeapUsage();
    EXPECT_EQ(CreateTestPic(affine, TEST_PALETTE_TAG), PIC_FAILURE);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG), 0xFF);
    ExpectNoActiveSprites();
    ExpectSameHeapUsage(before);
    Free(reservation);
    ExpectSameHeapUsage(initial);
    u16 spriteId = CreateTestPic(affine, TEST_PALETTE_TAG);
    EXPECT_LT(spriteId, MAX_SPRITES);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(spriteId), 0);
    ExpectSameHeapUsage(initial);
}

TEST("Pic sprites: ownership follows the last picture sharing a newly loaded palette tag")
{
    InitPicTest();
    struct HeapUsage before = GetHeapUsage();
    u16 first = CreateTestPic(FALSE, TEST_PALETTE_TAG);
    u16 second = CreateTestPic(FALSE, TEST_PALETTE_TAG);
    EXPECT_LT(first, MAX_SPRITES);
    EXPECT_LT(second, MAX_SPRITES);
    EXPECT_NE(first, second);
    u32 paletteSlot = IndexOfSpritePaletteTag(TEST_PALETTE_TAG);
    EXPECT_LT(paletteSlot, OBJ_PALETTE_COUNT);
    EXPECT_EQ((u32)gSprites[second].oam.paletteNum, paletteSlot);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(first), 0);
    EXPECT(gSprites[second].inUse);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG), paletteSlot);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(second), 0);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG), 0xFF);
    ExpectSameHeapUsage(before);
}

TEST("Pic sprites: borrowed and explicitly retained palettes remain caller-owned")
{
    bool32 borrowed = FALSE;
    PARAMETRIZE { borrowed = FALSE; }
    PARAMETRIZE { borrowed = TRUE; }

    InitPicTest();
    struct HeapUsage before = GetHeapUsage();
    if (borrowed)
        EXPECT_LT(AllocSpritePalette(TEST_PALETTE_TAG), OBJ_PALETTE_COUNT);
    u16 spriteId = CreateTestPic(FALSE, TEST_PALETTE_TAG);
    EXPECT_LT(spriteId, MAX_SPRITES);
    u32 paletteSlot = IndexOfSpritePaletteTag(TEST_PALETTE_TAG);
    EXPECT_LT(paletteSlot, OBJ_PALETTE_COUNT);
    EXPECT_EQ(borrowed ? FreeAndDestroyMonPicSprite(spriteId) : FreeAndDestroyMonPicSpriteNoPalette(spriteId), 0);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG), paletteSlot);
    FreeSpritePaletteByTag(TEST_PALETTE_TAG);
    ExpectSameHeapUsage(before);
}

TEST("Pic sprites: invalid palette slots and destruction IDs cannot modify unrelated sprites")
{
    bool32 affine = FALSE;
    PARAMETRIZE { affine = FALSE; }
    PARAMETRIZE { affine = TRUE; }

    InitPicTest();
    struct HeapUsage before = GetHeapUsage();
    u16 unrelated = CreateSprite(&gDummySpriteTemplate, 7, 9, 0);
    EXPECT_LT(unrelated, MAX_SPRITES);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(unrelated), PIC_FAILURE);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(MAX_SPRITES), PIC_FAILURE);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(PIC_FAILURE), PIC_FAILURE);
    EXPECT(gSprites[unrelated].inUse);
    EXPECT_EQ(gSprites[unrelated].x, 7);
    EXPECT_EQ(gSprites[unrelated].y, 9);
    for (u32 slot = OBJ_PALETTE_COUNT; slot <= 0xFF; slot += 0xFF - OBJ_PALETTE_COUNT)
    {
        u16 spriteId;
        if (affine)
            spriteId = CreateMonPicSprite_Affine(SPECIES_POOCHYENA, FALSE, 0, MON_PIC_AFFINE_FRONT, 40, 40, slot, TAG_NONE);
        else
            spriteId = CreateMonPicSprite(SPECIES_POOCHYENA, FALSE, 0, TRUE, 40, 40, slot, TAG_NONE);
        EXPECT_EQ(spriteId, PIC_FAILURE);
    }
    EXPECT(gSprites[unrelated].inUse);
    DestroySprite(&gSprites[unrelated]);
    ExpectSameHeapUsage(before);
}

TEST("Pic sprites: exhausted affine matrices fail without borrowing or freeing matrix zero")
{
    InitPicTest();
    struct HeapUsage before = GetHeapUsage();
    for (u32 i = 0; i < 32; i++)
        EXPECT_EQ(AllocOamMatrix(), i);
    EXPECT_EQ(AllocOamMatrix(), 0xFF);
    EXPECT_EQ(gOamMatrixAllocBitmap, 0xFFFFFFFF);
    EXPECT_EQ(CreateTestPic(TRUE, TEST_PALETTE_TAG), PIC_FAILURE);
    EXPECT_EQ(gOamMatrixAllocBitmap, 0xFFFFFFFF);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG), 0xFF);
    ExpectNoActiveSprites();
    ExpectSameHeapUsage(before);

    // The non-affine option must remain usable even when all matrices belong
    // to other sprites; it must not release any matrix during destruction.
    u16 spriteId = CreateMonPicSprite_Affine(SPECIES_POOCHYENA, FALSE, 0, F_MON_PIC_NO_AFFINE | MON_PIC_AFFINE_FRONT, 40, 40, 3, TEST_PALETTE_TAG);
    EXPECT_LT(spriteId, MAX_SPRITES);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(spriteId), 0);
    EXPECT_EQ(gOamMatrixAllocBitmap, 0xFFFFFFFF);
    ExpectSameHeapUsage(before);
    for (u32 i = 0; i < 32; i++)
        FreeOamMatrix(i);
    EXPECT_EQ(gOamMatrixAllocBitmap, 0);
    spriteId = CreateTestPic(TRUE, TEST_PALETTE_TAG);
    EXPECT_LT(spriteId, MAX_SPRITES);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(spriteId), 0);
    EXPECT_EQ(gOamMatrixAllocBitmap, 0);
    ExpectSameHeapUsage(before);
}

#if P_VIRELIA_STARTERS
TEST("Pic sprites: SMOL scratch OOM after both picture allocations releases every resource")
{
    bool32 affine = FALSE;
    PARAMETRIZE { affine = FALSE; }
    PARAMETRIZE { affine = TRUE; }

    InitPicTest();
    const struct MemBlock *head = HeapHead();
    const struct MemBlock *block = head;
    u32 reserveBytes = 0, freeBlocks = 0;
    do
    {
        if (!block->allocated)
        {
            reserveBytes = block->size;
            freeBlocks++;
        }
        block = block->next;
    } while (block != head);
    EXPECT_EQ(freeBlocks, 1);
    struct HeapUsage initial = GetHeapUsage();
    u32 frameBytes = affine ? MON_PIC_SIZE * MAX_MON_PIC_FRAMES
                            : max(MON_PIC_SIZE, TRAINER_PIC_SIZE) * max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES);
    u32 imageBytes = sizeof(struct SpriteFrameImage) * (affine ? MAX_MON_PIC_FRAMES : max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES));
    // Reserve enough for the two picture buffers and their intervening header,
    // leaving no third block for Poochyena's SMOL decompression scratch space.
    reserveBytes -= frameBytes + imageBytes + 2 * sizeof(struct MemBlock);
    void *reservation = AllocUnchecked(reserveBytes);
    EXPECT_NE(reservation, NULL);
    struct HeapUsage before = GetHeapUsage();
    u16 spriteId;
    if (affine)
        spriteId = CreateMonPicSprite_Affine(SPECIES_POOCHYENA_VIRELIA, FALSE, 0, MON_PIC_AFFINE_FRONT, 40, 40, 3, TEST_PALETTE_TAG);
    else
        spriteId = CreateMonPicSprite(SPECIES_POOCHYENA_VIRELIA, FALSE, 0, TRUE, 40, 40, 3, TEST_PALETTE_TAG);
    EXPECT_EQ(spriteId, PIC_FAILURE);
    EXPECT_EQ(IndexOfSpritePaletteTag(TEST_PALETTE_TAG), 0xFF);
    EXPECT_EQ(gOamMatrixAllocBitmap, 0);
    ExpectNoActiveSprites();
    ExpectSameHeapUsage(before);
    Free(reservation);
    ExpectSameHeapUsage(initial);
    if (affine)
        spriteId = CreateMonPicSprite_Affine(SPECIES_POOCHYENA_VIRELIA, FALSE, 0, MON_PIC_AFFINE_FRONT, 40, 40, 3, TEST_PALETTE_TAG);
    else
        spriteId = CreateMonPicSprite(SPECIES_POOCHYENA_VIRELIA, FALSE, 0, TRUE, 40, 40, 3, TEST_PALETTE_TAG);
    EXPECT_LT(spriteId, MAX_SPRITES);
    EXPECT_EQ(FreeAndDestroyMonPicSprite(spriteId), 0);
    EXPECT_EQ(gOamMatrixAllocBitmap, 0);
    ExpectSameHeapUsage(initial);
}
#endif
