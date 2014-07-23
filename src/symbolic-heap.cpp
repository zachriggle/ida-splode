#include "symbolic-heap.h"

// By default, search no more than 1 megabyte for a heap marker.
#define SYMBOLIC_HEAP_MAX_SEARCH 0x100000

//******************************************************************************
//                               HEAP STACK TRACE
//******************************************************************************

//---------------------- HEAP STACK TRACE :: CONSTRUCTOR -----------------------
HeapStackTrace::HeapStackTrace
(
    void * HeapAddress
)
{
    PageHeapBlock   = NULL;
    ResolvedAddress = 0;
    Resolve((ADDRINT) HeapAddress);
}



//-------------------------- HEAP STACK TRACE :: LOG ---------------------------
ostream&
operator<<
(
    ostream               & os,
    const HeapStackTrace  & h
)
{
    return h.log(os);
}



ostream&
HeapStackTrace::log
(
    ostream& os
) const
{
    //
    // Add metadata about the allocation itself
    //
    // Single trace lines have been observed as long as 1686 characters, when
    // there are two heap traces and the backtrace is heavily templated.
    //
    if (NULL == PageHeapBlock)
    {
        return os;
    }

    ADDRINT Base    = (ADDRINT) (PageHeapBlock + 1);
    ADDRINT Size    = PageHeapBlock->RequestedSize;
    ADDRINT Offset  = ResolvedAddress - Base;

    os  << "{ \"Base\": " << Base
        << ", \"Size\": " << Size
        //        << ", \"Address\": " << ResolvedAddress
        << ", \"Offset\": " << Offset;

    //
    // Check to see if the allocation is freed
    //
    switch (PageHeapBlock->StartStamp)
    {
     case FullBlockHeaderStartFillFree:
     case BlockHeaderStartFillFree:
         os << ", \"State\": \"Free\"";
    }

    //
    // If there is no stack trace information available, quit.
    //
    size_t Depth = PageHeapBlock && PageHeapBlock->StackTrace ? PageHeapBlock->StackTrace->Depth : 0;

    if (0 == Depth)
    {
        return os << "}";
    }


    //
    // Print out the relevant frames, and symbolize everything *not* in
    // a whitelisted module.
    //
    // Display up to 5 consecutive whitelisted frames.
    //
    // 'FindFirstXxx'    defaults to 0.
    // 'FindFirstNonXxx' defaults to 'Depth'.
    //
    size_t  WhitelistStarts = FindFirstWhitelistedModule();
    size_t  WhitelistStops  = FindFirstNonWhitelistedModule(WhitelistStarts);

    //
    // The number of frames to print that are deeper/shallower than
    // the series of whitelisted elements.
    //
    size_t  DeepFrames      = 1;
    size_t  ShallowFrames   = 2;


    //
    // Determine the first and last frame to print.
    //
    // Go back as many frames as are possible/wanted.
    //
    // If an allocation does not have any whitelisted info in the call tree,
    // then only 'ShallowFrames' count of frames will be displayed.
    //
    size_t  FirstFrame  = WhitelistStarts;
    FirstFrame  -= min(DeepFrames, WhitelistStarts);

    size_t  LastFrame   = WhitelistStops;
    LastFrame   += ShallowFrames;
    LastFrame   = min(LastFrame, Depth);


    size_t  Frame;
    os << ", \"Frames\": [";


    for (Frame = FirstFrame; Frame < LastFrame; Frame++)
    {
        ADDRINT RA              = PageHeapBlock->StackTrace->BackTrace[Frame];

        Symbol  *FrameSymbol    = NULL;

        if (  Frame < WhitelistStarts
           || WhitelistStops <= Frame)
        {
            FrameSymbol = Symbol::ResolveSymbol(RA);
        }

        if (FrameSymbol)
        {
            os << *FrameSymbol;
        }
        else
        {
            os << RA;
        }

        os << ", ";
    }

    return os << "]}";
}



//------------------------ HEAP STACK TRACE :: RESOLVE -------------------------

