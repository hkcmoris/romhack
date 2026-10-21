#include "global.h"
#include "sprite.h"
#include "window.h"
#include "malloc.h"
#include "palette.h"
#include "decompress.h"
#include "trainer.h"
#include "trainer_pokemon_sprites.h"
#include "data.h"
#include "pokemon.h"
#include "constants/trainers.h"

#define PICS_COUNT 8

// Needs to be large enough to store either a decompressed Pokémon pic or trainer pic
#define PIC_SPRITE_SIZE max(MON_PIC_SIZE, TRAINER_PIC_SIZE)
#define MAX_PIC_FRAMES  max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES)

struct PicData
{
    u8 *frames;
    struct SpriteFrameImage *images;
    u16 paletteTag;
    u8 spriteId;
    u8 active;
    bool8 ownsPalette;
};

static EWRAM_DATA struct SpriteTemplate sCreatingSpriteTemplate = {};
static EWRAM_DATA struct PicData sSpritePics[PICS_COUNT] = {};

static const struct PicData sDummyPicData = {};

static const struct OamData sOamData_Normal =
{
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64)
};

static const struct OamData sOamData_Affine =
{
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64)
};

static void DummyPicSpriteCallback(struct Sprite *sprite)
{

}

bool16 ResetAllPicSprites(void)
{
    int i;

    for (i = 0; i < PICS_COUNT; i ++)
        sSpritePics[i] = sDummyPicData;

    return FALSE;
}

static bool16 DecompressPic(u16 picId, u32 personality, bool8 isFrontPic, u8 *dest, bool8 isTrainer, u32 capacity)
{
    if (!isTrainer)
    {
        enum Species species = picId;
        return !TryLoadSpecialPokePic(dest, species, personality, isFrontPic, capacity);
    }
    else
    {
        enum TrainerPicID trainerPicId = picId;
        if (isFrontPic)
        {
            const u32 *pic = GetTrainerFrontPicData(trainerPicId);
            if (GetDecompressedDataSize(pic) > capacity)
                return TRUE;
            DecompressDataWithHeaderWram(pic, dest);
        }
        else
        {
            const struct SpriteFrameImage *frame = GetTrainerBackPicImage(trainerPicId);
            if (frame->size * GetTrainerBackPicCoords(trainerPicId)->y_offset > capacity)
                return TRUE;
            CopyTrainerBackspriteFramesToDest(trainerPicId, dest);
        }
    }
    return FALSE;
}

// A tagged palette may already belong to another sprite. Never claim ownership
// of a borrowed tag, and use the requested tag consistently with the template.
static bool32 LoadPicPaletteByTagOrSlot(u16 species, bool8 isShiny, u32 personality, u8 paletteSlot, u16 paletteTag, bool8 isTrainer, bool8 *ownsPalette)
{
    const u16 *palette = isTrainer ? GetTrainerFrontPicPalette(species)
        : GetMonSpritePalFromSpeciesAndPersonality(species, isShiny, personality);
    *ownsPalette = FALSE;
    sCreatingSpriteTemplate.paletteTag = paletteTag;
    if (paletteTag == TAG_NONE)
    {
        if (paletteSlot >= 16)
            return FALSE;
        LoadPalette(palette, OBJ_PLTT_ID(paletteSlot), PLTT_SIZE_4BPP);
        return TRUE;
    }
    if (IndexOfSpritePaletteTag(paletteTag) != 0xFF)
        return TRUE;
    if (LoadSpritePaletteWithTag(palette, paletteTag) == 0xFF)
        return FALSE;
    *ownsPalette = TRUE;
    return TRUE;
}

static void LoadPicPaletteBySlot(u16 species, bool8 isShiny, u32 personality, u8 paletteSlot, bool8 isTrainer)
{
    if (!isTrainer)
        LoadPalette(GetMonSpritePalFromSpeciesAndPersonality(species, isShiny, personality), PLTT_ID(paletteSlot), PLTT_SIZE_4BPP);
    else
        LoadPalette(GetTrainerFrontPicPalette(species), PLTT_ID(paletteSlot), PLTT_SIZE_4BPP);
}

static void AssignSpriteAnimsTable(bool8 isTrainer)
{
    if (!isTrainer)
        sCreatingSpriteTemplate.anims = gAnims_MonPic;
    else
        sCreatingSpriteTemplate.anims = gAnims_Trainer;
}

