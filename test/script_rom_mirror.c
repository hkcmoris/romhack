#include "global.h"
#include "event_data.h"
#include "script.h"
#include "test/test.h"
#include "test/overworld_script.h"

// Capture the PC in the callback itself: an ordinary C symbol address
// would hide execution through a ROM alias.
static EWRAM_DATA uintptr_t sExecutionPc;
static EWRAM_DATA u32 sExecutions;
static EWRAM_DATA bool8 sNativeFinished;

#define ROM_MIRROR_OFFSET 0x02000000
#define RECORD_EXECUTION() do { uintptr_t pc; asm volatile ("mov %0, pc" : "=r" (pc)); sExecutionPc = pc; sExecutions++; } while (0)

static NOINLINE bool8 MirrorCommand(struct ScriptContext *ctx)
{
    RECORD_EXECUTION();
    return TRUE;
}

static NOINLINE bool8 MirrorNative(void)
{
    RECORD_EXECUTION();
    return sNativeFinished;
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

static void ExpectExecutionRegion(u32 tagged)
{
    EXPECT_EQ(sExecutions, 1);
    EXPECT(sExecutionPc >= ROM_START + tagged * ROM_MIRROR_OFFSET);
    EXPECT(sExecutionPc < ROM_END + tagged * ROM_MIRROR_OFFSET);
}

TEST("Script ROM mirror raw callback tags distinguish primary and mirror execution")
{
    struct ScriptContext ctx;

    // Positive controls bypass the dispatcher. PC must distinguish primary
    // and mirror callbacks, rather than just reporting a C symbol's address.
    for (u32 tagged = 0; tagged < 2; tagged++)
    {
        const uintptr_t raw = (uintptr_t)MirrorCommand + tagged * ROM_MIRROR_OFFSET;
        ScrCmdFunc table[] = {(ScrCmdFunc)raw};
        InitScriptContext(&ctx, table, table + ARRAY_COUNT(table));
        EXPECT_EQ((uintptr_t)*ctx.cmdTable, raw);
        ResetProbe();
        (*ctx.cmdTable)(&ctx);
        ExpectExecutionRegion(tagged);
        EXPECT_EQ((uintptr_t)*ctx.cmdTable, raw);
    }
}

TEST("Script ROM mirror command and deferred native dispatch retain raw execution addresses")
{
    struct ScriptContext ctx;
    const u8 script[] = {0};

    for (u32 tagged = 0; tagged < 2; tagged++)
    {
        const uintptr_t rawCommand = (uintptr_t)MirrorCommand + tagged * ROM_MIRROR_OFFSET;
        const uintptr_t rawNative = (uintptr_t)MirrorNative + tagged * ROM_MIRROR_OFFSET;
        ScrCmdFunc table[] = {(ScrCmdFunc)rawCommand};
        InitScriptContext(&ctx, table, table + ARRAY_COUNT(table));
        SetupBytecodeScript(&ctx, script);
        ResetProbe();
        EXPECT(RunScriptCommand(&ctx));
        ExpectExecutionRegion(tagged);
        EXPECT_EQ((uintptr_t)*ctx.cmdTable, rawCommand);

        InitScriptContext(&ctx, table, table + ARRAY_COUNT(table));
        SetupNativeScript(&ctx, (bool8 (*)(void))rawNative);
        const u8 nativeMode = ctx.mode;
        EXPECT_EQ((uintptr_t)ctx.nativePtr, rawNative);
        sNativeFinished = FALSE;
        ResetProbe();
        EXPECT(RunScriptCommand(&ctx));
        ExpectExecutionRegion(tagged);
        EXPECT_EQ(ctx.mode, nativeMode);
        EXPECT_EQ((uintptr_t)ctx.nativePtr, rawNative);

        sNativeFinished = TRUE;
        ResetProbe();
        EXPECT(RunScriptCommand(&ctx));
        ExpectExecutionRegion(tagged);
        EXPECT_NE(ctx.mode, nativeMode);
        EXPECT_EQ((uintptr_t)ctx.nativePtr, rawNative);
        // There is no bytecode after this native, so its completed wait must
        // advance to the ordinary stopped context on the next update.
        EXPECT(!RunScriptCommand(&ctx));
    }
}

TEST("Script ROM mirror callnative and gotonative preserve marked and unmarked targets")
{
    struct ScriptContext ctx;
    extern ScrCmdFunc gScriptCmdTable[], gScriptCmdTableEnd[];
    const u8 *calls[] = {
        OVERWORLD_SCRIPT(callnative ScriptRomMirror_Call; end;),
        OVERWORLD_SCRIPT(callnative ScriptRomMirror_Call, requests_effects=1; end;),
    };
    const u8 *gotos[] = {
        OVERWORLD_SCRIPT(gotonative ScriptRomMirror_Goto; end;),
        OVERWORLD_SCRIPT(gotonative ScriptRomMirror_Goto, requests_effects=1; end;),
    };

    for (u32 tagged = 0; tagged < 2; tagged++)
    {
        ResetProbe();
        RunScriptImmediately(calls[tagged]);
        ExpectExecutionRegion(tagged);
        InitScriptContext(&ctx, gScriptCmdTable, gScriptCmdTableEnd);
        SetupBytecodeScript(&ctx, gotos[tagged]);
        ResetProbe();
        EXPECT(RunScriptCommand(&ctx));
        EXPECT_EQ(sExecutions, 0);
        EXPECT_EQ((uintptr_t)ctx.nativePtr, (uintptr_t)ScriptRomMirror_Goto + tagged * ROM_MIRROR_OFFSET);
        EXPECT(RunScriptCommand(&ctx));
        ExpectExecutionRegion(tagged);
        EXPECT_EQ((uintptr_t)ctx.nativePtr, (uintptr_t)ScriptRomMirror_Goto + tagged * ROM_MIRROR_OFFSET);
    }
}

TEST("Script ROM mirror effects analysis keeps raw tags and blocks save effects")
{
    const u8 *safe = OVERWORLD_SCRIPT(callnative ScriptRomMirror_Call, requests_effects=1; end;);
    const u8 *save = OVERWORLD_SCRIPT(callnative ScriptRomMirror_SaveEffect, requests_effects=1; end;);
    const u8 *unknown = OVERWORLD_SCRIPT(callnative ScriptRomMirror_Call; end;);
    ResetProbe();
    EXPECT(Script_HasNoEffect(safe));
    ExpectExecutionRegion(1);
    ResetProbe();
    EXPECT(!Script_HasNoEffect(save));
    EXPECT_EQ(sExecutions, 0);
    EXPECT(!Script_HasNoEffect(unknown));
    EXPECT_EQ(sExecutions, 0);
    EXPECT_EQ(gScriptEffectContext, NULL);
}

TEST("Script ROM mirror copyvar and compare preserve the fire starter switch")
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
