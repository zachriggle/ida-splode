/*
 * allocator.cpp
 *
 *  Created on: Oct 31, 2013
 */

#include <map>
#include <new>              // placement new
#include "common.h"

//******************************************************************************
//                                   GLOBALS
//******************************************************************************
/**
 * Maps a hooked call-frame to the number of bytes requested.
 *
 * Populated by FixAllocatorSizeArgument, and used/cleaned by
 * FixAllocatorReturnValue.
 */
map<CallFrame *, ADDRINT> CallFrameToAllocationSize;

//AllocatorHook   Arg0_AllocatorHookInstance(0);
//DeallocatorHook Arg0_DeallocatorHookInstance(0);

//******************************************************************************
//                                  STRUCTURES
//******************************************************************************

/**
 * Structure used to encapsulate the metadata placed at the
 * beginning of each instrumented allocation.  Mimicks the
 * structure of a PageHeap metadata block, so that we can use
 * existing code that looks for PageHeap metadata blocks.
 */
struct FAKE_PAGEHEAP_METADATA
{
    /**
     * Initializes self as a fake block, using the provided size,
     * and pulling stack frame information from the currently-instrumented
     * Pin thread stack.
     *
     * @param RequestedSize
     * Number of bytes in the original application's allocation request.
     *
     * @see #Thread
     */
    FAKE_PAGEHEAP_METADATA
    (
        ADDRINT RequestedSize
    )
    {
        //
        // Initialize all fields
        //
        memset(this, 0, sizeof(*this));

        //
        // Make the DPH_BLOCK_INFORMATION look like a valid block
        // created by PageHeap
        //
        dbi.StackTrace      = &rste;
        dbi.RequestedSize   = RequestedSize;
        dbi.StartStamp      = FullBlockHeaderStartFill;
        dbi.EndStamp        = FullBlockHeaderEndFill;

        //
        // Fill in the stack trace information.
        //
        // Note that RTL_STACK_TRACE_ENTRY has the most recent frame at index zero.
        // In order to accommodate this, we have to walk the Thread frames backward.
        //
        Thread  * t         = Thread::Get();
        size_t  AvailFrames = t->Frames.size();
        rste.Depth = min(AvailFrames, ARRAYSIZE(rste.BackTrace));

        for (size_t FrameCount = 0; FrameCount < rste.Depth; FrameCount++)
        {
            CallFrame   *Frame          = &t->Frames[AvailFrames - FrameCount];
            ADDRINT     *pReturnAddress = (ADDRINT *) Frame->StackPointerAtEntry;
            rste.BackTrace[FrameCount] = *pReturnAddress;
        }

        //
        // Two magic values to so that we can identify which allocations
        // we have shifted.
        //
        rste.Index = MAGIC;
        rste.BackTrace[ARRAYSIZE(rste.BackTrace) - 1] = MAGIC;
    }



    bool
    isValid
    (
    )
    {
        return MAGIC == rste.Index && MAGIC == rste.BackTrace[ARRAYSIZE(rste.BackTrace) - 1];
    }



    RTL_STACK_TRACE_ENTRY   rste;
    DPH_BLOCK_INFORMATION   dbi;

    const static UINT16     MAGIC = 0xb00f;
};


//******************************************************************************
//                                   ROUTINES
//******************************************************************************

//------------------------- FIX ALLOCATOR RETURN VALUE -------------------------
ADDRINT
FixAllocatorReturnValue
(
    ADDRINT AllocationBase
)
{
    void * pAllocationBase = (void *) AllocationBase;

    if (PIN_CheckWriteAccess((void *) AllocationBase))
    {
        //
        // Find the data we stashed during #FixAllocatorArgument,
        // and clean that data up.
        //
        CallFrame   * Frame         = CallFrame::Current();
        auto        Iterator        = CallFrameToAllocationSize.find(Frame);
        ADDRINT     AllocationSize  = (*Iterator).second;

        CallFrameToAllocationSize.erase(Iterator);

        //
        // Place the fake metadata so that our heap-analysis routines will
        // pick up on it, and modify the return value seen by the application
        // accordingly.
        //
        new (pAllocationBase) FAKE_PAGEHEAP_METADATA(AllocationSize);

        ADDRINT NewAllocationBase = AllocationBase + sizeof(FAKE_PAGEHEAP_METADATA);

        cerr << "[pintool] Allocation captured at " << AllocationBase << endl;

        return NewAllocationBase;
    }

    return AllocationBase;
}