static u16 CreatePicSprite(u16 species, bool8 isShiny, u32 personality, bool8 isFrontPic, s16 x, s16 y, u8 paletteSlot, u16 paletteTag, bool8 isTrainer)
{
    u8 i;
    u8 *framePics;
    struct SpriteFrameImage *images;
    int j;
    u8 spriteId;
    bool8 ownsPalette;

    if (paletteTag == TAG_NONE && paletteSlot >= 16)
        return 0xFFFF;
    if (!isTrainer)
        species = SanitizeSpeciesId(species);

    for (i = 0; i < PICS_COUNT; i ++)
    {
        if (!sSpritePics[i].active)
            break;
    }
    if (i == PICS_COUNT)
        return 0xFFFF;

    framePics = AllocUnchecked(PIC_SPRITE_SIZE * MAX_PIC_FRAMES);
    if (!framePics)
        return 0xFFFF;

    images = AllocUnchecked(sizeof(struct SpriteFrameImage) * MAX_PIC_FRAMES);
    if (!images)
    {
        Free(framePics);
        return 0xFFFF;
    }
    if (DecompressPic(species, personality, isFrontPic, framePics, isTrainer, PIC_SPRITE_SIZE * MAX_PIC_FRAMES))
    {
        Free(images);
        Free(framePics);
        return 0xFFFF;
    }
    for (j = 0; j < MAX_PIC_FRAMES; j ++)
    {
        images[j].data = framePics + PIC_SPRITE_SIZE * j;
        images[j].size = PIC_SPRITE_SIZE;
        images[j].relativeFrames = FALSE;
    }
    sCreatingSpriteTemplate.tileTag = TAG_NONE;
    sCreatingSpriteTemplate.oam = &sOamData_Normal;
    AssignSpriteAnimsTable(isTrainer);
    sCreatingSpriteTemplate.images = images;
    sCreatingSpriteTemplate.affineAnims = gDummySpriteAffineAnimTable;
    sCreatingSpriteTemplate.callback = DummyPicSpriteCallback;
    if (!LoadPicPaletteByTagOrSlot(species, isShiny, personality, paletteSlot, paletteTag, isTrainer, &ownsPalette))
    {
        Free(images);
        Free(framePics);
        return 0xFFFF;
    }
    spriteId = CreateSpriteUnchecked(&sCreatingSpriteTemplate, x, y, 0);
    if (spriteId >= MAX_SPRITES)
    {
        if (ownsPalette)
            FreeSpritePaletteByTag(paletteTag);
        Free(images);
        Free(framePics);
        return 0xFFFF;
    }
    if (paletteTag == TAG_NONE)
        gSprites[spriteId].oam.paletteNum = paletteSlot;
    sSpritePics[i].frames = framePics;
    sSpritePics[i].images = images;
    sSpritePics[i].paletteTag = paletteTag;
    sSpritePics[i].spriteId = spriteId;
    sSpritePics[i].ownsPalette = ownsPalette;
    sSpritePics[i].active = TRUE;
    return spriteId;
}

