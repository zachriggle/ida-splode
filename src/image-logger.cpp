
/*
 * image-logger.cpp
 *
 *  Created on: Dec 18, 2013
 */

#include <algorithm> // replace
#include "Common.h"

void
ImageLogger::Instrument
(
    IMG img
)
{
    ScopedLock  LogLock(&Modules.Lock);
    string      ImgName = IMG_Name(img);

    Modules << "{ \"Name\":\"" << Basename(ImgName) << "\""
            << ", \"Base\": " << IMG_StartAddress(img)
            << ", \"End\":  " << IMG_HighAddress(img)
            << ", \"Size\": " << IMG_SizeMapped(img)
            << ", \"MD5\": \"" << GetMd5(IMG_Name(img)) << "\""
            << ", \"Sections\": {";

    //
    // Dump the address of each section into the database,
    // and replace items in the name where needed.
    //
    for (SEC s = IMG_SecHead(img); SEC_Valid(s); s = SEC_Next(s))
    {
        ADDRINT SecAddr = SEC_Address(s);
        string  SecName = SEC_Name(s);

        //
        // MongoDB does not accept periods in key names
        //
        replace(SecName.begin(), SecName.end(), '.', '_');

        Modules << "\"" << SecName << "\": " << SecAddr << ",";
    }

    Modules << "}}\n";
}

bool ImageLogger::ShouldInstrument(IMG img)
{
    // Log information for all modules.
    return true;
}
