#include "stack.h"
#include <stack>


//------------------------- CALL FRAME :: CONSTRUCTOR --------------------------
CallFrame::CallFrame
(
    ADDRINT FunctionEntry_,
    ADDRINT StackPointer_
)
{
    FunctionEntry       = FunctionEntry_;
    StackPointerAtEntry = StackPointer_;
}



ADDRINT
CallFrame::OffsetFromRa
(
    ADDRINT local
) const
{
    return StackPointerAtEntry - local;
}



void
CallFrame::InvokeReturnHooks
(
    ADDRINT * pReturnValue
)
{
    for (size_t i = 0; i != Hooks.size(); i++)
    {
        RoutineHook * Hook = Hooks[i];
        Hook->OnRoutineExit(pReturnValue);
    }
}


void Dump(Thread* t)
{
    for(auto i = t->Frames.rbegin(); i != t->Frames.rend(); i++)
    {
        CallFrame& f = *i;
        Symbol *s = Symbol::ResolveSymbol(f.FunctionEntry,true);
        cerr << f.StackPointerAtEntry << " " << f.FunctionEntry << " ";
        if(s) cerr << (*s);
        cerr << endl;
    }
    cerr << endl;
}

void LS(ADDRINT a)
{
    Symbol* s = Symbol::ResolveSymbol(a,true);
    if(s) cerr << *s;
    else  cerr << a;
}

//------------------------------ ON ENTER ROUTINE ------------------------------
void PIN_FAST_ANALYSIS_CALL
CallFrame::OnEnter
(
    THREADID    ThreadId,
    ADDRINT     CallInstruction,
    ADDRINT     CallTarget,
    ADDRINT     StackPointer
)
{

    CallFrame   cf(CallTarget, StackPointer);
    Thread      * thread = Thread::Get(ThreadId);
    thread->Frames.push_back(cf);

#if 0
    if (IsAddressWhitelisted(cf.FunctionEntry) && Thread::Get()->ShouldLog())
    {
        Thread::Get()->DumpFrameToStderr(cf);
    }
#endif
}



//------------------------------ ON LEAVE ROUTINE ------------------------------
void PIN_FAST_ANALYSIS_CALL
CallFrame::OnLeave
(
    THREADID    ThreadId,
    ADDRINT     RetInstruction,
    ADDRINT     RetTarget,
    ADDRINT     StackPointerAfterRet,
    ADDRINT     *pReturnValue
)
{
    Thread * thread = Thread::Get(ThreadId);


    //
    // Iterate through all of the frames until we find one with a stack
    // pointer at entry lower than the current stack pointer.
    //
    // Call that routine's hooks, and kill the rest of the frames.
    //

    for (size_t i = 0; i < thread->Frames.size(); i++)
    {
        CallFrame &Frame = thread->Frames[i];

        if (Frame.StackPointerAtEntry < StackPointerAfterRet)
        {
            Frame.InvokeReturnHooks(pReturnValue);
            thread->Frames.erase(thread->Frames.begin() + i, thread->Frames.end());

#if 0
            if (IsAddressWhitelisted(thread->YoungestFrame()->FunctionEntry) && Thread::Get()->ShouldLog())
            {
                Thread::Get()->DumpFrameToStderr(*thread->YoungestFrame());
            }
#endif

            break;
        }
    }
}



ostream&
operator<<
(
    ostream           & os,
    const CallFrame   & cf
)
{
    Symbol *s = Symbol::ResolveSymbol(cf.FunctionEntry, true);

    os << "[" << cf.StackPointerAtEntry << "] ";

    if (s)
    {
        os << s->sip.si.Name;
    }
    else
    {
        os << cf.FunctionEntry;
    }

    return os;
}



CallFrame *
CallFrame::Current
(
)
{
    return Thread::Get()->YoungestFrame();
}
