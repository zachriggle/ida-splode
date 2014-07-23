#pragma once
/*
 * named-image-instrumenter.h
 *
 *  Created on: Dec 30, 2013
 */

#include "common.h"

/**
 * IMG instrumenter which only acts on IMGs whose full path
 * contain the specified substring.
 */
class NamedImageInstrumenter : public virtual Instrumenter<IMG>
{
public:
    NamedImageInstrumenter
    (
        string Substring
    );

    virtual
    bool
    ShouldInstrument
    (
        IMG img
    );
private:
    string Needle;
};
