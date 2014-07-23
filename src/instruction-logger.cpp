
/*
 * instruction-logger.cpp
 *
 *  Created on: Jan 2, 2014
 */

#include "common.h"

void InstructionLogger::Instrument(INS ins)
{
    ScopedLock LogLock(&Traces.Lock);
    ADDRINT Address = INS_Address(ins);
    Traces  << "{ \"IP\": " << Address
            << ", \"Counter\": " << ++Counter // ATOMIC::OPS::Increment<UINT32>(&Counter, 1)
            << ", \"Type\": \"Seen\""
            << ", \"Bytes\": [";

    for(size_t i = 0; i < INS_Size(ins); i++)
    {
        UINT8 *b = ((UINT8*) Address) + i;
        Traces << "\"" << (unsigned int)(*b) << "\",";
    }

    Traces << "]}\n";

}
