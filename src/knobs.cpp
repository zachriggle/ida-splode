/*
 * knobs.cpp
 *
 *  Created on: Oct 9, 2013
 */


#include "common.h"

//KNOB<string>    KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "python", "splode.py", "specify Python trace importer");
KNOB<string>    KnobModuleWhitelist(KNOB_MODE_APPEND, "pintool", "m", "test.exe", "module whitelist");
KNOB<string>    KnobLoggedRoutines(KNOB_MODE_APPEND, "pintool", "r", "main", "routine filter");
KNOB<string>    KnobMongoHost(KNOB_MODE_OVERWRITE, "pintool", "host", "localhost", "mongodb host");
KNOB<string>    KnobMongoPort(KNOB_MODE_OVERWRITE, "pintool", "port", "27017", "mongodb port");

KNOB<BOOL>      KnobCheckWhitelistSymbols(KNOB_MODE_OVERWRITE, "pintool", "whitelisted_symbols", "0",
                                          "resolve whitelisted modules");
KNOB<BOOL>      KnobCheckText(KNOB_MODE_OVERWRITE, "pintool", "text", "0", "check for text on memory access");
KNOB<BOOL>      KnobCheckSymbols(KNOB_MODE_OVERWRITE, "pintool", "symbols", "1", "resolve symbols");
KNOB<BOOL>      KnobCheckHeap(KNOB_MODE_OVERWRITE, "pintool", "heap", "1", "resolve heap allocations");
KNOB<BOOL>      KnobCheckStack(KNOB_MODE_OVERWRITE, "pintool", "stack", "1", "resolve stack addresses");
KNOB<BOOL>      KnobInstrumentMov(KNOB_MODE_OVERWRITE, "pintool", "mov", "1", "instrument mov mem instructions");
KNOB<BOOL>      KnobInstrumentIndirect(KNOB_MODE_OVERWRITE, "pintool", "call", "1",
                                       "instrument indirect call/jmp instructions");
KNOB<BOOL>      KnobInstrumentImmediate(KNOB_MODE_OVERWRITE, "pintool", "imm", "1", "instrument mov imm instructions");
