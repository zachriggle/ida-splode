/*
 * log-memory.cpp
 *
 *  Created on: Dec 19, 2013
 */

#include "common.h"


void
PIN_FAST_ANALYSIS_CALL
MemoryLogger::Instrumentation
(
    ADDRINT IsRead,
    ADDRINT IP,
    ADDRINT Address,
    ADDRINT Value,
    UINT32  Size
)
{
    if (Thread::Get()->ShouldLog())
    {
        //
        // If read operation would throw an exception, don't dump the info
        //
        if (IsRead && Size != PIN_SafeCopy(&Value, (void *) Address, Size))
        {
            Value = 0;
        }

        ScopedLock LogLock(&Traces.Lock);
        Traces  << "{ \"IP\": " << IP
                << ", \"Counter\": " << ++Counter //ATOMIC::OPS::Increment<UINT32>(&Counter, 1)
                << ", \"Type\": "    << (IsRead ? "\"Read\"" : "\"Write\"")
                << ", \"Address\": " << AddressMetadata(Address, sizeof(ADDRINT))
                << ", \"Value\": "   << AddressMetadata(Value, Size)
                << "}\n";
    }
}



void
PIN_FAST_ANALYSIS_CALL
ShittyMemoryReadLogger::Instrumentation
(
    ADDRINT IP,
    ADDRINT Base,
    ADDRINT Index,
    ADDRINT Scale,
    ADDRINT Disp,
    ADDRINT Size
)
{
    MemoryLogger::Instrumentation(true,                             // ADDRINT IsRead,
                                  IP,                               // ADDRINT IP,
                                  Base + Disp + (Scale * Index),    // ADDRINT Address,
                                  0,                                // ADDRINT Value,
                                  Size);                            // UINT32 Size
}



void
MemoryReadLogger::Instrument
(
    INS ins
)
{
    // Only instrument MOV and MOVZX
    OPCODE op = INS_Opcode(ins);

    if (!(XED_ICLASS_MOV == op || XED_ICLASS_MOVZX == op))
    {
        return;
    }

    //
    // Instrument 'mov reg, [mem]' instructions to save the value of 'mem' and '[mem]'
    //
    // Specifically, we want to know [1] what was read and [2] where it came from.
    //
    if (  INS_IsMemoryRead(ins)
       && INS_OperandIsReg(ins, 0)
       && INS_OperandIsMemory(ins, 1))
    {
        INS_InsertPredicatedCall(ins,
                                 IPOINT_BEFORE,
                                 (AFUNPTR) Instrumentation,
                                 IARG_FAST_ANALYSIS_CALL,
                                 IARG_ADDRINT, 1,           // ADDRINT IsRead,
                                 IARG_INST_PTR,             // ADDRINT IP,
                                 IARG_MEMORYREAD_EA,        // ADDRINT Address,
                                 IARG_ADDRINT, 0,           // ADDRINT Value,
                                 IARG_MEMORYREAD_SIZE,      // UINT32 Size
                                 IARG_END);
    }
}



void
MemoryWriteLogger::Instrument
(
    INS ins
)
{
    // Only instrument MOV and MOVZX
    OPCODE op = INS_Opcode(ins);

    if (!(XED_ICLASS_MOV == op || XED_ICLASS_MOVZX == op))
    {
        return;
    }

    //
    // Instrument 'mov [mem], reg' instructions to save the value of 'reg' and 'mem'
    //
    // Specifically, we want to know [1] what was written and [2] where it was written to.
    //
    if (  INS_IsMemoryWrite(ins)
       && INS_OperandIsMemory(ins, 0)
       && INS_OperandIsReg(ins, 1))
    {
        INS_InsertPredicatedCall(ins,
                                 IPOINT_BEFORE,
                                 (AFUNPTR) Instrumentation,
                                 IARG_FAST_ANALYSIS_CALL,
                                 IARG_ADDRINT, 0,                           // ADDRINT IsRead,
                                 IARG_INST_PTR,                             // ADDRINT IP,
                                 IARG_MEMORYWRITE_EA,                       // ADDRINT Address,
                                 IARG_REG_VALUE, INS_OperandReg(ins, 1),    // ADDRINT Value,
                                 IARG_MEMORYWRITE_SIZE,                     // UINT32 Size
                                 IARG_END);
    }
}



