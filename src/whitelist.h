#pragma once
/*
 * whitelist.h
 *
 *  Created on: Oct 6, 2013
 */

#include "Common.h"

/**
 * @return True if \a Address resides within a whitelisted module.
 */
bool
IsAddressWhitelisted
(
    ADDRINT Address
);


/**
 * Adds the specified image to the whitelist.
 */
void
AddWhitelistImage
(
    IMG img
);


/**
 * @return True if \a img is a whitelisted module.
 */
bool
IsImageWhitelisted
(
    IMG img
);
