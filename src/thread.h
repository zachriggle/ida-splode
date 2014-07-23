#pragma once
/*
 * thread.h
 *
 *  Created on: Nov 19, 2013
 */

#include "common.h"

//******************************************************************************
//                                    THREAD
//******************************************************************************
struct CallFrame;
struct _RTL_STACK_TRACE_ENTRY;

/**
 * Describes a thread.
 *
 * Specifically, includes information for tracking the state of the stack.
 * This is performed by instrumenting each CALL and RETN instruction.
 * Additionally, performs state tracking for entering routines of interest.
 */
struct Thread
{
    static TLS_KEY TlsKey;
    static long  DidInitializeTlsKey;

    Thread
    (
    );

    void
    Init
    (
        THREADID            threadid = 0,
        LEVEL_VM::CONTEXT   *Context = NULL
    );


    /**
     * Instrumentation routine invoked on each thread start.
     *
     * Used to keep track of stacks.
     *
     * @param ThreadId
     * @param Context
     * @param Flags
     * @param Ignored
     */
    static
    void
    OnStart
    (
        THREADID            ThreadId,
        LEVEL_VM::CONTEXT   * Context,
        INT32               Flags,
        void                * Ignored
    );


    /**
     * Instrumentation routine invoked on each thread exit.
     *
     * Used to keep track of stacks.
     *
     * @param ThreadId
     * @param Context
     * @param Flags
     * @param Ignored
     */
    static
    void
    OnFini
    (
        THREADID                ThreadId,
        const LEVEL_VM::CONTEXT * Context,
        INT32                   Code,
        void                    * Ignored
    );


    /**
     * @return Pointer to the #Thread for the specified threadid (as provided by PIN)
     */
    static
    Thread *
    Get
    (
        THREADID id = PIN_ThreadId()
    );


    /** @return Pointer to the #CallFrame containing the specified address, or NULL */
    static
    CallFrame *
    FindFrameContaining
    (
        ADDRINT addr
    );


    /** Deepest call frame */
    CallFrame *
    YoungestFrame
    (
    );


    /** Shallowest call frame */
    CallFrame *
    OldestFrame
    (
    );


    /** Conversion operator for convenience */
    _RTL_STACK_TRACE_ENTRY
    operator=
    (
        Thread&
    );


    /**
     * Upon entering a routine of interest, enable logging to file.
     * This setting is per-thread, and reentrant (i.e. counted).
     */
    void
    EnableLogging
    (
    );


    /** @see #EnableLogging */
    void
    DisableLogging
    (
    );


    /** @see #EnableLogging */
    bool
    ShouldLog
    (
    );


    THREADID    ThreadId;               //!< Thread ID as given by PIN
    MemRange    StackBounds;            //!< Describes the bounds of the thread's stack
    vector<CallFrame>   Frames;         //!< Frames in this thread's stack

    /** Lowest observed address */
    ADDRINT
    LowestStackFrameAddress
    (
    );


    /** Highest observed address */
    ADDRINT
    HighestStackFrameAddress
    (
    );



    /** Dumps the specified frame, indented */
    void
    DumpFrameToStderr
    (
        const CallFrame& cf
    ) const;


    friend inline ostream&
    operator<<
    (
        ostream         &,
        const Thread    &
    );


    /** Set to a nonzero value to allow instrumented instructions in the thread to log to file */
    long        Logging;
//private:
//    /** Map of Pin Thread IDs to #Thread structures */
//    static map<THREADID, Thread> ThreadIdToThread;
};


inline
ostream&
operator<<
(
    ostream       & os,
    const Thread  & ts
)
{
    return os << "stacks[" << ts.ThreadId << "]="  << MemRangeToString(ts.StackBounds);
}
