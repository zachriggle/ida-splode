/*
 * image-symbol-loader.cpp
 *
 *  Created on: Dec 18, 2013
 */

#include "common.h"

ImageSymbolLoader::ImageSymbolLoader
(
)   : SortableMixin(-1)
{}



void
ImageSymbolLoader::Instrument
(
    IMG img
)
{
    string  ImageName       = IMG_Name(img);
    string  ImageNameExt    = Basename(ImageName);
    ADDRINT Base            = IMG_StartAddress(img);
    MemRange Module(Base, IMG_SizeMapped(img));

    Symbol::RefreshSymbolByIMG(img);

    cerr << "[pintool] Module " << MemRangeToString(Module) << " "
         << ImageNameExt << " "
         << Symbol::SymbolInfoForModuleAtAddress(Base)
         << endl;
}



bool
ImageSymbolLoader::ShouldInstrument
(
    IMG img
)
{
    return true;
}
