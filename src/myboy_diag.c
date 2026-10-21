#include "global.h"
#include "myboy_diag.h"

#if MYBOY_DIAGNOSTIC_MODE
#include "main.h"
#include "malloc.h"
#include "script.h"
#include "sprite.h"
#include "task.h"
#include "battle.h"
#include "battle_transition.h"

// The linker reserves this object in EWRAM, outside save blocks and the heap.
// No guessed free address, persistent section, FLASH write or allocation.
EWRAM_DATA volatile struct MyBoyDiagBuffer gMyBoyDiagBuffer = {0};

_Static_assert(sizeof(struct MyBoyDiagEvent) == 24, "Diagnostic event ABI");
_Static_assert(sizeof(struct MyBoyDiagBuffer) == 1624, "Diagnostic buffer ABI");

bool32 MyBoyDiag_IsReadable(uintptr_t address, u32 size)
{
    uintptr_t end = address + size;

    if (end < address)
        return FALSE;
    return (address >= 0x02000000 && end <= 0x02040000)
        || (address >= 0x03000000 && end <= 0x03008000)
        || (address >= 0x08000000 && end <= 0x0E000000);
}

bool32 MyBoyDiag_IsExecutable(uintptr_t address)
{
    // ARM targets need word alignment; Thumb targets retain their state bit.
    if (address == 0 || (!(address & 1) && (address & 3)))
        return FALSE;
    return MyBoyDiag_IsReadable(address & ~1u, 4);
}

void MyBoyDiag_Init(void)
{
    // Plain stores: do not introduce a DMA, BIOS call or heap dependency.
    volatile u32 *word = (volatile u32 *)&gMyBoyDiagBuffer;
    u32 i;

    for (i = 0; i < sizeof(gMyBoyDiagBuffer) / sizeof(u32); i++)
        word[i] = 0;
    gMyBoyDiagBuffer.version = 1;
    gMyBoyDiagBuffer.mode = MYBOY_DIAGNOSTIC_MODE;
    gMyBoyDiagBuffer.capacity = MYBOY_DIAG_CAPACITY;
    gMyBoyDiagBuffer.eventSize = sizeof(struct MyBoyDiagEvent);
    gMyBoyDiagBuffer.nextSequence = 1;
    gMyBoyDiagBuffer.lowestSp = 0xFFFFFFFF;
    gMyBoyDiagBuffer.magic0 = MYBOY_DIAG_MAGIC0;
    gMyBoyDiagBuffer.magic1 = MYBOY_DIAG_MAGIC1;
    MyBoyDiag_Record(MYBOY_EVENT_BOOT, MYBOY_DIAGNOSTIC_MODE, 0);
}

void MyBoyDiag_Record(u32 kind, u32 arg0, u32 arg1)
{
    u32 sequence = gMyBoyDiagBuffer.nextSequence;
    volatile struct MyBoyDiagEvent *event;
    uintptr_t sp;

    // Logging is exclusively on the main thread, never in an IRQ handler.
    // Index with a constant mask even if the header is corrupted.
    if (sequence == 0)
        sequence = 1;
    event = &gMyBoyDiagBuffer.events[(sequence - 1) & (MYBOY_DIAG_CAPACITY - 1)];
    event->sequence = 0;
    event->frame = gMain.vblankCounter1;
    event->kind = kind;
    event->arg0 = arg0;
    event->arg1 = arg1;
    event->map = 0xFFFFFFFF;
    if (kind == MYBOY_EVENT_PREVIEW_SKIPPED)
        gMyBoyDiagBuffer.lastPreview = arg0;
    if (MyBoyDiag_IsReadable((uintptr_t)gSaveBlock1Ptr, sizeof(*gSaveBlock1Ptr)))
        event->map = ((u8)gSaveBlock1Ptr->location.mapGroup << 8) | (u8)gSaveBlock1Ptr->location.mapNum;
    asm volatile ("mov %0, sp" : "=r" (sp));
    gMyBoyDiagBuffer.lastSp = sp;
    if (sp < gMyBoyDiagBuffer.lowestSp)
        gMyBoyDiagBuffer.lowestSp = sp;
    event->sequence = sequence;
    gMyBoyDiagBuffer.nextSequence = sequence + 1;
    if (gMyBoyDiagBuffer.count < MYBOY_DIAG_CAPACITY)
        gMyBoyDiagBuffer.count++;
}

bool32 MyBoyDiag_IsScriptValid(const struct ScriptContext *ctx)
{
    uintptr_t table = (uintptr_t)ctx->cmdTable;
    uintptr_t end = (uintptr_t)ctx->cmdTableEnd;

    return ctx->mode <= 2 && ctx->stackDepth < SCRIPT_STACK_SIZE
        && MyBoyDiag_IsReadable((uintptr_t)ctx->scriptPtr, 1)
        && !(table & 3) && end > table && !(end & 3)
        && end - table <= 256 * sizeof(ScrCmdFunc)
        && MyBoyDiag_IsReadable(table, end - table);
}

