
/*
 * md5.cpp
 *
 *  Created on: Oct 15, 2013
 */
#include "md5.h"


//
// Code stolen from the least crappy implementation I could find.
// https://github.com/WPO-Foundation/webpagetest/blob/222887e063755e685d1c9a5297f0825a5cbae572/agent/wptdriver/util.cc#L468
//
namespace WIN
{
#include <Wincrypt.h>
string
HashFileMD5
(
    string file
)
{
    string      hash_result;
    HCRYPTPROV  crypto = 0;

    if (CryptAcquireContext(&crypto, NULL, NULL, PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT))
    {
        CHAR    file_hash[100];
        BYTE    buff[4096];
        DWORD   bytes           = 0;
        HCRYPTHASH crypto_hash  = 0;

        if (CryptCreateHash(crypto, CALG_MD5, 0, 0, &crypto_hash))
        {
            HANDLE file_handle = CreateFileA(file.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                             0, OPEN_EXISTING, 0, 0);

            if (file_handle != INVALID_HANDLE_VALUE)
            {
                bool ok = true;

                while (ReadFile(file_handle, buff, sizeof(buff), &bytes, 0) && bytes)
                {
                    if (!CryptHashData(crypto_hash, buff, bytes, 0))
                    {
                        ok = false;
                    }
                }

                if (ok)
                {
                    BYTE    hash[16];
                    DWORD   len = 16;

                    if (CryptGetHashParam(crypto_hash, HP_HASHVAL, hash, &len, 0))
                    {
                        sprintf(file_hash, "%02X%02X%02X%02X%02X%02X%02X%02X"
                                           "%02X%02X%02X%02X%02X%02X%02X%02X",
                                hash[0], hash[1], hash[2], hash[3], hash[4], hash[5],
                                hash[6], hash[7], hash[8], hash[9], hash[10], hash[11],
                                hash[12], hash[13], hash[14], hash[15]);
                        hash_result = file_hash;
                    }
                }

                CloseHandle(file_handle);
            }

            CryptDestroyHash(crypto_hash);
        }

        CryptReleaseContext(crypto, 0);
    }

    return hash_result;
}



}


/**
 * @return Hex string for the MD5 of the specified file.
 */
string
GetMd5
(
    string FullPathToFile
)
{
    return WIN::HashFileMD5(FullPathToFile);
}
