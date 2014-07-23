#pragma once

#include "Common.h"

/** Helper structure to resolve a symbol */
struct
Symbol
{

    /** Resolve a symbol for an address */
    Symbol
    (
        UINT64 SymAddr = 0
    );

    /** Resolve a symbol for an address */
    Symbol
    (
        void * SymAddr
    );

    /**
     * Resolve a symbol by name
     *
     * Note that SymName can be in the format of 'mod!sym', or just 'sym'
     */
    static
    ADDRINT
    FromName
    (
        string SymName
    );


    /** Log contents */
    ostream&
    log
    (
        ostream& os
    ) const;


    /** Initialized symbols for the process */
    static void
    InitializeSymbols
    (
    );


    /**
     * Refreshes a specific module's symbols, given its full path.
     *
     * @return Number of symbols loaded
     */
    static void
    RefreshSymbolByIMG
    (
        IMG img
    );


    /**
     * @return Number of symbols loaded for the specified image.
     */
    static size_t
    GetNumberOfSymbolsLoaded
    (
        UINT64 ModuleBaseAddress
    );


    /**
     * Returns the symbol path
     */
    static
    char *
    GetSearchPath
    (
    );


    /**
     * Returns a pointer to a resolved symbol, or NULL
     *
     * @param Address  Address to resolve
     * @param Force    Override 'Resolve Whitelist Symbols' knob
     * @return
     */
    static
    Symbol *
    ResolveSymbol
    (
        UINT64  Address,
        bool    Force = false
    );


    /** Enumerates symbols in the provided module to STDERR */
    static void
    Enumerate
    (
        UINT64 ModuleBaseAddress
    );


    /** Return information about the symbols loaded for the specified module */
    static string
    SymbolInfoForModuleAtAddress
    (
        UINT64 ModuleBaseAddress
    );


    IMAGEHLP_MODULE64   ModuleInfo; //!< Information about the module
    SYMBOL_INFO_PACKAGE sip;        //!< Information about the symbol
    UINT64 Displacement;            //!< Offset from the symbol, e.g. '0x34' in 'ntdll!VirtualAlloc+0x34'.

private:

    /** Internal routine which performs actual symbol resolution */
    bool
    Resolve
    (
        UINT64 SymAddr
    );


};

/**
 * Given a string in the format 'module!symbol', return 'module'.
 */
string GetSymModule(string ModuleBangSymbol);

/**
 * Given a string in the format 'module!symbol', return 'symbol'.
 */
string GetSymName(string ModuleBangSymbol);

/**
 * Dump a Symbol to a stream.
 */
ostream&
operator<<
(
    ostream       & os,
    const Symbol  & dt
);
