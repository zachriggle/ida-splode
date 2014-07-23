#pragma once
/*
 * vtable-trace.h
 *
 *  Created on: Oct 10, 2013
 */


//******************************************************************************
//                               UTILITY ROUTINES
//******************************************************************************


/** Prints out the usage */
INT32
Usage
(
);


/** Logs the current time */
void
LogCurrentTime
(
);


//******************************************************************************
//                           INSTRUMENTATION ROUTINES
//******************************************************************************

/** Invoked within the target application when it starts */
void
OnApplicationStart
(
    void *v
);


/** Invoked when the target application exits */
void
OnFini
(
    INT32   code,
    VOID    *v
);
