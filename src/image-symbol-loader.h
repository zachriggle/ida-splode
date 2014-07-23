#pragma once
/*
 * image-symbol-loader.h
 *
 *  Created on: Dec 18, 2013
 */

#include "common.h"

/**
 * Loads Windbg symbols for each module as they are loaded.
 */
class ImageSymbolLoader : public ImageInstrumenter
{
public:
    ImageSymbolLoader
    (
    );
private:
    bool
    ShouldInstrument
    (
        IMG img
    );


    void
    Instrument
    (
        IMG img
    );


};
