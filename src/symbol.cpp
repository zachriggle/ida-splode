#include "symbol.h"

PIN_LOCK WindbgLock;

///**
// * An automated mutex structure for holding a shared mutex; automatically
// * acquires the mutex on creation, releases it on destruction.
// *
// * Used to ensure that Windbg calls are not made concurrently.
// */
//struct WindbgMutex
//{
//    static HANDLE Mutex;
//
//    WindbgMutex
//    (
//    )
//    {
//        WaitForSingleObject(Mutex, INFINITE);
//    }
//
//
//
//    ~WindbgMutex
//    (
//    )
//    {
//        ReleaseMutex(Mutex);
//    }
//
//
//
//};
//
//HANDLE WindbgMutex::Mutex = CreateMutex(NULL, false, NULL);


ostream&
operator<<
(
    ostream       & os,
    const Symbol  & s
)
{
    return s.log(os);
}



//-------------------------------- SYMBOL::LOG ---------------------------------
ostream&
Symbol::log
(
    ostream& os
) const
{
    //
    // TODO See todo in ResolveSymbol about memoization of symbols.
    //
    typedef map<UINT64, char *> MemoizedSymbolName;
    static MemoizedSymbolName NameMemo;

    //
    // Attempt to find an *exact* memoized variant
    //
    auto i = NameMemo.find(sip.si.Address + Displacement);

    if (i != NameMemo.end())
    {
        os << (*i).second;
    }
    else
    {
        //
        // Implementation detail:
        //
        // 1. Symbol names may contain quotes, e.g. "`vftable'".  Use double quote
        // 2. This memory is **never** freed.
        //
        const char  * Format    = "[\"%s\", \"%s\", %#x]";
        size_t      Size        = 1 + _scprintf(Format, ModuleInfo.ModuleName, sip.si.Name, Displacement);
        char        *Name       = new char[Size];
        _snprintf_s(Name, Size, _TRUNCATE, Format, ModuleInfo.ModuleName, sip.si.Name, Displacement);

        //
        // Memoize the result and return
        //
        NameMemo[sip.si.Address] = Name;

        os << Name;
    }

    return os;
}



//------------------------ SYMBOL :: INITIALIZE SYMBOLS ------------------------
void
Symbol::InitializeSymbols
(
)
{
    ScopedLock  s(&WindbgLock);
    cerr << "[pintool] Initializing symbols..." << endl;

    UINT32      WantOptions = SymGetOptions() | SYMOPT_CASE_INSENSITIVE | SYMOPT_UNDNAME;
    UINT32      BadOptions  = 0; // SYMOPT_DEFERRED_LOADS;

    SymSetOptions(WantOptions & ~BadOptions);

    if (SymInitialize(GetCurrentProcess(),
                      NULL,
                      FALSE))
    {
        cerr << "[pintool] Symbol search path is " << GetSearchPath() << endl;
    }
    else
    {
        cerr    << "[pintool] Symbol initialization failed: " << GetLastError()
                << endl;
    }
}



//---------------------- SYMBOL :: REFRESH SYMBOL BY IMG -----------------------
void
Symbol::RefreshSymbolByIMG
(
    IMG img
)
{
    ScopedLock  s(&WindbgLock);

    PIN_LockClient();
    string      ImagePath   = IMG_Name(img);
    ADDRINT     Base        = IMG_StartAddress(img);
    PIN_UnlockClient();

    //
    // Contrary to the statements of the documentation, without the full
    // path *AND* base address of the module, WinDbg refuses to load symbols.
    //
    if (!SymLoadModule64(GetCurrentProcess(),
                         NULL,
                         ImagePath.c_str(),
                         NULL,
                         Base,
                         0))
    {
        cerr << "[pintool] Could not load symbol info for " << ImagePath << ": " << GetLastError() << endl;
    }
}



//------------------------------ SYMBOL :: SYMBOL ------------------------------
Symbol::Symbol
(
    UINT64 p
)
{
    Resolve(p);
}



