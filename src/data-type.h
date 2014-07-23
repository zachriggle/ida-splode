#pragma once

/**
 * @return True if the first few characters are ASCII-printable
 */
bool
IsProbablyAscii
(
    void    * p,
    size_t  bytes
);


/**
 * @return True if the first few characters are UNICODE-printable
 */
bool
IsProbablyUTF16
(
    void    * p,
    size_t  bytes
);


/**
 * @return True if the specified value is probably a pointer.
 */
bool
IsProbablyPointer
(
    ADDRINT Address
);
