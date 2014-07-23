
/*
 * indirect-call-instrumenter.cpp
 *
 *  Created on: Dec 19, 2013
 */

#include "Common.h"


void
BranchLogger::Instrument
(
    INS ins
)
{
    if (  (INS_HasFallThrough(ins) && INS_IsBranchOrCall(ins))
       || (INS_IsIndirectBranchOrCall(ins) && !INS_IsRet(ins)))
    {
        INS_InsertPredicatedCall(ins,
                                 IPOINT_BEFORE,
                                 (AFUNPTR) Instrumentation,
                                 IARG_FAST_ANALYSIS_CALL,
                                 IARG_INST_PTR,
                                 IARG_BRANCH_TARGET_ADDR,
                                 IARG_FALLTHROUGH_ADDR,
                                 IARG_BRANCH_TAKEN,
                                 IARG_END);
    }
}



//------------------------- ON INDIRECT BRANCH OR CALL -------------------------
VOID
PIN_FAST_ANALYSIS_CALL
BranchLogger::Instrumentation
(
    ADDRINT IP,
    ADDRINT Target,
    ADDRINT Fallthrough,
    ADDRINT DidGoToTarget
)
{
    if (!Thread::Get()->ShouldLog())
    {
        return;
    }

    ScopedLock  LogLock(&Traces.Lock);
    ADDRINT     ActualTarget    = DidGoToTarget ? Target : Fallthrough;
    Symbol      *TargetName     = Symbol::ResolveSymbol(ActualTarget);

    Traces  << "{ \"IP\": " << IP
            << ", \"Counter\": " << ++Counter // ATOMIC::OPS::Increment<UINT32>(&Counter, 1)
            << ", \"Type\": \"Branch\""
            << ", \"Target\": ";

    if (TargetName)
    {
        Traces << *TargetName;
    }
    else
    {
        Traces << ActualTarget;
    }

    Traces << " }\n";
}