Symbol::Symbol
(
    void * p
)
{
    Resolve((UINT64) p);
}



ADDRINT
Symbol::FromName
(
    string SymName
)
{
    ScopedLock s(&WindbgLock);
    SYMBOL_INFO_PACKAGE sip;
    memset(&sip, 0, sizeof(sip));

    sip.si.SizeOfStruct = sizeof(sip.si);
    sip.si.MaxNameLen   = sizeof(sip.name);

    if (!SymFromName(GetCurrentProcess(), SymName.c_str(), &sip.si))
    {
//        __debugbreak();
        cerr << "[pintool] Symbol resolution for " << SymName << " failed: " << GetLastError() << endl;
        return 0;
    }

    return (ADDRINT) sip.si.Address;
}



//-------------------------- SYMBOL :: RESOLVE SYMBOL --------------------------
Symbol *
Symbol::ResolveSymbol
(
    UINT64  SymAddr,
    bool    Force
)
{
    //    __debugbreak();

    //
    // TODO: Investigate whether the use of memoization actually has any real
    //       performance increase.  The idea being that resolving symbols will
    //       occur most frequently for a few specific symbol names, and usually
    //       when looking up return addresses for heap allocation stack traces.
    //
    //       Since we'd frequently be looking up the same thing, a binary search
    //       should be at least as fast as whatever Windbg uses internally
    //       (and much faster if it's a linear sweep through symbols)
    //
    //       Note that we also separately memoize the string representation of
    //       symbol names in Symbol::log.
    //
    static map<UINT64, Symbol *> SymbolMemo;

    //    if (!IsProbablyPointer(SymAddr))
    //    {
    //        return NULL;
    //    }

    //
    // Unless we want to resolve whitelisted symbols, we will only
    // resolve non-whitelisted symbols.
    //
    if (  IsAddressWhitelisted((ADDRINT) SymAddr)
       && !Force)
    {
            return NULL;
    }


    //
    // Attempt to use previously-resolved symbols
    //
    auto MemoizedResult = SymbolMemo.find(SymAddr);

    if (MemoizedResult != SymbolMemo.end())
    {
        return (*MemoizedResult).second;
    }

    Symbol *self = new Symbol();

    if (!self->Resolve(SymAddr))
    {
        delete self;
        self = NULL;
    }

    //
    // Memoize the result for future references, return the pointer.
    // The memory is never freed.
    //
    // Note that this also means the first failure to resolve an address
    // will add an entry to the memo.
    //
    SymbolMemo[SymAddr] = self;

    return self;
}



//----------------------------- SYMBOL :: RESOLVE ------------------------------
bool
Symbol::Resolve
(
    UINT64 SymAddr
)
{
    ScopedLock s(&WindbgLock);

    //
    // Initialize state
    //
    memset(this, 0, sizeof(*this));

    //
    // Don't attempt to resolve NULL; this is a special-case default
    // argument to the constructor.
    //
    if (SymAddr == 0)
    {
        return false;
    }

    //
    // Find the module information first.  Worst case scenario, we can do
    // module+displacement.
    //
    ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);
    sip.si.SizeOfStruct     = sizeof(sip.si);
    sip.si.MaxNameLen       = sizeof(sip.name);

    if (!SymGetModuleInfo64(GetCurrentProcess(), SymAddr, &ModuleInfo))
    {
        return false;
    }


    //
    // Attempt to resolve an actual symbol
    //
    if (!SymFromAddr(GetCurrentProcess(), SymAddr, &Displacement, &sip.si))
    {
        //
        // We have succeeded in at least finding module information.
        // Calculate a displacement within the module as a worst-case scenario.
        //
        Displacement = SymAddr - ModuleInfo.BaseOfImage;
    }

//    __debugbreak();


    return true;
}



//---------------------------- SYMBOL :: ENUMERATE -----------------------------

int __stdcall
MySymEnumSymbolsProc
(
    SYMBOL_INFO    * pSymInfo,
    UINT                SymbolSize,
    void                * UserContext
);