u16 CreateMonPicSprite_Affine(enum Species species, bool8 isShiny, u32 personality, u8 flags, s16 x, s16 y, u8 paletteSlot, u16 paletteTag)
{
    u8 *framePics;
    struct SpriteFrameImage *images;
    int j;
    u8 i;
    u8 spriteId;
    u8 type;
    bool8 ownsPalette;
    if (paletteTag == TAG_NONE && paletteSlot >= 16)
        return 0xFFFF;
    species = SanitizeSpeciesId(species);

    for (i = 0; i < PICS_COUNT; i++)
    {
        if (!sSpritePics[i].active)
            break;
    }
    if (i == PICS_COUNT)
        return 0xFFFF;

    framePics = AllocUnchecked(MON_PIC_SIZE * MAX_MON_PIC_FRAMES);
    if (!framePics)
        return 0xFFFF;

    if (flags & F_MON_PIC_NO_AFFINE)
    {
        flags &= ~F_MON_PIC_NO_AFFINE;
        type = MON_PIC_AFFINE_NONE;
    }
    else
    {
        type = flags;
    }
    images = AllocUnchecked(sizeof(struct SpriteFrameImage) * MAX_MON_PIC_FRAMES);
    if (!images)
    {
        Free(framePics);
        return 0xFFFF;
    }
    if (DecompressPic(species, personality, flags, framePics, FALSE, MON_PIC_SIZE * MAX_MON_PIC_FRAMES))
    {
        Free(images);
        Free(framePics);
        return 0xFFFF;
    }
    for (j = 0; j < MAX_MON_PIC_FRAMES; j ++)
    {
        images[j].data = framePics + MON_PIC_SIZE * j;
        images[j].size = MON_PIC_SIZE;
        images[j].relativeFrames = FALSE;
    }
    if (type != MON_PIC_AFFINE_NONE)
    {
        u8 matrixNum = AllocOamMatrix();
        if (matrixNum == 0xFF)
        {
            Free(images);
            Free(framePics);
            return 0xFFFF;
        }
        // Creation is synchronous; release the probe before CreateSprite
        // performs the normal affine allocation.
        FreeOamMatrix(matrixNum);
    }
    sCreatingSpriteTemplate.tileTag = TAG_NONE;
    sCreatingSpriteTemplate.anims = gSpeciesInfo[species].frontAnimFrames;
    sCreatingSpriteTemplate.images = images;
    if (type == MON_PIC_AFFINE_FRONT)
    {
        sCreatingSpriteTemplate.affineAnims = gAffineAnims_BattleSpriteOpponentSide;
        sCreatingSpriteTemplate.oam = &sOamData_Affine;
    }
    else if (type == MON_PIC_AFFINE_BACK)
    {
        sCreatingSpriteTemplate.affineAnims = gAffineAnims_BattleSpritePlayerSide;
        sCreatingSpriteTemplate.oam = &sOamData_Affine;
    }
    else // MON_PIC_AFFINE_NONE
    {
        sCreatingSpriteTemplate.oam = &sOamData_Normal;
        sCreatingSpriteTemplate.affineAnims = gDummySpriteAffineAnimTable;
    }
    sCreatingSpriteTemplate.callback = DummyPicSpriteCallback;
    if (!LoadPicPaletteByTagOrSlot(species, isShiny, personality, paletteSlot, paletteTag, FALSE, &ownsPalette))
    {
        Free(images);
        Free(framePics);
        return 0xFFFF;
    }
    spriteId = CreateSpriteUnchecked(&sCreatingSpriteTemplate, x, y, 0);
    if (spriteId >= MAX_SPRITES)
    {
        if (ownsPalette)
            FreeSpritePaletteByTag(paletteTag);
        Free(images);
        Free(framePics);
        return 0xFFFF;
    }
    if (paletteTag == TAG_NONE)
        gSprites[spriteId].oam.paletteNum = paletteSlot;
    sSpritePics[i].frames = framePics;
    sSpritePics[i].images = images;
    sSpritePics[i].paletteTag = paletteTag;
    sSpritePics[i].spriteId = spriteId;
    sSpritePics[i].ownsPalette = ownsPalette;
    sSpritePics[i].active = TRUE;
    return spriteId;
}

static u16 FreeAndDestroyPicSpriteInternal(u16 spriteId, bool8 clearPalette)
{
    u32 i, j;
    bool32 liveSprite;
    u16 paletteTag;

    if (spriteId >= MAX_SPRITES)
        return 0xFFFF;
    for (i = 0; i < PICS_COUNT; i++)
        if (sSpritePics[i].active && sSpritePics[i].spriteId == spriteId)
            break;
    if (i == PICS_COUNT)
        return 0xFFFF;

    // An externally destroyed pic may have had its OAM slot reused. Dispose of
    // our orphaned buffers without destroying an unrelated replacement sprite.
    liveSprite = gSprites[spriteId].inUse && gSprites[spriteId].images == sSpritePics[i].images;
    if (liveSprite)
    {
        FreeSpriteOamMatrix(&gSprites[spriteId]);
        DestroySprite(&gSprites[spriteId]);
    }
    paletteTag = sSpritePics[i].paletteTag;
    if (clearPalette && sSpritePics[i].ownsPalette && paletteTag != TAG_NONE)
    {
        // Keep a shared tag alive until the last picture using it is released.
        for (j = 0; j < PICS_COUNT; j++)
            if (j != i && sSpritePics[j].active && sSpritePics[j].paletteTag == paletteTag)
                break;
        if (j < PICS_COUNT)
        {
            sSpritePics[j].ownsPalette = TRUE;
        }
        else
        {
            u32 paletteNum = IndexOfSpritePaletteTag(paletteTag);
            for (j = 0; j < MAX_SPRITES; j++)
                if (gSprites[j].inUse && gSprites[j].oam.paletteNum == paletteNum)
                    break;
            if (j == MAX_SPRITES)
                FreeSpritePaletteByTag(paletteTag);
        }
    }
    Free(sSpritePics[i].frames);
    Free(sSpritePics[i].images);
    sSpritePics[i] = sDummyPicData;
    return liveSprite ? 0 : 0xFFFF;
}

