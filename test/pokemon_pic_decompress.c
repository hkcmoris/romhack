#include "global.h"
#include "data.h"
#include "decompress.h"
#include "malloc.h"
#include "pokemon.h"
#include "test/test.h"

#if P_VIRELIA_STARTERS

// Expected FNV-1a hashes come from independently decoding the committed PNG
// tile pixels, not from the implementation under test. Both 64x64 front
// frames must survive the same loader used by ordinary battle graphics.
struct PicExpectations
{
    enum Species species;
    u32 front[2];
    u32 back;
    u32 normalPalette;
    u32 shinyPalette;
};

static const struct PicExpectations sPics[] =
{
    { SPECIES_POOCHYENA_VIRELIA, { 0x2BD992BF, 0x5C02B914 }, 0xB41476F3, 0xF316420D, 0xF7946526 },
    { SPECIES_SPINARAK_VIRELIA,  { 0xA42C0840, 0xADF47381 }, 0x4C62322A, 0x5277B941, 0xD458E707 },
    { SPECIES_EKANS_VIRELIA,     { 0xDD599030, 0x160EA5AE }, 0xEB4E42D5, 0x848FE9C0, 0x7E6EB0C4 },
};

static EWRAM_DATA u32 sGuardedPic[MON_PIC_SIZE * MAX_MON_PIC_FRAMES / sizeof(u32) + 2] = {0};

static u32 HashBytes(const void *data, u32 size)
{
    const u8 *bytes = data;
    u32 hash = 2166136261u;
    for (u32 i = 0; i < size; i++)
        hash = (hash ^ bytes[i]) * 16777619u;
    return hash;
}

static u32 AllocatedPicBytes(void)
{
    const struct MemBlock *head = HeapHead();
    const struct MemBlock *block = head;
    u32 size = 0;
    do
    {
        if (block->allocated)
            size += block->size;
        block = block->next;
    } while (block != head);
    return size;
}

static void ExpectUntouched(const void *data, u32 size)
{
    const u8 *bytes = data;
    for (u32 i = 0; i < size; i++)
        EXPECT_EQ(bytes[i], 0xA5);
}

TEST("Pokemon picture decompression preserves both Virelia front frames and battle backs within bounds")
{
    u32 choice = 0;
    bool32 front = FALSE, checked = FALSE;
    for (u32 i = 0; i < ARRAY_COUNT(sPics); i++)
        for (u32 f = 0; f < 2; f++)
            for (u32 c = 0; c < 2; c++)
                PARAMETRIZE_LABEL("species:%d front:%d checked:%d", sPics[i].species, f, c)
                { choice = i; front = f; checked = c; }

    const struct PicExpectations *pic = &sPics[choice];
    u32 capacity = MON_PIC_SIZE * (front ? MAX_MON_PIC_FRAMES : 1);
    u8 *pixels = (u8 *)&sGuardedPic[1];
    u32 heapBytes = AllocatedPicBytes();
    CpuFill32(0xA5A5A5A5, sGuardedPic, sizeof(sGuardedPic));
    if (checked)
        EXPECT(TryLoadSpecialPokePic(pixels, pic->species, 0x8000, front, capacity));
    else
        LoadSpecialPokePic(pixels, pic->species, 0x8000, front);
    EXPECT_EQ(sGuardedPic[0], 0xA5A5A5A5);
    ExpectUntouched(pixels + capacity, sizeof(sGuardedPic) - sizeof(u32) - capacity);
    EXPECT_EQ(HashBytes(pixels, MON_PIC_SIZE), front ? pic->front[0] : pic->back);
    if (front)
        EXPECT_EQ(HashBytes(pixels + MON_PIC_SIZE, MON_PIC_SIZE), pic->front[1]);
    EXPECT_EQ(AllocatedPicBytes(), heapBytes);
}

