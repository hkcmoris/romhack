#include "global.h"
#include "event_data.h"
#include "fieldmap.h"
#include "virelia_first_gym.h"
#include "virelia_quarry_outdoor.h"
#include "constants/virelia_first_gym.h"
#include "constants/virelia_quarry_outdoor.h"

void VireliaQuarry_Transition(void)
{
    VireliaFirstGym_BeginChapter();
    VireliaFirstGym_ResetQuarryPuzzle();
}

#if !IS_FRLG
static void SetQuarryTile(u32 x, u32 y, u16 tile, bool32 blocked)
{
    MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET,
                             0x3000 | (blocked ? 0x400 : 0) | tile);
}

static void SetControl(u32 x, u32 y, u16 firstTile)
{
    for (u32 row = 0; row < 2; row++)
        for (u32 column = 0; column < 2; column++)
            SetQuarryTile(x + column, y + row, firstTile + row * 2 + column, TRUE);
}
#endif

// ON_LOAD runs after Continue restores mapView. Rewrite complete machinery and
// both cells of each gate, so stale cached art cannot close a completed route.
void VireliaQuarry_RefreshMap(void)
{
#if !IS_FRLG
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    bool32 powered = FlagGet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
    u16 gate = powered ? VQ_GROUND : VQ_GATE;
    if (map == MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS)
    {
        bool32 pumping = powered || VarGet(VAR_VIRELIA_QUARRY_SWITCH) == 1;
        SetControl(22, 12, pumping ? VQ_PUMP_CONTROL_ON_0_0 : VQ_PUMP_CONTROL_OFF_0_0);
        SetControl(10, 12, powered ? VQ_ELECTRICAL_FEED_ON_0_0 : VQ_ELECTRICAL_FEED_OFF_0_0);
        SetQuarryTile(20, 22, gate, !powered);
        SetQuarryTile(21, 22, gate, !powered);
    }
    else if (map == MAP_VIRELIA_SLATEFORD_QUARRY_GALLERY)
    {
        SetQuarryTile(26, 8, gate, !powered);
        SetQuarryTile(27, 8, gate, !powered);
        SetQuarryTile(27, 24, gate, !powered);
        SetQuarryTile(27, 25, gate, !powered);
    }
#endif
}
