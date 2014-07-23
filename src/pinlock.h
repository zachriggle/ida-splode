#pragma once
/*
 * spinlock.h
 *
 *  Created on: Oct 14, 2013
 */

#include "Common.h"

/**
 * Pin Lock helper to synchronize instrumentation so that logging doesn't step
 * all over itself during multithreaded operations.  Also provides an OO interface
 * so that we can use scoping to automatically acquire/release the lock.
 */
struct ScopedLock
{
    PIN_LOCK *Lock;
    ScopedLock
    (
        PIN_LOCK *RaiiLock
    ) : Lock(RaiiLock)
    {
        PIN_GetLock(Lock, 0);
    }



    ~ScopedLock
    (
    )
    {
        PIN_ReleaseLock(Lock);
    }



};
