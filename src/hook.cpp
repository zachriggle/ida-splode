/*
 * hook.cpp
 *
 *  Created on: Nov 19, 2013
 */

#include "common.h"

//******************************************************************************
//                                   GLOBALS
//******************************************************************************

//LogEnablingRoutineHook      LogEnablingRoutineHookInstance;
//vector<SymbolicHooker *>    *SymbolicHooker::Instances = NULL;



//******************************************************************************
//                                   METHODS
//******************************************************************************


//------------------- ROUTINE HOOK :: INSTRUMENT INSTRUCTION -------------------
void
RoutineHook::Instrument
(
    INS ins
)
{
    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR) RoutineHook::InstrumentationEntry,
                             IARG_ADDRINT, (ADDRINT) this,
                             IARG_FUNCARG_ENTRYPOINT_REFERENCE, 0,
                             IARG_FUNCARG_ENTRYPOINT_REFERENCE, 1,
                             IARG_FUNCARG_ENTRYPOINT_REFERENCE, 2,
                             IARG_FUNCARG_ENTRYPOINT_REFERENCE, 3,
                             IARG_END);
}



//------------------- ROUTINE HOOK :: INSTRUMENTATION ENTRY --------------------
void
RoutineHook::InstrumentationEntry
(
    void    * self,
    ADDRINT * arg0,
    ADDRINT * arg1,
    ADDRINT * arg2,
    ADDRINT * arg3
)
{
    RoutineHook * rh = (RoutineHook *) self;
    CallFrame::Current()->Hooks.push_back(rh);
    rh->OnRoutineEntry(arg0, arg1, arg2, arg3);
}



//--------------- LOG-ENABLING ROUTINE HOOK :: ON ROUTINE ENTRY ----------------
void
LogEnablingRoutineHook::OnRoutineEntry
(
    ADDRINT * arg0,
    ADDRINT * arg1,
    ADDRINT * arg2,
    ADDRINT * arg3
)
{
    cerr << "[pintool] Enabling logging" << endl;
    Thread::Get()->EnableLogging();
}



//---------------- LOG-ENABLING ROUTINE HOOK :: ON ROUTINE EXIT ----------------
void
LogEnablingRoutineHook::OnRoutineExit
(
    ADDRINT * ReturnValue
)
{
    cerr << "[pintool] Disabling logging" << endl;
    Thread::Get()->DisableLogging();
}
