#pragma once
/*
 * symbolic-resolver.h
 *
 *  Created on: Dec 30, 2013
 */

#include "Common.h"

/**
 * Helper class which operates on an #AddressInstrumenter.
 * Automatically registers addresses for a named symbol
 * in the form of `Module!Symbol` when the specified module
 * is initially loaded.
 */
template<typename T>
class SymbolResolver : public NamedImageInstrumenter
{
public:
    SymbolResolver
    (
        AddressInstrumenter<T>  *Instance,
        string                  Symbol
    );

    void
    Instrument
    (
        IMG img
    );


private:
    AddressInstrumenter<T> *Instance;
    string ImageName;
    string SymbolName;
};


//---------------------- SYMBOLIC RESOLVER :: CONSTRUCTOR ----------------------
template<typename T>
SymbolResolver<T>::SymbolResolver
(
    AddressInstrumenter<T>  * Instance_,
    string                  Symbol_
)   : NamedImageInstrumenter(GetSymModule(Symbol_)),
      Instance(Instance_),
      ImageName(GetSymModule(Symbol_)),
      SymbolName(GetSymName(Symbol_))
{
    cerr << "[pintool] Looking for symbol " << ImageName << "!" << SymbolName << endl;
}



//---------------------- SYMBOLIC RESOLVER :: INSTRUMENT -----------------------
template<typename T>
void
SymbolResolver<T >::Instrument
(
    IMG img
)
{
    if(  ImageName.size() == 0
      && IMG_IsMainExecutable(img))
    {
        ImageName = IMG_Name(img);
    }

    string  ImageBasename       = Basename(ImageName);
    string  ImageBasenameNoExt  = ImageBasename.substr(0, ImageBasename.rfind('.'));
    string  FullSymbolName      = ImageBasenameNoExt + "!" + SymbolName;
    ADDRINT SymbolAddress       = Symbol::FromName(FullSymbolName);

    if (SymbolAddress)
    {
        Instance->AddrsToInstrument.insert(SymbolAddress);
        cerr << "[pintool] Hook    " << SymbolAddress << " " << FullSymbolName << endl;
    }
    else
    {
        cerr << "[pintool] Could not resolve hook addr for " << FullSymbolName << endl;
    }
}
