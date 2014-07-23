#pragma once
/*
 * hook.h
 *
 *  Created on: Nov 19, 2013
 */
#include "common.h"

/**
 * Hooks a routine by instrumenting its entry, and coordinating with
 * stack-tracking instrumentation to invoke a routine when the call
 * frame is popped on a RET instruction.
 *
 * Effectively re-implements RTN_Xxx instrumentation, which is needed
 * because RTN_Xxx is only available with meaningful symbols.
 *
 * Example use-cases:
 *      Instrument allocation sizes/returned memory ranges.
 *      Modify rand() to always return zero.
 */
class RoutineHook : public virtual AddressInstrumenter<INS>
{
public:
    virtual
    ~RoutineHook
    (
    ){}



    /** @see #Instrumenter::Instrument */
    virtual void
    Instrument
    (
        INS ins
    );


    /**
     * Instrumentation entry point; sets up stack-based callback to
     * #OnRoutineExit, then passes control to #OnRoutineEntry for
     * the provided instance #self.
     */
    static void
    InstrumentationEntry
    (
        void    * self,
        ADDRINT * arg0,
        ADDRINT * arg1,
        ADDRINT * arg2,
        ADDRINT * arg3
    );


    /**
     * Function invoked when the routine is entered
     *
     * @param arg0 First argument passed to the hooked routine
     * @param arg1 Second argument passed to the hooked routine
     * @param arg2 Third argument passed to the hooked routine
     * @param arg3 Fourth argument passed to the hooked routine
     */
    virtual void
    OnRoutineEntry
    (
        ADDRINT * arg0,
        ADDRINT * arg1,
        ADDRINT * arg2,
        ADDRINT * arg3
    ) = 0;


    /**
     * Function invoked when the routine is exited
     *
     * @param ReturnValue
     * Pointer to the value the routine will return.  If modified, the return value
     * is updated accordingly by Pin (see IARG_FUNCRET_EXITPOINT_REFERENCE).
     */
    virtual void
    OnRoutineExit
    (
        ADDRINT * ReturnValue
    ) = 0;


};



/**
 * Implementation of #RoutineHook which toggles #Logging for the current thread
 * on routine entry/exit.
 */
class LogEnablingRoutineHook : public RoutineHook
{
public:
    void
    OnRoutineEntry
    (
        ADDRINT *arg0,
        ADDRINT * arg1,
        ADDRINT * arg2,
        ADDRINT * arg3
    );


    void
    OnRoutineExit
    (
        ADDRINT * ReturnValue
    );


};


///**
// * Helper structure which automatically loads a DLL, resolves a named
// * routine, and registers the hook for that routine's address.
// */
//class SymbolicHooker : public ImageInstrumenter
//{
//public:
//    SymbolicHooker
//    (
//        RoutineHook * H,
//        string      Lib,
//        string      Routine
//    );
//
//    void
//    Instrument(IMG img);
//
//
//private:
//    // CRT initialization order non-determinism means that this
//    // *must* be a pointer to a vector, not just a static vector.
////    static vector<SymbolicHooker *> *Instances;
//
//    RoutineHook *Hook;
//    string LibName;
//    string RoutineName;
//};