TEST("Pokemon picture palettes preserve all Virelia normal and shiny colors")
{
    u32 choice = 0;
    bool32 shiny = FALSE;
    for (u32 i = 0; i < ARRAY_COUNT(sPics); i++)
        for (u32 s = 0; s < 2; s++)
            PARAMETRIZE_LABEL("species:%d shiny:%d", sPics[i].species, s)
            { choice = i; shiny = s; }

    const struct PicExpectations *pic = &sPics[choice];
    const u16 *palette = GetMonSpritePalFromSpeciesAndPersonality(pic->species, shiny, 0x8000);
    EXPECT_NE(palette, NULL);
    EXPECT_EQ(HashBytes(palette, PLTT_SIZE_4BPP), shiny ? pic->shinyPalette : pic->normalPalette);
    for (u32 i = 0; i < PLTT_SIZE_4BPP / sizeof(u16); i++)
        EXPECT_EQ(palette[i] & 0x8000, 0);
}

TEST("Pokemon picture checked decompression rejects short and unaligned destinations before writing")
{
    u32 choice = 0;
    bool32 front = FALSE;
    for (u32 i = 0; i < ARRAY_COUNT(sPics); i++)
        for (u32 f = 0; f < 2; f++)
            PARAMETRIZE_LABEL("species:%d front:%d", sPics[i].species, f)
            { choice = i; front = f; }

    const struct PicExpectations *pic = &sPics[choice];
    u32 capacity = MON_PIC_SIZE * (front ? MAX_MON_PIC_FRAMES : 1);
    u8 *pixels = (u8 *)&sGuardedPic[1];
    u32 heapBytes = AllocatedPicBytes();
    CpuFill32(0xA5A5A5A5, sGuardedPic, sizeof(sGuardedPic));
    EXPECT(!TryLoadSpecialPokePic(pixels, pic->species, 0x8000, front, capacity - 1));
    EXPECT(!TryLoadSpecialPokePic(pixels + 1, pic->species, 0x8000, front, capacity));
    EXPECT(!TryLoadSpecialPokePic(NULL, pic->species, 0x8000, front, capacity));
    EXPECT(!TryLoadSpecialPokePic(pixels, pic->species, 0x8000, front, 0));
    ExpectUntouched(sGuardedPic, sizeof(sGuardedPic));
    EXPECT_EQ(AllocatedPicBytes(), heapBytes);
}

TEST("Pokemon picture SMOL scratch exhaustion returns failure after frame and image buffers exist")
{
    void *pressure[64];
    u32 count = 0;
    u32 initialBytes = AllocatedPicBytes();
    // Match the ordinary picture sprite's successful first two allocations.
    u32 frameBytes = max(MON_PIC_SIZE, TRAINER_PIC_SIZE) * max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES);
    void *frames = AllocUnchecked(frameBytes);
    void *images = AllocUnchecked(sizeof(struct SpriteFrameImage) * max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES));
    EXPECT_NE(frames, NULL);
    EXPECT_NE(images, NULL);
    CpuFill32(0xA5A5A5A5, frames, frameBytes);

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
    u32 pressureBytes = AllocatedPicBytes();
    EXPECT(!TryLoadSpecialPokePic(frames, SPECIES_POOCHYENA_VIRELIA, 0x8000, TRUE, frameBytes));
    ExpectUntouched(frames, frameBytes);
    EXPECT_EQ(AllocatedPicBytes(), pressureBytes);
    for (u32 i = 0; i < count; i++)
        Free(pressure[i]);
    Free(images);
    Free(frames);
    EXPECT_EQ(AllocatedPicBytes(), initialBytes);

    // Failure leaves no stale decoder state or allocation that prevents retry.
    EXPECT(TryLoadSpecialPokePic(&sGuardedPic[1], SPECIES_POOCHYENA_VIRELIA, 0x8000, TRUE, MON_PIC_SIZE * MAX_MON_PIC_FRAMES));
    EXPECT_EQ(HashBytes(&sGuardedPic[1], MON_PIC_SIZE), sPics[0].front[0]);
    EXPECT_EQ(AllocatedPicBytes(), initialBytes);
}

#endif // P_VIRELIA_STARTERS
