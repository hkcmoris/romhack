#include "global.h"
#include "event_data.h"
#include "fieldmap.h"
#include "virelia_sanctuary.h"
#include "constants/maps.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_sanctuary.h"

#if !IS_FRLG
static void SetConnectionGate(s32 x, s32 y, u16 shrub, u16 floor, bool32 opened)
{
    MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET,
                          opened ? floor : shrub | MAPGRID_IMPASSABLE);
}
#endif

void VireliaSanctuary_UpdateConnectionGates(void)
{
#if !IS_FRLG
    bool32 hanaComplete = FlagGet(FLAG_VIRELIA_SIDEQUEST_REWARDED);
    bool32 shortcutOpen = hanaComplete && FlagGet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN);

    // Seamless transitions restore the old camera view after the new map's
    // load script. Update the neighbor's copied tiles as well, so that view
    // cannot restore a closed gate over a correctly opened destination gate.
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_VIRELIA_ROUTE01)
     && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_VIRELIA_ROUTE01))
    {
        for (u32 y = 16; y <= 17; y++)
            SetConnectionGate(-2, y, y == 16 ? 0xE : 0xC6, 0xD, hanaComplete);
        for (u32 y = 8; y <= 9; y++)
            SetConnectionGate(-6, y, y == 8 ? 0xE : 0xC6, y == 8 ? 0x1 : 0x1CF, shortcutOpen);
    }
    else if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_VIRELIA_WHISPERWOOD_SANCTUARY)
          && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_VIRELIA_WHISPERWOOD_SANCTUARY))
    {
        for (u32 y = 10; y <= 11; y++)
            SetConnectionGate(23, y, y == 10 ? 0xE : 0x200, 0xD, hanaComplete);
        for (u32 y = 2; y <= 3; y++)
            SetConnectionGate(23, y, y == 2 ? 0xE : 0x200, 0x1, shortcutOpen);
    }
#endif
}