static u16 LoadPicSpriteInWindow(u16 species, bool8 isShiny, u32 personality, bool8 isFrontPic, u8 paletteSlot, u8 windowId, bool8 isTrainer)
{
    if (DecompressPic(species, personality, isFrontPic, (u8 *)GetWindowAttribute(windowId, WINDOW_TILE_DATA), FALSE, GetWindowAttribute(windowId, WINDOW_WIDTH) * GetWindowAttribute(windowId, WINDOW_HEIGHT) * TILE_SIZE_4BPP))
        return 0xFFFF;

    LoadPicPaletteBySlot(species, isShiny, personality, paletteSlot, isTrainer);
    return 0;
}

static u16 CreateTrainerCardSprite(u16 species, bool8 isShiny, u32 personality, bool8 isFrontPic, u16 destX, u16 destY, u8 paletteSlot, u8 windowId, bool8 isTrainer)
{
    u8 *framePics;

    framePics = AllocUnchecked(TRAINER_PIC_SIZE * MAX_TRAINER_PIC_FRAMES);
    if (framePics && !DecompressPic(species, personality, isFrontPic, framePics, isTrainer, TRAINER_PIC_SIZE * MAX_TRAINER_PIC_FRAMES))
    {
        BlitBitmapRectToWindow(windowId, framePics, 0, 0, TRAINER_PIC_WIDTH, TRAINER_PIC_HEIGHT, destX, destY, TRAINER_PIC_WIDTH, TRAINER_PIC_HEIGHT);
        LoadPicPaletteBySlot(species, isShiny, personality, paletteSlot, isTrainer);
        Free(framePics);
        return 0;
    }
    Free(framePics);
    return 0xFFFF;
}

u16 CreateMonPicSprite(enum Species species, bool8 isShiny, u32 personality, bool8 isFrontPic, s16 x, s16 y, u8 paletteSlot, u16 paletteTag)
{
    return CreatePicSprite(species, isShiny, personality, isFrontPic, x, y, paletteSlot, paletteTag, FALSE);
}

u16 FreeAndDestroyMonPicSprite(u16 spriteId)
{
    return FreeAndDestroyPicSpriteInternal(spriteId, TRUE);
}

u16 FreeAndDestroyMonPicSpriteNoPalette(u16 spriteId)
{
    return FreeAndDestroyPicSpriteInternal(spriteId, FALSE);
}

static u16 UNUSED LoadMonPicInWindow(enum Species species, bool8 isShiny, u32 personality, bool8 isFrontPic, u8 paletteSlot, u8 windowId)
{
    return LoadPicSpriteInWindow(species, isShiny, personality, isFrontPic, paletteSlot, windowId, FALSE);
}

// Unused, FRLG only
u16 CreateTrainerCardMonIconSprite(enum Species species, bool8 isShiny, u32 personality, bool8 isFrontPic, u16 destX, u16 destY, u8 paletteSlot, u8 windowId)
{
    return CreateTrainerCardSprite(species, isShiny, personality, isFrontPic, destX, destY, paletteSlot, windowId, FALSE);
}

u16 CreateTrainerPicSprite(u16 species, bool8 isFrontPic, s16 x, s16 y, u8 paletteSlot, u16 paletteTag)
{
    return CreatePicSprite(species, FALSE, 0, isFrontPic, x, y, paletteSlot, paletteTag, TRUE);
}

u16 FreeAndDestroyTrainerPicSprite(u16 spriteId)
{
    return FreeAndDestroyPicSpriteInternal(spriteId, TRUE);
}

static u16 UNUSED LoadTrainerPicInWindow(u16 species, bool8 isFrontPic, u8 paletteSlot, u8 windowId)
{
    return LoadPicSpriteInWindow(species, FALSE, 0, isFrontPic, paletteSlot, windowId, TRUE);
}

u16 CreateTrainerCardTrainerPicSprite(u16 species, bool8 isFrontPic, u16 destX, u16 destY, u8 paletteSlot, u8 windowId)
{
    return CreateTrainerCardSprite(species, FALSE, 0, isFrontPic, destX, destY, paletteSlot, windowId, TRUE);
}

u16 PlayerGenderToFrontTrainerPicId_Debug(enum Gender gender, bool8 getClass)
{
    if (getClass == TRUE)
    {
        if (gender != MALE)
            return gFacilityClassToPicIndex[FACILITY_CLASS_MAY];
        else
            return gFacilityClassToPicIndex[FACILITY_CLASS_BRENDAN];
    }
    return gender;
}

void CopyTrainerBackspriteFramesToDest(enum TrainerPicID trainerPicId, u8 *dest)
{
    const struct SpriteFrameImage *frame = GetTrainerBackPicImage(trainerPicId);
    // y_offset is repurposed to indicates how many frames does the trainer pic have.
    u32 size = (frame->size * GetTrainerBackPicCoords(trainerPicId)->y_offset);
    CpuSmartCopy16(frame->data, dest, size);
}
