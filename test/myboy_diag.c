#include "global.h"
#include "myboy_diag.h"

#if MYBOY_DIAGNOSTIC_MODE
#include "script.h"
#include "test/test.h"

static u32 sDiagnosticCommandCalls;

static bool8 DiagnosticCommand(struct ScriptContext *ctx)
{
    sDiagnosticCommandCalls++;
    return TRUE;
}

TEST("My Boy diagnostic ring stays bounded and retains the newest committed records")
{
    u32 sequence;

    MyBoyDiag_Init();
    EXPECT_EQ(gMyBoyDiagBuffer.magic0, MYBOY_DIAG_MAGIC0);
    EXPECT_EQ(gMyBoyDiagBuffer.magic1, MYBOY_DIAG_MAGIC1);
    EXPECT_EQ(gMyBoyDiagBuffer.mode, MYBOY_DIAGNOSTIC_MODE);
    EXPECT_EQ(gMyBoyDiagBuffer.capacity, MYBOY_DIAG_CAPACITY);
    EXPECT_EQ(gMyBoyDiagBuffer.count, 1);
    EXPECT_EQ(gMyBoyDiagBuffer.faultCode, 0);
    EXPECT_EQ(gMyBoyDiagBuffer.events[0].kind, MYBOY_EVENT_BOOT);

    for (u32 i = 0; i < MYBOY_DIAG_CAPACITY + 8; i++)
        MyBoyDiag_Record(MYBOY_EVENT_RESOURCE, i, ~i);
    EXPECT_EQ(gMyBoyDiagBuffer.count, MYBOY_DIAG_CAPACITY);
    EXPECT_EQ(gMyBoyDiagBuffer.nextSequence, MYBOY_DIAG_CAPACITY + 10);
    for (sequence = 10; sequence < MYBOY_DIAG_CAPACITY + 10; sequence++)
    {
        u32 slot = (sequence - 1) & (MYBOY_DIAG_CAPACITY - 1);
        EXPECT_EQ(gMyBoyDiagBuffer.events[slot].sequence, sequence);
        EXPECT_EQ(gMyBoyDiagBuffer.events[slot].arg0, sequence - 2);
        EXPECT_EQ(gMyBoyDiagBuffer.events[slot].arg1, ~(sequence - 2));
    }
    EXPECT(gMyBoyDiagBuffer.lastSp >= 0x03000000);
    EXPECT(gMyBoyDiagBuffer.lastSp < 0x03008000);
    EXPECT(gMyBoyDiagBuffer.lowestSp <= gMyBoyDiagBuffer.lastSp);
}

TEST("My Boy diagnostic sequence wrap never commits the reserved zero sequence")
{
    MyBoyDiag_Init();
    for (u32 i = 0; i < MYBOY_DIAG_CAPACITY; i++)
        gMyBoyDiagBuffer.events[i].sequence = 0;
    gMyBoyDiagBuffer.nextSequence = 0xFFFFFFFE;
    gMyBoyDiagBuffer.count = 0;
    MyBoyDiag_Record(MYBOY_EVENT_RESOURCE, 1, 0);
    MyBoyDiag_Record(MYBOY_EVENT_RESOURCE, 2, 0);
    EXPECT_EQ(gMyBoyDiagBuffer.nextSequence, 0);
    MyBoyDiag_Record(MYBOY_EVENT_RESOURCE, 3, 0);
    MyBoyDiag_Record(MYBOY_EVENT_RESOURCE, 4, 0);
    EXPECT_EQ(gMyBoyDiagBuffer.nextSequence, 3);
    EXPECT_EQ(gMyBoyDiagBuffer.count, 4);
    EXPECT_EQ(gMyBoyDiagBuffer.events[61].sequence, 0xFFFFFFFE);
    EXPECT_EQ(gMyBoyDiagBuffer.events[62].sequence, 0xFFFFFFFF);
    EXPECT_EQ(gMyBoyDiagBuffer.events[0].sequence, 1);
    EXPECT_EQ(gMyBoyDiagBuffer.events[1].sequence, 2);
    EXPECT_EQ(gMyBoyDiagBuffer.events[63].sequence, 0);
}

