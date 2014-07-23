#pragma once
/*
 * image-whitelister.h
 *
 *  Created on: Dec 30, 2013
 */

#include "common.h"

/**
 * Mixin which whitelists any image that it instruments
 */
class ImageWhitelisterMixin : public virtual Instrumenter<IMG>
{
public:
    ImageWhitelisterMixin();

    virtual void
    Instrument(IMG img);
};

/**
 * Mixin which instruments the main executable
 */
class MainExecutableImageInstrumenter : public virtual Instrumenter<IMG>
{
public:
    virtual bool
    ShouldInstrument(IMG img);
};

/**
 * Adds images to the whitelist if they match the provided string.
 *
 * Modules are whitelisted if any portion of their path matches any whitelist
 * string, and are compared in a case-insensitive manner.
 */
class NamedImageWhitelister : public virtual NamedImageInstrumenter,
                              public virtual ImageWhitelisterMixin
{
public:
    NamedImageWhitelister
    (
        string Name
    );
//
// MSVC is overly verbose when overriding a the base class'
// virtual method Instrument::ShouldInstrument via virtual inheritance
// through NamedImageInstrumenter.
//
#pragma warning(suppress: 4250)
};

/**
 * Adds image to the whitelist if it's the main target binary.
 */

class MainExecutableImageWhitelister : public virtual MainExecutableImageInstrumenter,
                                       public virtual ImageWhitelisterMixin
{
//
// MSVC is overly verbose when overriding a the base class'
// virtual method Instrument::ShouldInstrument via virtual inheritance
// through MainExecutableImageInstrumenter.
//
#pragma warning(suppress: 4250) // dominance is desired
};


