#pragma once
/*
 * instruction-logger.h
 *
 *  Created on: Jan 2, 2014
 */





#include "common.h"

/**
 * Logs information on each instruction as it is instrumented.
 */
class InstructionLogger : public InstructionInstrumenter
{
public:
    void
    Instrument
    (
        INS ins
    );
};
