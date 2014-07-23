#pragma once
/*
 * allocator.h
 *
 *  Created on: Oct 31, 2013
 */

#include "common.h"

struct Thread;

void
InstallAllocatorHookList
(
);


/**
 * Callback which adds metadata to the allocation, and modifies the
 * return value from an allocator so that the application does not
 * see the metadata we add to the beginning.
 *
 * @param Eax
 * Value of register Eax upon return from the routine.
 *
 * @return Value to set Eax to.
 * @see #SetAllocatorReturnCallback
 */
ADDRINT
FixAllocatorReturnValue
(
    ADDRINT ReturnValue
);


/**
 * Helper which fixes the argument size to the allocator.
 *
 * e.g. If the allocation request is for 0x1000 bytes, we need
 * to change it to ~0x10b0 bytes in order to store the metadata.
 */
ADDRINT
FixAllocatorSizeArgument
(
    ADDRINT size
);


/**
 * Helper which fixes the argument address on a de-allocator
 *
 * e.g. If the free request is for address 0x12341000, we may have
 * metadata before that address, which means the actual address
 * that needs to be passed to the free() routine is 0x12340f50
 */
ADDRINT
FixDeallocatorBaseArgument
(
    ADDRINT base
);


/**
 * Class which hooks allocator routines, in order to add metadata
 * to the beginning of the allocation.
 *
 * This implementation only works with
 */
class AllocatorHook : public RoutineHook
{
public:
    /** Constructur which specifies the argument index for the size argument */
    AllocatorHook
    (
        ADDRINT SizeArgIdx = 0
    );

    virtual void
    OnRoutineEntry
    (
        ADDRINT *arg0,
        ADDRINT * arg1,
        ADDRINT * arg2,
        ADDRINT * arg3
    );


    virtual void
    OnRoutineExit
    (
        ADDRINT * ReturnValue
    );


    ADDRINT SizeArgument;
};

/**
 * Class which hooks de-allocator routines, in order to shift the
 * base address passed in to account for the modification made to
 * the value returned to the application by #AllocatorHook::OnRoutineExit
 */
class DeallocatorHook : public RoutineHook
{
public:
    /** Constructur which specifies the argument index for the base address argument */
    DeallocatorHook
    (
        ADDRINT BaseAddrArgIdx = 0
    );

    virtual void
    OnRoutineEntry
    (
        ADDRINT * arg0,
        ADDRINT * arg1,
        ADDRINT * arg2,
        ADDRINT * arg3
    );


    virtual void
    OnRoutineExit
    (
        ADDRINT * ReturnValue
    );


    ADDRINT BaseArgument;
};
