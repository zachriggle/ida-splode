#pragma once

#include "Common.h"


//******************************************************************************
//                         WINDOWS INTERNAL STRUCTURES
//******************************************************************************

/**
 * Describes a stack trace entry, as defined by Windows Research Kernel.
 */
typedef struct _RTL_STACK_TRACE_ENTRY
{
    struct _RTL_STACK_TRACE_ENTRY *HashChain;
    UINT32 TraceCount;
    UINT16 Index;
    UINT16 Depth;
    ADDRINT BackTrace[32];
} RTL_STACK_TRACE_ENTRY, *PRTL_STACK_TRACE_ENTRY;


//******************************************************************************
//                                  CALL FRAME
//******************************************************************************
class AllocatorHook;
class RoutineHook;

/**
 * Describes a call frame.
 *
 * IP and SP are at the call site, *before* the call pushes $ra.
 */
struct CallFrame
{
    CallFrame
    (
        ADDRINT Target,
        ADDRINT StackPointer
    );
    ~CallFrame
    (
    ){}



    /**
     * Instrumentation routine invoked on each CALL instruction.
     *
     * Used to keep track of stack frames.
     */
    static void PIN_FAST_ANALYSIS_CALL
    OnEnter
    (
        THREADID    ThreadId,
        ADDRINT     CallInstruction,
        ADDRINT     CallTarget,
        ADDRINT     StackPointerAfterCall
    );


    /**
     * Instrumentation routine invoked on each RETN instruction.
     *
     * Used to keep track of stack frames.
     */
    static void PIN_FAST_ANALYSIS_CALL
    OnLeave
    (
        THREADID    ThreadId,
        ADDRINT     RetInstruction,
        ADDRINT     RetTarget,
        ADDRINT     StackPointerAfterRet,
        ADDRINT     *pReturnValue
    );


    /**
     * Helper routine to get the most recent call-frame for the
     * currently-instrumented Pin thread.
     */
    static CallFrame *
    Current
    (
    );


    union
    {
        ADDRINT FunctionEntry;      //!< Address of the routine entry point
        AFUNPTR pFunctionEntry;     //!< Debugger aid.  Makes Windbg resovle the address.
    };

    /** Stack pointer as the routine is entered (i.e. $ra is on top of the stack) */
    ADDRINT StackPointerAtEntry;

    void
            InvokeReturnHooks
    (
        ADDRINT * pReturnValue
    );


    vector<RoutineHook *> Hooks;

    /**
     * @return Stack variable offset, from the return value.
     *
     * For example, if ESP is 0x6bfe1c on entry to a routine, then the value
     * 0x6bfdfc is at esp-0x20.
     *
     * @warning In IDA pro, **MUST** account for whether the function has a frame pointer,
     *          or this offset will be incorrect!!!
     *          To check: 'idc.GetFunctionFlags(fn) & idc.FUNC_FRAME'
     */
    ADDRINT
    OffsetFromRa
    (
        ADDRINT local
    ) const;


};


ostream&
operator<<
(
    ostream           & os,
    const CallFrame   & cf
);
