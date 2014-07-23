#pragma once
/*
 * heap-handle.h
 *
 *  Created on: Dec 9, 2013
 */

#include "Common.h"



/**
 * Uses information recorded by the {Local,Global}{Alloc,ReAlloc,Lock,Unlock,Free}
 * routines in order to populate information about a HGLOBAL/HLOCAL handle.
 *
 * @param Candidate   Candidate handle value
 *
 * @return
 * Pointer to the base of the allocation as it is provided to the user,
 * if \a Candidate is a valid HGLOBAL\HLOCAL.  Otherwise, returns zero.
 */
ADDRINT
TranslateThroughRtlHandle
(
    ADDRINT Candidate
);


/**
 * Hooks RtlAllocateHandle so that we have a database of all HGLOBAL/HLOCAL
 * handles which have been allocated via GlobalAlloc\LocalAlloc.
 *
 * It's important to understand Local/GlobalAlloc's behavior in regard to
 * the MEM_FIXED vs MEM_MOVEABLE flags.  For MEM_FIXED, the allocator
 * routines behave exactly like any other -- and just return memory.  This
 * is properly caught by PageHeap.  However, MEM_MOVEABLE returns an opaque
 * handle.
 *
 * This opaque handle is actually four bytes into the handle value returned
 * by RtlAllocateHandle.  Because of this, we can track every handle returned
 * by RtlAllocateHandle, and compare candidate values (after ensuring proper
 * alignment and subtracting four bytes).
 *
 * Additionally, each Handle is always backed by memory when allocated through
 * GlobalAlloc or LocalAlloc.  This memory allocation is captured by PageHeap,
 * so there is no need to expressly add metadata.
 */
class RtlAllocateHandleHook : public RoutineHook
{
public:
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


};


class RtlFreeHandleHook : public RoutineHook
{
public:
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


};