//------------------------ FIX ALLOCATOR SIZE ARGUMENT -------------------------
ADDRINT
FixAllocatorSizeArgument
(
    ADDRINT size
)
{
    //
    // Record the original size requested, for use when we return
    //
    CallFrameToAllocationSize[CallFrame::Current()] = size;

    return size += sizeof(FAKE_PAGEHEAP_METADATA);
}



//----------------------- FIX DE-ALLOCATOR BASE ARGUMENT -----------------------
ADDRINT
FixDeallocatorBaseArgument
(
    ADDRINT addr
)
{
    FAKE_PAGEHEAP_METADATA *Metadata = (FAKE_PAGEHEAP_METADATA *) addr;

    //
    // Move back one metadata-size.  If the user passed in the correct
    // 'base address' as we gave it to them, this should line us up
    // perfectly with the metadata we put at the beginning.
    //
    Metadata--;

    if (PIN_CheckReadAccess(Metadata) && Metadata->isValid())
    {
        cerr << "[pintool] Found freed allocation at " << Metadata << endl;
        return (ADDRINT) Metadata;
    }

    cerr << "[pintool] Ignoring unknown freed allocation at " << addr << endl;
    return addr;
}



//******************************************************************************
//                                   METHODS
//******************************************************************************

//----------------------- ALLOCATOR HOOK :: CONSTRUCTOR ------------------------
AllocatorHook::AllocatorHook
(
    ADDRINT SizeArgIdx
)   : SizeArgument(SizeArgIdx)
{}



DeallocatorHook::DeallocatorHook
(
    ADDRINT BaseAdrArgIdx
)   : BaseArgument(BaseAdrArgIdx)
{}



//--------------------- ALLOCATOR HOOK :: ON ROUTINE ENTRY ---------------------
void
AllocatorHook::OnRoutineEntry
(
    ADDRINT * arg0,
    ADDRINT * arg1,
    ADDRINT * arg2,
    ADDRINT * arg3
)
{
    ADDRINT *pSize = NULL;

    switch (SizeArgument)
    {
     case 0: pSize  = arg0;
         break;
     case 1: pSize  = arg1;
         break;
     case 2: pSize  = arg2;
         break;
     case 3: pSize  = arg3;
         break;
     default:
         return;
    }

    *pSize = FixAllocatorSizeArgument(*pSize);
}



//--------------------- ALLOCATOR HOOK :: ON ROUTINE EXIT ----------------------
void
AllocatorHook::OnRoutineExit
(
    ADDRINT * ReturnValue
)
{

    *ReturnValue = FixAllocatorReturnValue(*ReturnValue);

}



//------------------- DE-ALLOCATOR HOOK :: ON ROUTINE ENTRY --------------------
void
DeallocatorHook::OnRoutineEntry
(
    ADDRINT * arg0,
    ADDRINT * arg1,
    ADDRINT * arg2,
    ADDRINT * arg3
)
{
    ADDRINT *pBase = NULL;

    switch (BaseArgument)
    {
     case 0: pBase  = arg0;
         break;
     case 1: pBase  = arg1;
         break;
     case 2: pBase  = arg2;
         break;
     case 3: pBase  = arg3;
         break;
     default:
         return;
    }

    *pBase = FixDeallocatorBaseArgument(*pBase);

}



//-------------------- DE-ALLOCATOR HOOK :: ON ROUTINE EXIT --------------------
void
DeallocatorHook::OnRoutineExit
(
    ADDRINT * ReturnValue
)
{}
