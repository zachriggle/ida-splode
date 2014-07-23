#pragma once
/*
 * stringutil.h
 *
 *  Created on: Dec 31, 2013
 */

#include "Common.h"
#include <algorithm>

inline
string
Basename
(
    string& in
)
{
    return in.substr(in.find_last_of("\\/") + 1);
}



inline
string
StripExt
(
    string& in
)
{
    return in.substr(in.find_last_of(".") + 1);
}



inline
string
Lowercase
(
    string in
)
{
    string out(in);
    transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}



using::Basename;
