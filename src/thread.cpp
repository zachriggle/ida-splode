/*
 * thread.cpp
 *
 *  Created on: Nov 19, 2013
 */

#include "thread.h"

//map<THREADID, Thread> Thread::ThreadIdToThread;

TLS_KEY Thread::TlsKey;
long    Thread::DidInitializeTlsKey = 0;


Thread::Thread
(
)   : Logging(0),
      ThreadId(-1)
{
    if(0 == _InterlockedCompareExchange(&DidInitializeTlsKey, 1, 0))
    {
        TlsKey = PIN_CreateThreadDataKey(0);
    }
}



//------------------------------ ON THREAD START -------------------------------
void
Thread::OnStart
(
    THREADID            ThreadId,
    LEVEL_VM::CONTEXT   * Context,
    INT32               Flags,
    void                * Ignored
)
{
    cerr << "[pintool] Thread " << ThreadId << " starting" << endl;
    Thread* t = new Thread;
    t->Init(ThreadId, Context);
    PIN_SetThreadData(TlsKey, t, ThreadId);
    // ThreadIdToThread[ThreadId].Init(ThreadId, Context);
}



//------------------------------- ON THREAD FINI -------------------------------
void
Thread::OnFini
(
    THREADID                ThreadId,
    const LEVEL_VM::CONTEXT * Context,
    INT32                   Code,
    void                    * Ignored
)
{
    cerr << "[pintool] Thread " << ThreadId << " exiting" << endl;
    // ThreadIdToThread.erase(ThreadId);
}



//------------------------ THREAD STACK :: CONSTRUCTOR -------------------------
void
Thread::Init
(
    THREADID            threadid,
    LEVEL_VM::CONTEXT   *context
)
{
    memset(this, 0, sizeof(*this));

    if(0 == KnobLoggedRoutines.NumberOfValues())
    {
        Logging = 1;
    }

    MEMORY_BASIC_INFORMATION mbi;
    PIN_GetTid();

    if (threadid)
    {
        ThreadId = threadid;
    }


    //
    // Walk the entire allocation chain for a stack.  For reference,
    // here's what a stack allocation generally looks like:
    //
    // > !address
    //    BaseAddr EndAddr+1 RgnSize     Type       State                 Protect             Usage
    // +   5f0000   6ec000    fc000 MEM_PRIVATE MEM_RESERVE                                    Stack      [~0; 1b38.2624]
    //     6ec000   6ee000     2000 MEM_PRIVATE MEM_COMMIT  PAGE_READWRITE|PAGE_GUARD          Stack      [~0; 1b38.2624]
    //     6ee000   6f0000     2000 MEM_PRIVATE MEM_COMMIT  PAGE_READWRITE                     Stack      [~0; 1b38.2624]
    //
    // The bounds that we want are 5f0000 (min) to 6f0000-1 (max).
    // By querying directly on ESP, we see. that these values are captured
    // by the 'AllocationBase' and 'EndAddress' values.
    //
    // > !address @esp
    // Usage:                  Stack
    // Base Address:           006ee000
    // End Address:            006f0000
    // Region Size:            00002000
    // State:                  00001000    MEM_COMMIT
    // Protect:                00000004    PAGE_READWRITE
    // Type:                   00020000    MEM_PRIVATE
    // Allocation Base:        005f0000
    // Allocation Protect:     00000004    PAGE_READWRITE
    // More info:              ~0k
    //
    ADDRINT StackPointer = PIN_GetContextReg(context, REG_STACK_PTR);

    if (StackPointer && VirtualQuery((void *) StackPointer, &mbi, sizeof(mbi)))
    {
        StackBounds = MemRange(mbi.AllocationBase, PtrAtOffset(mbi.BaseAddress, mbi.RegionSize));
        cerr << "[pintool] Stack  " << MemRangeToString(StackBounds) << " for thread #" << ThreadId << endl;
    }


    //
    // Push a fake CallFrame, so that the entry point can be analyzed
    //
    Frames.push_back(CallFrame(PIN_GetContextReg(context, REG_INST_PTR), StackPointer));
}



