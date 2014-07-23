
/*
 * stack-instrumenter.cpp
 *
 *  Created on: Dec 18, 2013
 */

#include "common.h"

void
StackInstrumenter::Instrument
(
    INS ins
)
{


    //
    // Instrument CALL and RET instructions for stack tracking.
    //
    // Ideally, we could use the RTN_XXX API for instrumenting this,
    // but this breaks when symbols aren't available.
    //
    // Also, we can't just instrument inside whitelisted modules,
    // as then we lose stack frame information for the first frame
    // when a cross-module call is made into a whitelisted module.
    //
    if (INS_IsCall(ins))
    {
        ADDRINT ReturnAddress = INS_Address(ins) + INS_Size(ins);

        INS_InsertCall(ins,
                       IPOINT_TAKEN_BRANCH,
                       (AFUNPTR) CallFrame::OnEnter,
                       IARG_FAST_ANALYSIS_CALL,
                       IARG_THREAD_ID,
                       IARG_ADDRINT, INS_Address(ins),
                       IARG_BRANCH_TARGET_ADDR,
                       IARG_REG_VALUE,  REG_STACK_PTR,
                       IARG_END);
    }
    else if (INS_IsRet(ins))
    {
        //
        // RETN instructions must be instrumented after the branch is taken,
        // due to the variability in the number of bytes popped.
        //
        // Additionally, IARG_FUNCRET_EXITPOINT_REFERENCE is functionally equivalent
        // to (IARG_REG_REFERENCE, REG_PIN_GAX) except that the former is the only
        // thing that works properly for manipulating return values.
        //
        INS_InsertCall(ins,
                       IPOINT_TAKEN_BRANCH,
                       (AFUNPTR) CallFrame::OnLeave,
                       IARG_FAST_ANALYSIS_CALL,
                       IARG_THREAD_ID,
                       IARG_ADDRINT, INS_Address(ins),
                       IARG_BRANCH_TARGET_ADDR,
                       IARG_REG_VALUE,  REG_STACK_PTR,
                       IARG_FUNCRET_EXITPOINT_REFERENCE,
                       IARG_END);
    }
}


bool
StackInstrumenter::ShouldInstrument(INS ins)
{
    return true;
}
