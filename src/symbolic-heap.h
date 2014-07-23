#pragma once

#include "stack.h"
#include "common.h"
#include "symbol.h"

/**
 * Describes the various 'stamps' at the beginning/end of a PageHeap block.
 *
 * @see Documentation for Windbg !heap extension (http://goo.gl/h2H0UE)
 * @see 'Advanced Windows Debugging' pp753-757 by Hewardt and Pravat for info,
 *      specifically tables A.2 and A.3.
 */
const UINT32 BlockHeaderEndFill             = 0xdcbaaaaa,
             BlockHeaderEndFillFree         = 0xdcbaaaa9,
             BlockHeaderEndMask             = 0xdcba0000,
             BlockHeaderStartFill           = 0xabcdaaaa,
             BlockHeaderStartFillFree       = 0xabcdaaa9,
             BlockHeaderStartMask           = 0xabcd0000,
             FullBlockHeaderEndFill         = 0xdcbabbbb,
             FullBlockHeaderEndFillFree     = 0xdcbabbba,
             FullBlockHeaderStartFill       = 0xabcdbbbb,
             FullBlockHeaderStartFillFree   = 0xabcdbbba,
             FullPostAllocationFill         = 0xd0d0d0d0,
             FullUserAllocationFill         = 0xc0c0c0c0,
             FullUserAllocationFillFree     = 0xf0f0f0f0,
             PostAllocationFillPatern       = 0xa0a0a0a0,
             UserAllocationFill             = 0xe0e0e0e0,
             UserAllocationFillFree         = 0xf0f0f0f0;

/**
 * Describes a PageHeap header block, as defined by MSDN.
 *
 * @see "The Structure of a Page Heap Block" on MSDN (http://goo.gl/YBEVPO)
 */
__declspec(align(0x10))
typedef struct _DPH_BLOCK_INFORMATION
{

    UINT32  StartStamp;
    void                * Heap;
    UINT32  RequestedSize;
    UINT32  ActualSize;
    union
    {
        LIST_ENTRY         FreeQueue;
        SINGLE_LIST_ENTRY  FreePushList;
        UINT16 TraceIndex;
    };
    struct _RTL_STACK_TRACE_ENTRY * StackTrace;
    UINT32 EndStamp;

} DPH_BLOCK_INFORMATION, *PDPH_BLOCK_INFORMATION;



/**
 * Helper structure for resolving information regarding the stack trace
 * for a heap allocation.
 *
 * @warning Relies on brute-force discovery of internal heap structures
 *          relative to (preceding) the provided heap addresses.
 *          VerifierEnumerateResource is the proper API to use, but it
 *          is far too slow (as it walks all heap allocations).
 */
struct HeapStackTrace
{
public:
    /** Initialize the heap stack trace based on an allocation */
    HeapStackTrace
    (
        void * HeapAddress
    );

    /** Write the trace to the log file */
    ostream&
    HeapStackTrace::log
    (
        ostream& os
    ) const;


    /**
     * Pointer to the Page Heap block which describes the allocation provided.
     *
     * @warning This member is *only* valid within the PIN instrumentation
     *          routine in which it is acquired, as once execution is returned
     *          to the application, it may be freed/unmapped.
     */
    struct _DPH_BLOCK_INFORMATION * PageHeapBlock;

private:

    /**
     * Base of the requested allocation
     */
    ADDRINT ResolvedAddress;

    /** Static initializer */
    static
    void
            HeapStackTrace::Initialize
    (
    );


    /** Maximum number of frames outside of the whitelist to resolve */
    const static size_t MAX_FRAMES_OUTSIDE_WHITELIST    = 1;

    /** Maximum number of frames to resolve */
    const static size_t MAX_FRAMES_WITHIN_WHITELIST     = 5;

    /** Performs the actual resolution */
    void
    Resolve
    (
        ADDRINT HeapAddress
    );


    MemRange
    FindWhitelistRange
    (
    );


    /**
     * @return The index of first whitelisted module in the stack trace,
     *         or zero (if none after-or-including \a Begin whitelisted).
     * @arg Begin Begins search at the specified index.
     */
    size_t
    FindFirstWhitelistedModule
    (
        size_t Begin = 0
    ) const;


    /**
     * @return (Index-1) of the first non-whitelisted module in the stack trace,
     *                   or AvrfBacktraceInformation.Depth.
     * @arg Begin Begins the search at the specified index.
     * @arg Max   Maximum frames to traverse
     */
    size_t
    FindFirstNonWhitelistedModule
    (
        size_t  Begin = 0,
        size_t  Max = 5
    ) const;


};



/**
 * Dump a HeapStackTrace to a stream.
 */
ostream&
operator<<
(
    ostream           & os,
    const HeapStackTrace&
);
