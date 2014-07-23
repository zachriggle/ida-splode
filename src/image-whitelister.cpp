/*
 * image-whitelister.cpp
 *
 *  Created on: Dec 30, 2013
 */

#include "Common.h"
#include <algorithm>

ImageWhitelisterMixin::ImageWhitelisterMixin()
: SortableMixin(-2) {}

NamedImageWhitelister::NamedImageWhitelister
(
    string Name
)   : NamedImageInstrumenter(Name)
{}



void
ImageWhitelisterMixin::Instrument
(
    IMG img
)
{
    string Name = IMG_Name(img);
    cerr << "[pintool] Whitelisting " << Basename(Name) << endl;
    AddWhitelistImage(img);
}

bool MainExecutableImageInstrumenter::ShouldInstrument(IMG img)
{
    if(IMG_IsMainExecutable(img))
    {
        return true;
    }

    return false;
}
