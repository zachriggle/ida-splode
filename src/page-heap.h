#pragma once
/*
 * page-heap.h
 *
 *  Created on: Dec 16, 2013
 */

#include "Common.h"

/**
 * Hook for redirecting existing allocators to PageHeap allocators.
 *
 * This has the triple-whammy effect of:
 *
 * - Not needing to specify an AllocatorHook for an allocator
 * - Getting trace/size information for an allocation via PageHeap
 * - Generating #AV via the guard page allocated by PageHeap
 *
 * Works by moving execution via PIN_ExecuteAt, after fixing up the stack
 * and stack-based arguments as necessary.  Execution is moved to either
 * @c malloc or @c free.
 */
class PageHeapRedirector : public AddressInstrumenter<INS>
{
protected:
    PageHeapRedirector
    (
    );

    ADDRINT ArgumentIndex;
    ADDRINT NumberOfStackArguments;
    ADDRINT TargetAddress;

    /**
     * Instrumentation for the allocator page heap hook.
     * @param ArgumentToPass
     * Argument to pass to @c malloc or @c free.
     *
     * @param ArgumentsPassed
     * Total stack-based arguments passed to the original routine.
     *
     * @param ReturnAddress
     * Return address, written back onto the stack
     *
     * @param Ctx
     * Execution context
     */
    static void
    InstrumentationEntry
    (
        PageHeapRedirector  * self,
        ADDRINT             ArgumentToPass,
        ADDRINT             ReturnAddress,
        CONTEXT             * Ctx
    );


    /** @copydoc InstructionInstrumenter::Instrument */
    void
    Instrument
    (
        INS ins
    );


    /** Unused, since we define our own InstrumentInstruction */
    void
    OnRoutineEntry
    (
        ADDRINT *arg0,
        ADDRINT * arg1,
        ADDRINT * arg2,
        ADDRINT * arg3
    );


    /** Unused, since we define our own InstrumentInstruction */
    void
    OnRoutineExit
    (
        ADDRINT * ReturnValue
    );


};

class MallocPageHeapRedirector : public PageHeapRedirector
{
public:
    /**
     * @param ArgumentIndex
     * Argument index of the requested allocation size.
     *
     * @param NumberOfStackArguments
     * Total number of arguments to the allocator.  Used to fix the stack.
     */
    MallocPageHeapRedirector
    (
        ADDRINT ArgumentIndex,
        ADDRINT NumberOfStackArguments
    );
};

class FreePageHeapRedirector : public PageHeapRedirector
{
public:
    /**
     * @param ArgumentIndex
     * Argument index of the requested allocation size.
     *
     * @param NumberOfStackArguments
     * Total number of arguments to the allocator.  Used to fix the stack.
     */
    FreePageHeapRedirector
    (
        ADDRINT ArgumentIndex,
        ADDRINT NumberOfStackArguments
    );
};
