/*
 * page-heap.cpp
 *
 *  Created on: Dec 16, 2013
 */

#include "Common.h"

//MallocPageHeapRedirector    RedirectToMalloc(0, 1);
//FreePageHeapRedirector      RedirectToFree(0, 1);

//SymbolicHooker  hook_testexe_fmalloc(&RedirectToMalloc, "test.exe", "fmalloc");
//SymbolicHooker  hook_testexe_ffree(&RedirectToFree, "test.exe", "ffree");


PageHeapRedirector::PageHeapRedirector
(
)   : // Must instrument before anything else does, and ExecuteAt stops instrumentation
      SortableMixin(-1),
      ArgumentIndex(0),
      NumberOfStackArguments(0),
      TargetAddress(0)
{}



void
PageHeapRedirector::InstrumentationEntry
(
    PageHeapRedirector  * self,
    ADDRINT             ArgumentToPass,
    ADDRINT             ReturnAddress,
    CONTEXT             * Ctx
)
{
    // Get the current stack pointer
    ADDRINT *Esp = (ADDRINT *) PIN_GetContextReg(Ctx, REG_STACK_PTR);

    // Move the stack pointer back as needed
    Esp         += (self->NumberOfStackArguments - 1);

    // Write the return address and argument
    *(Esp + 0)  = ReturnAddress;
    *(Esp + 1)  = ArgumentToPass;

    // Resume execution
    PIN_SetContextReg(Ctx, REG_STACK_PTR, (ADDRINT) Esp);
    PIN_SetContextReg(Ctx, REG_INST_PTR, self->TargetAddress);
    PIN_ExecuteAt(Ctx);
}



void
PageHeapRedirector::Instrument
(
    INS ins
)
{
    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR) PageHeapRedirector::InstrumentationEntry, IARG_ADDRINT,
                             this, IARG_FUNCARG_ENTRYPOINT_VALUE, ArgumentIndex, IARG_RETURN_IP,
                             IARG_CONTEXT, IARG_END);
}



MallocPageHeapRedirector::MallocPageHeapRedirector
(
    ADDRINT ArgumentIndex,
    ADDRINT NumberOfStackArguments
)
{
    this->ArgumentIndex             = ArgumentIndex;
    this->NumberOfStackArguments    = NumberOfStackArguments;


    //
    // N.B. Cannot just use '&malloc' because pin tools are not linked
    //      against any CRT -- all implementations come from the PIN
    //      binary itself.
    //
    HINSTANCE msvcrt = LoadLibraryA("msvcrt.dll");
    this->TargetAddress             = (ADDRINT) GetProcAddress(msvcrt, "malloc");
}



FreePageHeapRedirector::FreePageHeapRedirector
(
    ADDRINT ArgumentIndex,
    ADDRINT NumberOfStackArguments
)
{
    this->ArgumentIndex             = ArgumentIndex;
    this->NumberOfStackArguments    = NumberOfStackArguments;

    //
    // N.B. See note above by 'malloc'
    //
    HINSTANCE msvcrt = LoadLibraryA("msvcrt.dll");
    this->TargetAddress             = (ADDRINT) GetProcAddress(msvcrt, "free");
}