void
MemoryWriteImmLogger::Instrument
(
    INS ins
)
{
    // Only instrument MOV and MOVZX
    OPCODE op = INS_Opcode(ins);

    if (!(XED_ICLASS_MOV == op || XED_ICLASS_MOVZX == op))
    {
        return;
    }


    if (  INS_IsMemoryWrite(ins)
       && INS_OperandIsMemory(ins, 0)
       && INS_OperandIsImmediate(ins, 1))
    {
        ADDRINT Imm = (ADDRINT) INS_OperandImmediate(ins, 1);
        INS_InsertPredicatedCall(ins,
                                 IPOINT_BEFORE,
                                 (AFUNPTR) Instrumentation,
                                 IARG_FAST_ANALYSIS_CALL,
                                 IARG_ADDRINT, 0,           // ADDRINT IsRead,
                                 IARG_INST_PTR,             // ADDRINT IP,
                                 IARG_MEMORYWRITE_EA,       // ADDRINT Address,
                                 IARG_ADDRINT, Imm,         // ADDRINT Value,
                                 IARG_MEMORYWRITE_SIZE,     // UINT32 Size
                                 IARG_END);
    }
}



void
MemoryLeaLogger::Instrument
(
    INS ins
)
{
    // Only instrument LEA instructions
    if (INS_Opcode(ins) != XED_ICLASS_LEA)
    {
        return;
    }

    //
    // LEA needs special treatment.  Per the documentation for
    // INS_OperandIsMemory, LEA operands are *not* included.
    // You also cannot use IARG_MEMORYOP_EA or IARG_MEMORYREAD_EA
    // on this type of instruction.
    //
    // Instead, we instrument after the value has been written
    // to the register.
    //
    // For reference, LEA instructions have information in the following:
    // - INS_MemoryDisplacement
    // - INS_MemoryBaseReg
    // - INS_MemoryIndexReg
    // - INS_MemoryScale
    //
    for (size_t i = 0; i < INS_OperandCount(ins); i++)
    {
        if (INS_OperandIsAddressGenerator(ins, i))
        {
            REG     BaseReg         = INS_MemoryBaseReg(ins);
            REG     IndxReg         = INS_MemoryIndexReg(ins);

            ADDRINT IARG_BASE_TYPE  = REG_valid(BaseReg) ? IARG_REG_VALUE : IARG_ADDRINT;
            ADDRINT IARG_INDX_TYPE  = REG_valid(IndxReg) ? IARG_REG_VALUE : IARG_ADDRINT;

            INS_InsertPredicatedCall(ins,
                                     IPOINT_BEFORE,
                                     (AFUNPTR) Instrumentation,
                                     IARG_FAST_ANALYSIS_CALL,
                                     IARG_INST_PTR,                                         //   ADDRINT IP,
                                     IARG_BASE_TYPE, (REG_valid(BaseReg) ? BaseReg : 0),    //   ADDRINT Base,
                                     IARG_INDX_TYPE, (REG_valid(IndxReg) ? IndxReg : 0),    //   ADDRINT Index,
                                     IARG_ADDRINT, INS_MemoryScale(ins),                    //   ADDRINT Scale,
                                     IARG_ADDRINT, INS_MemoryDisplacement(ins),             //   ADDRINT Disp,
                                     IARG_ADDRINT, INS_OperandWidth(ins, i) / 8,            //   ADDRINT Size
                                     IARG_END);
        }
    }
}



void
MemoryCmpLogger::Instrument
(
    INS ins
)
{
    // Only instrument CMP instructions
    if (INS_Opcode(ins) != XED_ICLASS_CMP)
    {
        return;
    }

    //
    // Note for CMP instructions:
    // PIN considers the EFLAGS register to be the last/third operand.
    //
    // Also, PIN does not actually support IARG_MEMORYOP_EA or IARG_MEMORYREAD_EA,
    // even though it clearly identifies the operand as a memory read.
    //
    for (size_t i = 0; i < INS_OperandCount(ins); i++)
    {
        if (INS_OperandIsMemory(ins, i) && INS_MemoryOperandIsRead(ins, i))
        {
            REG     BaseReg         = INS_MemoryBaseReg(ins);
            REG     IndxReg         = INS_MemoryIndexReg(ins);

            ADDRINT IARG_BASE_TYPE  = REG_valid(BaseReg) ? IARG_REG_VALUE : IARG_ADDRINT;
            ADDRINT IARG_INDX_TYPE  = REG_valid(IndxReg) ? IARG_REG_VALUE : IARG_ADDRINT;

            INS_InsertPredicatedCall(ins,
                                     IPOINT_BEFORE,
                                     (AFUNPTR) Instrumentation,
                                     IARG_FAST_ANALYSIS_CALL,
                                     IARG_INST_PTR,                                         //   ADDRINT IP,
                                     IARG_BASE_TYPE, (REG_valid(BaseReg) ? BaseReg : 0),    //   ADDRINT Base,
                                     IARG_INDX_TYPE, (REG_valid(IndxReg) ? IndxReg : 0),    //   ADDRINT Index,
                                     IARG_ADDRINT, INS_MemoryScale(ins),                    //   ADDRINT Scale,
                                     IARG_ADDRINT, INS_MemoryDisplacement(ins),             //   ADDRINT Disp,
                                     IARG_ADDRINT, INS_OperandWidth(ins, i) / 8,            //   ADDRINT Size
                                     IARG_END);
        }
    }
}
