#pragma once
/*
 * indirect-call-instrumenter.h
 *
 *  Created on: Dec 19, 2013
 */

#include "common.h"

class BranchLogger : public InstructionInstrumenter
{
    void
    Instrument
    (
        INS ins
    );


    static
    VOID
    PIN_FAST_ANALYSIS_CALL
    Instrumentation
    (
        ADDRINT IP,
        ADDRINT Target,
        ADDRINT Fallthrough,
        ADDRINT DidGoToTarget
    );


};
