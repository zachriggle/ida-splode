#include <ctype.h>
#include "Common.h"

const static size_t PROBE_LENGTH = 7;

inline bool
IsNotAsciiPrintableCharacter
(
    int c
)
{
    return (c <= ' ' || c > '~');
}



template<typename T>
bool
IsProbablyText
(
    T       * t,
    size_t  bytes
)
{
    size_t want = sizeof(T) * PROBE_LENGTH;

    if (bytes < want)
    {
        return false;
    }

    for (size_t i = 0; i < PROBE_LENGTH; i++)
    {
        if (IsNotAsciiPrintableCharacter(t[i]))
        {
            return false;
        }
    }

    return true;
}



bool
IsProbablyAscii
(
    void    * p,
    size_t  bytes
)
{
    return IsProbablyText((char *) p, bytes);
}



bool
IsProbablyUTF16
(
    void    * p,
    size_t  bytes
)
{
    return IsProbablyText((wchar_t *) p, bytes);
}



bool
IsProbablyPointer
(
    ADDRINT Address
)
{
    //    XXX: Check performance vs
    //    return PIN_CheckReadAccess((void*) Address)
    return (0x10000 < Address && Address < 0x80000000);
}
