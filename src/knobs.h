#pragma once
#include "common.h"

//******************************************************************************
//                                KNOB ROUTINES
//******************************************************************************
template <typename T>
ostream&
operator<<
(
    ostream       & os,
    const KNOB<T> & ts
)
{

    return os << ts.Name() << "= " << ts.ValueString();
}



//******************************************************************************
//                             CONFIGURATION KNOBS
//******************************************************************************

/** File to write log data to */
//extern KNOB<string> KnobOutputFile;

/** List of modules whose instructions should be instrumented */
extern KNOB<string> KnobModuleWhitelist;

/** Routines to treat as entry point */
extern KNOB<string> KnobLoggedRoutines;

/** Whether to resolve whitelisted modules */
extern KNOB<BOOL>   KnobCheckWhitelistSymbols;

/** Whether to check for ASCII/UNICODE strings on memory accesses */
extern KNOB<BOOL>   KnobCheckText;

/** Whether to resolve symbols at all */
extern KNOB<BOOL>   KnobCheckSymbols;

/** Whether to resolve heap allocations */
extern KNOB<BOOL>   KnobCheckHeap;

/** Whether to track stack addresses */
extern KNOB<BOOL>   KnobCheckStack;

/** Whether to instrument mov mem instructions */
extern KNOB<BOOL>   KnobInstrumentMov;

/** Whether to instrument indirect call/branch instructions */
extern KNOB<BOOL>   KnobInstrumentIndirect;

/** Whether to instrument mov imm instructions */
extern KNOB<BOOL>   KnobInstrumentImmediate;

// MongoDB configuration
extern KNOB<string> KnobMongoHost;
extern KNOB<string> KnobMongoPort;