TEST("My Boy diagnostic address checks reject low memory, range overflow and corrupt callbacks")
{
    static const uintptr_t ranges[][2] = {
        {0x02000000, 0x02040000}, {0x03000000, 0x03008000},
        {0x08000000, 0x0E000000},
    };

    for (u32 i = 0; i < ARRAY_COUNT(ranges); i++)
    {
        EXPECT(MyBoyDiag_IsReadable(ranges[i][0], 1));
        EXPECT(MyBoyDiag_IsReadable(ranges[i][1] - 1, 1));
        EXPECT(!MyBoyDiag_IsReadable(ranges[i][0] - 1, 1));
        EXPECT(!MyBoyDiag_IsReadable(ranges[i][1], 1));
        EXPECT(!MyBoyDiag_IsReadable(ranges[i][1] - 1, 2));
    }
    EXPECT(!MyBoyDiag_IsReadable(0xFFFFFFF0, 0x30));
    EXPECT(!MyBoyDiag_IsReadable(0x00000068, 4));
    EXPECT(!MyBoyDiag_IsExecutable(0));
    EXPECT(!MyBoyDiag_IsExecutable(0x23682368));
    EXPECT(!MyBoyDiag_IsExecutable(0x08000002));
    EXPECT(MyBoyDiag_IsExecutable(0x08000000));
    EXPECT(MyBoyDiag_IsExecutable(0x08000001));
    EXPECT(MyBoyDiag_IsExecutable(0x08000003));
    EXPECT(MyBoyDiag_IsExecutable(0x0A223381));
    EXPECT(MyBoyDiag_IsExecutable(0x0C223381));
    EXPECT(MyBoyDiag_IsExecutable(0x02000001));
    EXPECT(MyBoyDiag_IsExecutable(0x03000000));
}

TEST("My Boy diagnostic script validation rejects corrupt context fields before dereference")
{
    const u8 script[] = {0};
    ScrCmdFunc table[] = {DiagnosticCommand};
    struct ScriptContext ctx;
    struct ScriptContext bad;

    InitScriptContext(&ctx, table, table + ARRAY_COUNT(table));
    SetupBytecodeScript(&ctx, script);
    EXPECT(MyBoyDiag_IsScriptValid(&ctx));
    bad = ctx;
    bad.scriptPtr = (const u8 *)0x23682368;
    EXPECT(!MyBoyDiag_IsScriptValid(&bad));
    bad = ctx;
    bad.scriptPtr = NULL;
    EXPECT(!MyBoyDiag_IsScriptValid(&bad));
    bad = ctx;
    bad.mode = 3;
    EXPECT(!MyBoyDiag_IsScriptValid(&bad));
    bad = ctx;
    bad.stackDepth = SCRIPT_STACK_SIZE;
    EXPECT(!MyBoyDiag_IsScriptValid(&bad));
    bad = ctx;
    bad.cmdTableEnd = bad.cmdTable;
    EXPECT(!MyBoyDiag_IsScriptValid(&bad));
    bad = ctx;
    bad.cmdTable = (ScrCmdFunc *)0x02000001;
    bad.cmdTableEnd = (ScrCmdFunc *)0x02000005;
    EXPECT(!MyBoyDiag_IsScriptValid(&bad));
    bad = ctx;
    bad.cmdTable = (ScrCmdFunc *)0x23682368;
    bad.cmdTableEnd = (ScrCmdFunc *)0x2368236C;
    EXPECT(!MyBoyDiag_IsScriptValid(&bad));
}

TEST("My Boy diagnostic opcodes distinguish execution from effect look-ahead")
{
    const u8 script[] = {0};
    ScrCmdFunc table[] = {DiagnosticCommand};
    struct ScriptContext ctx;
    u32 slot;

    MyBoyDiag_Init();
    InitScriptContext(&ctx, table, table + ARRAY_COUNT(table));
    SetupBytecodeScript(&ctx, script);
    MyBoyDiag_Script(&ctx, FALSE);
    slot = (gMyBoyDiagBuffer.nextSequence - 2) & (MYBOY_DIAG_CAPACITY - 1);
    EXPECT_EQ(gMyBoyDiagBuffer.events[slot].kind, MYBOY_EVENT_OPCODE);
    EXPECT_EQ(gMyBoyDiagBuffer.events[slot].arg0, (uintptr_t)script);
    EXPECT_EQ(gMyBoyDiagBuffer.events[slot].arg1, 0);
    MyBoyDiag_Script(&ctx, TRUE);
    slot = (gMyBoyDiagBuffer.nextSequence - 2) & (MYBOY_DIAG_CAPACITY - 1);
    EXPECT_EQ(gMyBoyDiagBuffer.events[slot].arg1, 0x80000000);
    EXPECT_EQ(gMyBoyDiagBuffer.lastScript, (uintptr_t)script);
    EXPECT_EQ(gMyBoyDiagBuffer.lastOpcode, 0);
    sDiagnosticCommandCalls = 0;
    EXPECT(RunScriptCommand(&ctx));
    EXPECT_EQ(sDiagnosticCommandCalls, 1);
    EXPECT_EQ(gMyBoyDiagBuffer.faultCode, 0);
}
#endif
