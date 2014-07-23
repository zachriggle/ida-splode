#pragma once
/*
 * stack-instrumenter.h
 *
 *  Created on: Dec 18, 2013
 */

#include "common.h"

/**
 * Instruments instructions to enable stack-based tracking.
 */
class StackInstrumenter : public InstructionInstrumenter
{
    void
    Instrument
    (
        INS ins
    );


    bool
    ShouldInstrument
    (
        INS ins
    );
};
