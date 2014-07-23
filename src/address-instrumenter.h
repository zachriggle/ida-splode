#pragma once
/*
 * address-instrumenter.h
 *
 *  Created on: Dec 25, 2013
 */

#include "common.h"

/**
 * Implements a simple address-based filter for an #Instrumenter.
 * Instead of the default behavior (instrument all whitelisted instructions),
 * this will instrument specific memory addresses.

 * Example use-case:
 *   malloc/calloc would need the same instrumentation,
 *   and the entry point to those routines is the only
 *   thing that needs the instrumentation.
 */
template<typename T>
class AddressInstrumenter : public virtual Instrumenter<T>
{
public:
    virtual
    ~AddressInstrumenter
    (
    ){}



    /**
     * Set of instructions which will be instrumented when #Instrument
     * is invoked.
     *
     * @see #Instrumenter::Instrument
     */
    set<ADDRINT> AddrsToInstrument;

    /**
     * Filter function to be overridden by the implementing class.
     *
     * While this logic could be encapsulated in #Instrument, by separating
     * it into a different virtual routine, we can have useful mixins.
     */
    virtual bool
    ShouldInstrument
    (
        T t
    );


};

template<typename T>
bool
AddressInstrumenter<T >::ShouldInstrument
(
    T t
)
{
    return (1 == AddrsToInstrument.count(GetAddress(t)));
}
