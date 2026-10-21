#include "global.h"
#include "event_data.h"
#include "script.h"
#include "test/test.h"
#include "test/overworld_script.h"

// Capture the PC in the callback itself: an ordinary C symbol address
// would hide execution through a ROM alias.
static EWRAM_DATA uintptr_t sExecutionPc;
static EWRAM_DATA u32 sExecutions;
#define RECORD_EXECUTION() do { uintptr_t pc; asm volatile ("mov %0, pc" : "=r" (pc)); sExecutionPc = pc; sExecutions++; } while (0)

static NOINLINE bool8 MirrorCommand(struct ScriptContext *ctx)
{
    RECORD_EXECUTION();
    return TRUE;
}

static NOINLINE bool8 MirrorNative(void)
{
    RECORD_EXECUTION();
    return TRUE;
}

static USED NOINLINE void ScriptRomMirror_Call(struct ScriptContext *ctx)
{
    Script_RequestEffects(SCREFF_V1);
    RECORD_EXECUTION();
}

static USED NOINLINE bool8 ScriptRomMirror_Goto(void)
{
    Script_RequestEffects(SCREFF_V1);
    RECORD_EXECUTION();
    return TRUE;
}

static USED NOINLINE void ScriptRomMirror_SaveEffect(struct ScriptContext *ctx)
{
    Script_RequestEffects(SCREFF_V1 | SCREFF_SAVE);
    RECORD_EXECUTION();
}

static void ResetProbe(void)
{
    sExecutionPc = 0;
    sExecutions = 0;
}

static void ExpectPrimaryExecution(void)
{
    EXPECT_EQ(sExecutions, 1);
    EXPECT(sExecutionPc >= ROM_START);
    EXPECT(sExecutionPc < ROM_END);
}

TEST("Script ROM mirror normalization preserves primary ROM, RAM, null and Thumb bits")
{
    static const uintptr_t addresses[][2] = {
        {0, 0}, {0x02001235, 0x02001235}, {0x03000100, 0x03000100},
        {0x08001235, 0x08001235}, {0x09FFFFFF, 0x09FFFFFF},
        {0x0A000000, 0x08000000}, {0x0A223311, 0x08223311},
        {0x0BFFFFFF, 0x09FFFFFF}, {0x0C000000, 0x0C000000},
    };
    for (u32 i = 0; i < ARRAY_COUNT(addresses); i++)
        EXPECT_EQ(Script_GetExecutableAddress(addresses[i][0]), addresses[i][1]);
}

TEST("Script ROM mirror command and deferred native dispatch execute primary ROM")
{
    struct ScriptContext ctx;
    const u8 script[] = {0};
    // Control: the runner really can observe execution in mirror 1.
    ScrCmdFunc mirrored = (ScrCmdFunc)((uintptr_t)MirrorCommand + 0x02000000);
    ResetProbe();
    mirrored(&ctx);
    EXPECT_EQ(sExecutions, 1);
    EXPECT(sExecutionPc >= 0x0A000000);
    EXPECT(sExecutionPc < 0x0C000000);
    for (u32 tagged = 0; tagged < 2; tagged++)
    {
        ScrCmdFunc table[] = {(ScrCmdFunc)((uintptr_t)MirrorCommand + tagged * 0x02000000)};
        InitScriptContext(&ctx, table, table + ARRAY_COUNT(table));
        SetupBytecodeScript(&ctx, script);
        ResetProbe();
        EXPECT(RunScriptCommand(&ctx));
        ExpectPrimaryExecution();
        InitScriptContext(&ctx, table, table + ARRAY_COUNT(table));
        SetupNativeScript(&ctx, (bool8 (*)(void))((uintptr_t)MirrorNative + tagged * 0x02000000));
        EXPECT_EQ((uintptr_t)ctx.nativePtr, (uintptr_t)MirrorNative + tagged * 0x02000000);
        ResetProbe();
        EXPECT(RunScriptCommand(&ctx));
        ExpectPrimaryExecution();
    }
}

TEST("Script ROM mirror callnative and gotonative execute primary ROM")
{
    struct ScriptContext ctx;
    extern ScrCmdFunc gScriptCmdTable[], gScriptCmdTableEnd[];
    const u8 *call = OVERWORLD_SCRIPT(callnative ScriptRomMirror_Call, requests_effects=1; end;);
    const u8 *go = OVERWORLD_SCRIPT(gotonative ScriptRomMirror_Goto, requests_effects=1; end;);
    ResetProbe();
    RunScriptImmediately(call);
    ExpectPrimaryExecution();
    InitScriptContext(&ctx, gScriptCmdTable, gScriptCmdTableEnd);
    SetupBytecodeScript(&ctx, go);
    ResetProbe();
    EXPECT(RunScriptCommand(&ctx));
    EXPECT_EQ(sExecutions, 0);
    EXPECT((uintptr_t)ctx.nativePtr >= 0x0A000000);
    EXPECT((uintptr_t)ctx.nativePtr < 0x0C000000);
    EXPECT(RunScriptCommand(&ctx));
    ExpectPrimaryExecution();
}

TEST("Script ROM mirror effects analysis keeps raw tags and blocks save effects")
{
    const u8 *safe = OVERWORLD_SCRIPT(callnative ScriptRomMirror_Call, requests_effects=1; end;);
    const u8 *save = OVERWORLD_SCRIPT(callnative ScriptRomMirror_SaveEffect, requests_effects=1; end;);
    const u8 *unknown = OVERWORLD_SCRIPT(callnative ScriptRomMirror_Call; end;);
    ResetProbe();
    EXPECT(Script_HasNoEffect(safe));
    ExpectPrimaryExecution();
    ResetProbe();
    EXPECT(!Script_HasNoEffect(save));
    EXPECT_EQ(sExecutions, 0);
    EXPECT(!Script_HasNoEffect(unknown));
    EXPECT_EQ(sExecutions, 0);
    EXPECT_EQ(gScriptEffectContext, NULL);
}

TEST("Script ROM mirror copyvar and compare preserve the starter switch")
{
    const u8 *script = OVERWORLD_SCRIPT(
        copyvar VAR_0x8000, VAR_0x8004;
        compare VAR_0x8000, 2;
        goto_if_eq Matched;
        setvar VAR_RESULT, 0;
        end;
      Matched:
        setvar VAR_RESULT, 1;
        end;
    );
    const u16 saved0 = gSpecialVar_0x8000;
    const u16 saved4 = gSpecialVar_0x8004;
    const u16 savedResult = gSpecialVar_Result;
    gSpecialVar_0x8000 = 0;
    gSpecialVar_0x8004 = 2;
    gSpecialVar_Result = 0;
    RunScriptImmediately(script);
    EXPECT_EQ(gSpecialVar_0x8000, 2);
    EXPECT_EQ(gSpecialVar_0x8004, 2);
    EXPECT_EQ(gSpecialVar_Result, 1);
    gSpecialVar_0x8000 = saved0;
    gSpecialVar_0x8004 = saved4;
    gSpecialVar_Result = savedResult;
}
