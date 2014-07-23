/*
 * named-image-instrumenter.cpp
 *
 *  Created on: Dec 30, 2013
 */

#include "common.h"
#include <algorithm>

NamedImageInstrumenter::NamedImageInstrumenter
(
    string Substring
)
{
    Needle = Lowercase(Substring);
}



bool
NamedImageInstrumenter::ShouldInstrument
(
    IMG img
)
{
    string ImgName = Lowercase(IMG_Name(img));

    // Any occurrences?  Instrument.
    return (ImgName.find(Needle) != string::npos);
}
