#pragma once
/*
 * log-memory.h
 *
 *  Created on: Dec 19, 2013
 */

#include "common.h"

/**
 * Basic logger instrumenter/instrumentation for memory accesses.
 */
class MemoryLogger : public InstructionInstrumenter
{
protected:
    static
    VOID
    PIN_FAST_ANALYSIS_CALL
    Instrumentation
    (
        ADDRINT IsRead,
        ADDRINT IP,
        ADDRINT Address,
        ADDRINT Value,
        UINT32  Size
    );


};


/**
 * Specialization of #MemoryLogger to log memory reads; i.e. 'mov reg, [mem]'
 */
class MemoryReadLogger : public MemoryLogger
{
    void
    Instrument
    (
        INS ins
    );


};

/**
 * Specialization of #MemoryLogger to log memory writes; i.e. 'mov [mem], reg'
 */
class MemoryWriteLogger : public MemoryLogger
{
    void
    Instrument
    (
        INS ins
    );


};

/**
 * Specialization of #MemoryLogger to log memory immediate writes; i.e. 'mov [mem], 0x1234'
 */
class MemoryWriteImmLogger : public MemoryLogger
{
    void
    Instrument
    (
        INS ins
    );


};

/**
 * Specialization of #MemoryLogger to log memory reads for which Pin does
 * not have a simple mechanism to get at the address being read/read from
 * via IARG_MEMORYOP_EA or IARG_MEMORYREAD_EA.
 */
class ShittyMemoryReadLogger : public MemoryLogger
{
protected:
    static
    VOID
    PIN_FAST_ANALYSIS_CALL
    Instrumentation
    (
        ADDRINT IP,
        ADDRINT Base,
        ADDRINT Index,
        ADDRINT Scale,
        ADDRINT Disp,
        ADDRINT Size
    );


};

/**
 * Specialization of #ShittyMemoryReadLogger to log addresses loaded by 'lea' instructions
 */
class MemoryLeaLogger : public ShittyMemoryReadLogger
{
    void
    Instrument
    (
        INS ins
    );


};

/**
 * Specialization of #ShittyMemoryReadLogger to log addresses checked
 * via instructions that look like:
 *
 * - cmp [mem], reg
 * - cmp [mem], 0x1234
 * - cmp 0x1234, [mem]
 * - cmp reg,    [mem]
 */
class MemoryCmpLogger : public ShittyMemoryReadLogger
{
    void
    Instrument
    (
        INS ins
    );


};
