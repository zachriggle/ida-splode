#pragma once
/*
 * module-logger.h
 *
 *  Created on: Dec 18, 2013
 */

#include "common.h"

/**
 * Logs information on each module as they are loaded.
 */
class ImageLogger : public ImageInstrumenter
{
public:
    void
    Instrument
    (
        IMG img
    );

    bool ShouldInstrument(IMG img);
};
