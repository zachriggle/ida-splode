#include "Common.h"
#include <algorithm>
#include <intrin.h>

//----------------------------------- USAGE ------------------------------------
INT32
Usage
(
)
{
    cerr    << "This pin tool logs all indirect calls/branches, and memory operations\n"
            << KNOB_BASE::StringKnobSummary()
            << endl;
    return -1;
}



//------------------------------ LOG CURRENT TIME ------------------------------
//
// Aren't functors fun?
//
UINT64 StartTime = GetTickCount();


//---------------------------- ON APPLICATION START ----------------------------
void
OnApplicationStart
(
    void *v
)
{
    cerr << "[pintool] Starting program..." << endl;
}



//---------------------------------- ON FINI -----------------------------------
void
OnFini
(
    INT32   code,
    VOID    *v
)
{
    //
    // Time elapsed
    //
    UINT64  milliseconds    = GetTickCount() - StartTime;
    UINT64  seconds         = (UINT64) (milliseconds / 1000) % 60;
    UINT64  minutes         = (UINT64) ((milliseconds / (1000 * 60)) % 60);
    UINT64  hours           = (UINT64) ((milliseconds / (1000 * 60 * 60)) % 24);

    cerr    << std::dec
            << "[pintool] Time Elapsed: "
            << hours << " hours, "
            << minutes << " minutes, "
            << seconds << " seconds" << endl;
}



//------------------------------------ MAIN ------------------------------------
int
main
(
    int     argc,
    char    * argv[]
)
{
    cerr << showbase << hex;

    Symbol::InitializeSymbols();
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return -1;
    }

    //
    // Try to find the name of the main executable, if no modules
    // were specified in the whitelist.
    //
    string MainExecutable;

    for(int i = 0; i < argc; i++)
    {
        string arg(argv[i]);
        if(arg == "--" && i+1 < argc)
        {
            MainExecutable = string(argv[i+1]);
            MainExecutable = Basename(MainExecutable);
        }
    }


    //
    // If no white-list modules were specified, automatically white-list
    // the main executable.
    //
    if(0 == KnobModuleWhitelist.NumberOfValues())
    {
        KnobModuleWhitelist.AddValue(MainExecutable);
    }

    //
    // Automatically white-list matching modules as they are loaded.
    //
    for (size_t i = 0; i < KnobModuleWhitelist.NumberOfValues(); i++)
    {
        new NamedImageWhitelister(KnobModuleWhitelist.Value(i));
    }


    //
    // Log all modules as they are loaded, and all instructions as they
    // are instrumented.
    //
    new ImageLogger();
    new InstructionLogger();

    //
    // Instantiate instrumentation objects corresponding to various knobs
    //
    // -- Metadata providers
    KnobCheckText.Value() && new TextualMetadataProvider();
    KnobCheckHeap.Value() && new HeapAllocationMetadataProvider();
    KnobCheckStack.Value() && new StackFrameMetadataProvider();
    KnobCheckSymbols.Value() && new SymbolMetadataProvider();

    // -- Instrumenters
    KnobCheckStack.Value() && new StackInstrumenter();
    KnobCheckSymbols.Value() && new ImageSymbolLoader();
    KnobCheckWhitelistSymbols.Value() && 0; // TODO
    KnobInstrumentIndirect.Value() && new BranchLogger();
    KnobInstrumentImmediate.Value() && new MemoryWriteImmLogger();

    if (KnobInstrumentMov.Value())
    {
        new MemoryReadLogger();
        new MemoryWriteLogger();
        new MemoryLeaLogger();
        new MemoryCmpLogger();
    }

    // -- Routine hooks
    if(KnobCheckHeap.Value())
    {
        new SymbolResolver<INS>(new RtlAllocateHandleHook, "ntdll.dll!RtlAllocateHandle");
        new SymbolResolver<INS>(new RtlFreeHandleHook, "ntdll.dll!RtlFreeHandle");

        // -- Demo application allocator hooks
        new SymbolResolver<INS>(new MallocPageHeapRedirector(0,1), "demo.exe!custom_malloc");
    }


    //
    // If the hooked routine entry is specified as an empty string "",
    // then log everything that happens inside the whitelisted modules.
    //
    // Set up some Symbolic Hookers to enable logging when any of the
    // specified routines get hit.
    //
    for (size_t i = 0; i < KnobLoggedRoutines.NumberOfValues(); i++)
    {
        new SymbolResolver<INS>(new LogEnablingRoutineHook(),
                                KnobLoggedRoutines.Value(i));
    }

    //
    // MongoDB does not accept periods or spaces in database names
    //
    string  DatabaseName    = MainExecutable;
    replace(DatabaseName.begin(), DatabaseName.end(), '.', '_');
    replace(DatabaseName.begin(), DatabaseName.end(), ' ', '_');

    //
    // Write the Python script which will add all of the information
    // into MongoDB when run.
    //
    Script.open(MainExecutable + ".py");
    Script  << "#!/usr/bin/env python\n"
            << "# -*- coding: utf-8 -*-\n"
            << "# Indirect Trace Generated By Pin\n"
            << "import pymongoloader\n"
            << "# Symbol path: " << Symbol::GetSearchPath() << endl
            << "# Looking for routine " << KnobLoggedRoutines.Value() << endl
            << "# " << KnobCheckHeap << endl
            << "# " << KnobCheckSymbols << endl
            << "# " << KnobCheckText << endl
            << "# " << KnobCheckWhitelistSymbols << endl
            << "# " << KnobInstrumentIndirect << endl
            << "# " << KnobInstrumentMov << endl
            << "# " << KnobInstrumentImmediate << endl
            << "time_begin=" << TimeArray() << endl
            << "now      ='_'.join([str(i) for i in time_begin])\n"
            << "dbName   = 'IdaSplode_" << DatabaseName << "_' + now\n"
            << "if __name__ == '__main__':\n"
            << "  pymongoloader.ProcessFileIntoCollectition('" << MainExecutable << "' + '.modules.py', dbName, 'modules')\n"
            << "  pymongoloader.ProcessFileIntoCollectition('" << MainExecutable << "' + '.traces.py', dbName, 'traces')\n";
    Traces.open(MainExecutable + ".traces.py");
    Modules.open(MainExecutable + ".modules.py");

    //
    // XXX For some reason doing this inside of LogFile::open has no effect
    //
//    Traces << showbase << hex;
//    Modules << showbase << hex;
//    Script << showbase << hex;

    //
    // Set up instrumentation calls for instrumenter classes
    //
    IMG_AddInstrumentFunction(ImageInstrumenter::InstrumentationFunction, 0);
    INS_AddInstrumentFunction(InstructionInstrumenter::InstrumentationFunction, 0);
    TRACE_AddInstrumentFunction(TraceInstrumenter::InstrumentationFunction, 0);

    PIN_AddApplicationStartFunction(OnApplicationStart, 0);
    PIN_AddFiniFunction(OnFini, 0);

    PIN_AddThreadStartFunction(Thread::OnStart, NULL);
    PIN_AddThreadFiniFunction(Thread::OnFini, NULL);

    PIN_StartProgram();

    return 0;
}