void
HeapStackTrace::Resolve
(
    ADDRINT Address
)
{
    //
    // If 'Address' is actually a RtlAllocHandle handle (i.e. MEM_MOVEABLE allocated
    // via GlobalAlloc or LocalAlloc), translate to the actual allocation.
    //
    // This breaks the 1:1-ness in some ways, but it's for the better.
    //
    ADDRINT ActualHeapAddr = TranslateThroughRtlHandle(Address);

    if (ActualHeapAddr)
    {
        Address = ActualHeapAddr;
    }

    //
    // Union for ease of manipulation
    //
    union
    {
        void    * loopPvoid;
        ADDRINT loopAddrint;
        DPH_BLOCK_INFORMATION * loopBlock;
    };

    loopAddrint = Address;

    //
    // See Microsoft's site for documentation of the PageHeap internals, at:
    // http://msdn.microsoft.com/en-us/library/ms220938(v=vs.90).aspx
    //
    size_t          AllocatorAlignment  = 8;
    size_t          InitialSubtraction  = sizeof(DPH_BLOCK_INFORMATION) - AllocatorAlignment;
    const size_t    StampSize           = sizeof(BlockHeaderStartFill);
    size_t          BytesCopied         = 0;
    ptrdiff_t       StopStampOffset     = FIELD_OFFSET(DPH_BLOCK_INFORMATION, EndStamp);
    size_t          TotalBytes          = 0;

    //
    // Align to allocator granularity
    //
    loopAddrint &= ~(AllocatorAlignment - 1);

    //
    // Skip back at least one struct size, minus the size we subtract
    // at the top of the loop
    //
    loopAddrint -= InitialSubtraction;

    //
    // Search backward, starting one full structure size back.
    // If an exception occurs, bail.
    //
    // For whatever reason, normal __try/__except handling doesn't work
    // inside pintools.  Because of this, we have to use PIN_SafeCopy.
    //
    // Exit the loop if a copy fails (i.e. throws a #AV), if we find a match,
    // or if we process 64KB.
    //
    PageHeapBlock = NULL;

    for (  TotalBytes = 0;
           NULL == PageHeapBlock
        && TotalBytes < 0x10000;
           TotalBytes += AllocatorAlignment)
    {
        //
        // Work backwards in memory, as the block precedes the allocation
        // which the user should/ought to write to.
        //
        loopAddrint -= AllocatorAlignment;

        //
        // If the read will cross a new page boundary (moving backward),
        // ensure we won't throw a #AV.
        //
        // We don't have to check in the forward direction.
        //
        // N.B.: Using PIN_SafeCopy instead of this read check **DOUBLES**
        //       the total run-time.
        //
        ADDRINT StartPage   = loopAddrint & ~(0xfff);
        ADDRINT EndPage     = (loopAddrint + sizeof(DPH_BLOCK_INFORMATION)) & ~(0xfff);

        if (StartPage != EndPage && !PIN_CheckReadAccess(loopPvoid))
        {
            return;
        }

        //
        // Attempt to read and verify the contents of the stamps
        //
        switch (loopBlock->StartStamp)
        {
         case BlockHeaderStartFill:
         case FullBlockHeaderStartFillFree:
         case BlockHeaderStartFillFree:
         case FullBlockHeaderStartFill:
             break;
         default:
             continue;
        }

        switch (loopBlock->EndStamp)
        {
         case BlockHeaderEndFill:
         case FullBlockHeaderEndFill:
         case BlockHeaderEndFillFree:
         case FullBlockHeaderEndFillFree:
             break;
         default:
             continue;
        }

        //
        // Probe the stack entry to see if it's valid.  If it contains a non-NULL
        // address, but points to invalid memory, do not accept it, and bail out.
        //
        if (loopBlock != NULL && PIN_CheckReadAccess(loopBlock->StackTrace))
        {
            ResolvedAddress = Address;
            PageHeapBlock   = loopBlock;
        }
    }
}



//------------- HEAP STACK TRACE :: FIND FIRST WHITELISTED MODULE --------------
size_t
HeapStackTrace::FindFirstWhitelistedModule
(
    size_t Begin
) const
{
    const size_t    Depth = PageHeapBlock->StackTrace->Depth;

    size_t          Frame;

    for (Frame = Begin;
         Frame < Depth;
         Frame++)
    {
        if (IsAddressWhitelisted(PageHeapBlock->StackTrace->BackTrace[Frame]))
        {
            return Frame;
        }
    }

    return SIZE_MAX;
}



//----------- HEAP STACK TRACE :: FIND FIRST NON-WHITELISTED MODULE ------------
size_t
HeapStackTrace::FindFirstNonWhitelistedModule
(
    size_t  Begin,
    size_t  Max
) const
{
    const size_t    Depth = PageHeapBlock->StackTrace->Depth;

    size_t          Frame;

    for (  Frame = Begin;
           Frame < Depth
        && Max--;
           Frame++)
    {
        if (!IsAddressWhitelisted(PageHeapBlock->StackTrace->BackTrace[Frame]))
        {
            return Frame;
        }
    }

    return Depth;
}
