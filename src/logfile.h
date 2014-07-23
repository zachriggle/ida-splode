#pragma once
/*
 * logfile.h
 *
 *  Created on: Dec 30, 2013
 */

#include "common.h"

/**
 * Wrapper around ofstream which flushes and closes the file when the
 * instrumented application exits.  This is used to ensure that the
 * log contents are flushed to file in the (likely ;) event of an app crash.
 *
 * Also forces a few flags on the file for speed and readability.
 */
class Logfile : public std::ofstream
{
public:
    // Lock which should be held before interacting with the log file,
    // if there is any risk of concurrency.
    PIN_LOCK Lock;
    string filename;

    void
    open
    (
        string s
    )
    {
        filename = s;
        ofstream::open(s, ios::binary | ios::trunc);
        PIN_InitLock(&Lock);
        PIN_AddFiniFunction(OnFini, this);
    }



    static
    void
    OnFini
    (
        INT32   Code,
        VOID    * Context
    )
    {
        Logfile *l = (Logfile *) Context;
        if (l)
        {
            cerr << "[pintool] Flushing " << l->filename << endl;
            l->flush();
            l->close();
        }
    }



};