void MyBoyDiag_CheckCallback(uintptr_t address, u32 origin)
{
    if (!MyBoyDiag_IsExecutable(address))
        MyBoyDiag_Fail(MYBOY_FAULT_CALLBACK, address, origin);
}

void MyBoyDiag_Script(const struct ScriptContext *ctx, bool32 effects)
{
    u32 opcode;

    if (!MyBoyDiag_IsScriptValid(ctx))
        MyBoyDiag_Fail(MYBOY_FAULT_SCRIPT, (uintptr_t)ctx->scriptPtr, (uintptr_t)ctx->cmdTable);
    opcode = *ctx->scriptPtr;
    gMyBoyDiagBuffer.lastScript = (uintptr_t)ctx->scriptPtr;
    gMyBoyDiagBuffer.lastOpcode = opcode;
    if (opcode >= ctx->cmdTableEnd - ctx->cmdTable)
        MyBoyDiag_Fail(MYBOY_FAULT_OPCODE, opcode, (uintptr_t)ctx->scriptPtr);
    MyBoyDiag_CheckCallback((uintptr_t)ctx->cmdTable[opcode], 1);
    // High bit distinguishes effect look-ahead from a command being executed.
    MyBoyDiag_Record(MYBOY_EVENT_OPCODE, (uintptr_t)ctx->scriptPtr, opcode | (effects ? 0x80000000 : 0));
}

void MyBoyDiag_Resources(void)
{
    u32 i, tasks = 0, sprites = 0, freeBytes = 0;
    const struct MemBlock *head = HeapHead();
    const struct MemBlock *block = head;
    bool32 heapComplete = FALSE;

    for (i = 0; i < NUM_TASKS; i++)
        tasks += !gTasks[i].isActive;
    for (i = 0; i < MAX_SPRITES; i++)
        sprites += !gSprites[i].inUse;
    // Bounded even if the allocator links were corrupted. Resource snapshots
    // run only at preview boundaries, never for every opcode or interrupt.
    for (i = 0; i < 64; i++)
    {
        if ((uintptr_t)block < (uintptr_t)gHeap
         || (uintptr_t)block > (uintptr_t)gHeap + HEAP_SIZE - sizeof(*block)
         || ((uintptr_t)block & 3) || block->magic != MALLOC_SYSTEM_ID)
            break;
        if (!block->allocated)
            freeBytes += block->size;
        block = block->next;
        if (block == head)
        {
            heapComplete = TRUE;
            break;
        }
    }
    if (!heapComplete)
        freeBytes = 0xFFFFFFFF; // Invalid/too long: do not claim a free total.
    MyBoyDiag_Record(MYBOY_EVENT_RESOURCE, tasks | (sprites << 16), freeBytes);
    MyBoyDiag_Record(MYBOY_EVENT_RESOURCE, 0x80000000 | CountFreePaletteSlots(), 0);
}

void MyBoyDiag_Battle(u32 transition, u32 state)
{
    gMyBoyDiagBuffer.lastBattleFlags = gBattleTypeFlags;
    MyBoyDiag_Record(state == 0xFFFFFFFF ? MYBOY_EVENT_BATTLE_CREATE : MYBOY_EVENT_BATTLE_TASK,
                    transition | (state << 16), gBattleTypeFlags);
    MyBoyDiag_Record(MYBOY_EVENT_CALLBACK, (uintptr_t)gMain.savedCallback, 4);
    // Do not reject an empty enemy party here: ordinary trainer teams may be
    // populated later by CB2_InitBattle. The saved return callback must exist.
    if (!MyBoyDiag_IsExecutable((uintptr_t)gMain.savedCallback))
        MyBoyDiag_Fail(MYBOY_FAULT_BATTLE, (uintptr_t)gMain.savedCallback, gBattleTypeFlags);
    if (transition >= B_TRANSITION_COUNT || (state != 0xFFFFFFFF && state > 1))
        MyBoyDiag_Fail(MYBOY_FAULT_BATTLE, transition, state);
}

void MyBoyDiag_Fail(u32 code, u32 arg0, u32 arg1)
{
    // Preserve the first detection and stop. Never repair a pointer or continue
    // a corrupt battle and label that a compatibility fix.
    if (gMyBoyDiagBuffer.faultCode == 0)
    {
        gMyBoyDiagBuffer.faultArg0 = arg0;
        gMyBoyDiagBuffer.faultArg1 = arg1;
        gMyBoyDiagBuffer.faultIme = REG_IME;
        gMyBoyDiagBuffer.faultIe = REG_IE;
        gMyBoyDiagBuffer.faultIf = REG_IF;
        gMyBoyDiagBuffer.faultWaitcnt = REG_WAITCNT;
        gMyBoyDiagBuffer.faultCode = code;
        MyBoyDiag_Record(MYBOY_EVENT_FAULT, code, arg0);
    }
    // Keep interrupts enabled as they were so emulator menus/save states can
    // still be used when supported. This is intentionally a diagnostic stop.
    for (;;)
        asm volatile ("" ::: "memory");
}
#endif