//---------------------------- THREAD STACK :: DEEP ----------------------------
CallFrame *
Thread::YoungestFrame
(
)
{
    return &(Frames[Frames.size() - 1]);
}



//-------------------------- THREAD STACK :: SHALLOW ---------------------------
CallFrame *
Thread::OldestFrame
(
)
{
    return &(Frames[0]);
}



//---------------------------- THREAD STACK :: LOW -----------------------------
ADDRINT
Thread::LowestStackFrameAddress
(
)
{
    return OldestFrame()->StackPointerAtEntry;
}



//---------------------------- THREAD STACK :: HIGH ----------------------------
ADDRINT
Thread::HighestStackFrameAddress
(
)
{
    return YoungestFrame()->StackPointerAtEntry;
}



//------------------------ THREAD STACK :: GET CURRENT -------------------------
Thread *
Thread::Get
(
    THREADID id
)
{
    Thread * t = (Thread*) PIN_GetThreadData(TlsKey, id);
    return t;
}


//------------------- THREAD STACK :: FIND FRAME CONTAINING --------------------
CallFrame *
Thread::FindFrameContaining
(
    ADDRINT addr
)
{
    //
    // XXX We only search the current thread stack for variables.
    //     If there's some cross-thread stack action going on,
    //     we ignore that for now.  However, that could be a cool
    //     way to find concurrency bugs.
    //
    Thread * ts = Thread::Get();

    //
    // Check the allocation limits
    //
    // If there is only one frame, we can only tell whether the pointer
    // is in the current frame
    //
    if (  ts != NULL
       && ts->Frames.size() >= 1
       && ts->StackBounds.Contains(Addrint2VoidStar(addr)))
    {
        //
        // Walk frames backward from the most recent to the least,
        // it makes sense that this would be faster.
        //
        ptrdiff_t i = ts->Frames.size();

        //
        // If the address is below ESP as of when we entered the
        // frame, then assume that it is in the current frame.
        //
        // Since we don't know where the stack frame stops, make a judgment
        // that if a single frame shouldn't use more than a page.
        //
        CallFrame * Frame = &ts->Frames[0];

        while (--i >= 0)
        {
            size_t Offset = Frame[i].StackPointerAtEntry - addr;

            if (Offset < 0x1000)
            {
                //                __debugbreak();
                return &Frame[i];
            }
        }

    }

    return NULL;
}



//-------------------------- THREAD STACK :: DUMP TOP --------------------------
void
Thread::DumpFrameToStderr
(
    const CallFrame& cf
) const
{
    cerr << "[pintool] [" << ThreadId << "] " << cf.StackPointerAtEntry << " ";

    for (size_t i = 0; i < Frames.size(); i++)
    {
        cerr << ".";
    }

    cerr << cf << endl;
}



_RTL_STACK_TRACE_ENTRY
Thread::operator=
(
    Thread& Stack
)
{
    RTL_STACK_TRACE_ENTRY   rste;
    // IMPLEMENT
    //    __debugbreak();
    CallFrame               * Frame     = NULL; //Stack.youngest();
    const size_t            NumFrames   = min(Stack.Frames.size(), ARRAYSIZE(rste.BackTrace));
    rste.Depth = NumFrames;

    for (size_t DstFrame = 0;
         Frame && DstFrame < NumFrames;
         DstFrame++, Frame--)
    {
        rste.BackTrace[DstFrame] = *(ADDRINT *)Frame->StackPointerAtEntry;
    }

    return rste;
}



void
Thread::EnableLogging
(
)
{
    ATOMIC::OPS::Increment(&Logging, 1L);
}



void
Thread::DisableLogging
(
)
{
    ATOMIC::OPS::Increment(&Logging, -1L);
}



bool
Thread::ShouldLog
(
)
{
    return Logging > 0;
}