void
Symbol::Enumerate
(
    UINT64 BaseAddress
)
{
    ScopedLock s(&WindbgLock);
    cerr << "[pintool] Enumerating symbols at " << BaseAddress << endl;
#pragma warning(suppress:4996)

    if (!SymEnumSymbols(GetCurrentProcess(), BaseAddress, NULL,
                        (PSYM_ENUMERATESYMBOLS_CALLBACK) MySymEnumSymbolsProc, NULL))
    {
        cerr << "[pintool] SymEnumerateSymbols failed: " << GetLastError() << endl;
    }
}



int __stdcall
MySymEnumSymbolsProc
(
    SYMBOL_INFO    * pSymInfo,
    UINT                SymbolSize,
    void                * UserContext
)
{
    cout << pSymInfo->Address << ", " << pSymInfo->Name << endl;
    return 1;
}



//---------------- SYMBOL :: SYMBOL INFO FOR MODULE AT ADDRESS -----------------
string
Symbol::SymbolInfoForModuleAtAddress
(
    UINT64 ModuleBaseAddress
)
{
    ScopedLock  s(&WindbgLock);
    string      SymType;
    IMAGEHLP_MODULE64 ModuleInfo;
    size_t      NumberOfSymbols = GetNumberOfSymbolsLoaded(ModuleBaseAddress);

    //
    // Load module info
    //
    memset(&ModuleInfo, 0, sizeof(ModuleInfo));
    ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);

    if (!SymGetModuleInfo64(GetCurrentProcess(), ModuleBaseAddress, &ModuleInfo))
    {
        return "(error: " + decstr((UINT32)GetLastError()) + ")";
    }

    //
    // Dump info
    //
    static const char * SymbolTypeNames[] = {
        "",         // SymNone = 0,
        "coff",     // SymCoff,
        "cv",       // SymCv,
        ".pdb",     // SymPdb,
        "export",   // SymExport,
        "deferred", // SymDeferred,
        ".sym",     // SymSym,       // .sym file
        ".dia",     // SymDia,
        "virtual",  // SymVirtual,
        ""          // NumSymTypes
    };

    return "(" + decstr(NumberOfSymbols) + " " + SymbolTypeNames[ModuleInfo.SymType] + " symbols)";
}



//------------------------- SYMBOL :: GET SEARCH PATH --------------------------
const size_t SymbolPathBufferSize           = MAX_PATH * 10;
char SymbolPathBuffer[SymbolPathBufferSize] = {0};

char *
Symbol::GetSearchPath
(
)
{
    if (SymGetSearchPath(GetCurrentProcess(), SymbolPathBuffer, SymbolPathBufferSize - 1))
    {
        SymbolPathBuffer[SymbolPathBufferSize - 1] = '\0';
    }

    return SymbolPathBuffer;
}



//------------------- SYMBOL :: GET NUMBER OF SYMBOLS LOADED -------------------
int __stdcall
EnumSymbolsCallback
(
    SYMBOL_INFO    * pSymInfo,
    UINT                SymbolSize,
    void                * UserContext
)
{
    size_t *NumSymbols = (size_t *) UserContext;
    (*NumSymbols)++;
    return 1;
}



size_t
Symbol::GetNumberOfSymbolsLoaded
(
    UINT64 ModuleBaseAddress
)
{
    size_t NumSymbols = 0;

    if (!SymEnumSymbols(GetCurrentProcess(),
                        ModuleBaseAddress,
                        NULL,
                        (PSYM_ENUMERATESYMBOLS_CALLBACK) EnumSymbolsCallback,
                        &NumSymbols))
    {
        cerr    << "[pintool] Could not force-load symbols for " << ModuleBaseAddress << ": " << GetLastError()
                << endl;
    }

    return NumSymbols;
}

string GetSymModule(string s)
{
    return s.substr(0, s.find('!'));
}

string GetSymName(string s)
{
    return s.substr(s.find('!')+1);
}
