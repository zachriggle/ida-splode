#pragma once
/*
 * whitelist-instrumenter.h
 *
 *  Created on: Dec 30, 2013
 */



/**
 * Similar to #AddressInstrumenter, only invokes #Instrument if-and-when
 * the provided
 */
template <typename T>
class WhitelistInstrumenter : public Instrumenter <T>
{
    Instrumenter(int Order_ = 0);
    virtual bool
    ShouldInstrument
    (
        T t
    );


};
