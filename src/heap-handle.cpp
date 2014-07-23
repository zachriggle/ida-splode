/*
 * heap-handle.cpp
 *
 *  Created on: Dec 9, 2013
 */
#include "Common.h"

//
// For Global/LocalAlloc, the return addresses for memory are
// aligned on 8-byte boundaries (end in XX0/XX8), while handles
// are aligned to be offset by four bytes (end in XX4/XXC)
//
#define BASE_HANDLE_MARK_BIT 0x04
#define IS_HEAP_HANDLE(x) ((ADDRINT)x & BASE_HANDLE_MARK_BIT)

set<ADDRINT> KnownHandles;

//------------------------ TRANSLATE THROUGH RTL HANDLE ------------------------
ADDRINT
TranslateThroughRtlHandle
(
    ADDRINT Candidate
)
{
    if (IS_HEAP_HANDLE(Candidate))
    {
        auto iter = KnownHandles.find(Candidate);

        if (iter != KnownHandles.end())
        {
            ADDRINT HeapMemory = *(ADDRINT *) Candidate;
            //            cerr << "[pintool] Handle " << Candidate << " => " << HeapMemory << endl;
            return HeapMemory;
        }
    }

    return 0;
}


//---------------- RTL ALLOCATE HANDLE HOOK :: ON ROUTINE ENTRY ----------------
void
RtlAllocateHandleHook::OnRoutineEntry
(
    ADDRINT * arg0,
    ADDRINT * arg1,
    ADDRINT * arg2,
    ADDRINT * arg3
)
{}


//---------------- RTL ALLOCATE HANDLE HOOK :: ON ROUTINE EXIT -----------------
void
RtlAllocateHandleHook::OnRoutineExit
(
    ADDRINT * ReturnValue
)
{
    KnownHandles.insert((*ReturnValue) + BASE_HANDLE_MARK_BIT);
}


//------------------ RTL FREE HANDLE HOOK :: ON ROUTINE ENTRY ------------------
void
RtlFreeHandleHook::OnRoutineEntry
(
    ADDRINT * arg0,
    ADDRINT * arg1,
    ADDRINT * arg2,
    ADDRINT * arg3
)
{
    KnownHandles.erase((*arg0) + BASE_HANDLE_MARK_BIT);
}


//------------------ RTL FREE HANDLE HOOK :: ON ROUTINE EXIT -------------------
void
RtlFreeHandleHook::OnRoutineExit
(
    ADDRINT * ReturnValue
)
{}
