#ifndef GUARD_MYBOY_DIAG_H
#define GUARD_MYBOY_DIAG_H

// All fields are little-endian u32s. Keep tools/myboy/decode_state.py in sync.
#define MYBOY_DIAG_CAPACITY 64
#define MYBOY_DIAG_MAGIC0 0x4456424D // "MBVD"
#define MYBOY_DIAG_MAGIC1 0x31474149 // "IAG1"

enum MyBoyDiagEventKind
{
    MYBOY_EVENT_BOOT = 1,
    MYBOY_EVENT_SCRIPT_START,
    MYBOY_EVENT_OPCODE,
    MYBOY_EVENT_NATIVE,
    MYBOY_EVENT_CALLBACK,
    MYBOY_EVENT_PREVIEW_REQUEST,
    MYBOY_EVENT_PREVIEW_CREATED,
    MYBOY_EVENT_PREVIEW_FAILED,
    MYBOY_EVENT_PREVIEW_CLOSED,
    MYBOY_EVENT_PREVIEW_SKIPPED,
    MYBOY_EVENT_BATTLE_CREATE,
    MYBOY_EVENT_BATTLE_TASK,
    MYBOY_EVENT_FAULT,
    MYBOY_EVENT_RESOURCE,
    MYBOY_EVENT_OPCODE_RETURN,
    MYBOY_EVENT_SPECIAL,
};

enum MyBoyDiagFault
{
    MYBOY_FAULT_SCRIPT = 1,
    MYBOY_FAULT_OPCODE,
    MYBOY_FAULT_CALLBACK,
    MYBOY_FAULT_BATTLE,
};

struct MyBoyDiagEvent
{
    u32 sequence; // Written last: zero denotes an unfinished entry.
    u32 frame;
    u32 kind;
    u32 arg0;
    u32 arg1;
    u32 map; // group << 8 | number; 0xFFFFFFFF before save pointers exist.
};

struct MyBoyDiagBuffer
{
    u32 magic0;
    u32 magic1;
    u32 version;
    u32 mode;
    u32 capacity;
    u32 eventSize;
    u32 nextSequence;
    u32 count;
    u32 faultCode;
    u32 faultArg0;
    u32 faultArg1;
    u32 lastScript;
    u32 lastOpcode;
    u32 lastCallback;
    u32 lastPreview;
    u32 lastBattleFlags;
    u32 lastSp;
    u32 lowestSp;
    u32 faultIme;
    u32 faultIe;
    u32 faultIf;
    u32 faultWaitcnt;
    struct MyBoyDiagEvent events[MYBOY_DIAG_CAPACITY];
};

struct ScriptContext;

#if MYBOY_DIAGNOSTIC_MODE
extern volatile struct MyBoyDiagBuffer gMyBoyDiagBuffer;
void MyBoyDiag_Init(void);
void MyBoyDiag_Record(u32 kind, u32 arg0, u32 arg1);
bool32 MyBoyDiag_IsReadable(uintptr_t address, u32 size);
bool32 MyBoyDiag_IsExecutable(uintptr_t address);
bool32 MyBoyDiag_IsScriptValid(const struct ScriptContext *ctx);
void MyBoyDiag_CheckCallback(uintptr_t address, u32 origin);
void MyBoyDiag_Script(const struct ScriptContext *ctx, bool32 effects);
void MyBoyDiag_Resources(void);
void MyBoyDiag_Battle(u32 transition, u32 state);
void MyBoyDiag_Fail(u32 code, u32 arg0, u32 arg1) __attribute__((noreturn));
#else
#define MyBoyDiag_Init() ((void)0)
#define MyBoyDiag_Record(kind, arg0, arg1) ((void)0)
#define MyBoyDiag_CheckCallback(address, origin) ((void)0)
#define MyBoyDiag_Script(ctx, effects) ((void)0)
#define MyBoyDiag_Resources() ((void)0)
#define MyBoyDiag_Battle(transition, state) ((void)0)
#endif

#endif
