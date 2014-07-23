#pragma once
/*
 * timearray.h
 *
 *  Created on: Dec 26, 2013
 */

#include "common.h"

struct
TimeArray
{
    time_t t;
    TimeArray
    (
        time_t x
    ) : t(x) {}



    TimeArray
    (
    ) : t(time(NULL)){}



};

inline
ostream&
operator<<
(
    ostream           & os,
    const TimeArray   & s
)
{
    tm * timeinfo = localtime(&s.t);

    os  << "[" << decstr(1900 + timeinfo->tm_year)
        << "," << decstr(1 + timeinfo->tm_mon)
        << "," << decstr(timeinfo->tm_mday)
        << "," << decstr(timeinfo->tm_hour)
        << "," << decstr(timeinfo->tm_min)
        << "," << decstr(timeinfo->tm_sec)
        << "]";
    return os;
}
