#include "stdafx.h"

#include <windows.h>

#include "ProtMachineId.h"

#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"

#include "Utils/CryptUtils.h"

namespace Pcid
{
    
    bool GetMachineID(std::string &info)
    {
        const int CBuffSize = 256;

        // get windows path
        std::wstring dir;
        {
            wchar_t buff[CBuffSize];
            int len = GetWindowsDirectory(buff, CBuffSize);
            if (len > CBuffSize) return false;
            dir += buff;
        }



        // get volume info
        DWORD volumeID;
        {
            wchar_t volume[CBuffSize];
            wchar_t fsName[CBuffSize];
            DWORD componentLength, systemFlags;

			
            bool ok = GetVolumeInformation(
                dir.substr(0, 3).c_str(),
                volume,
                CBuffSize,
                &volumeID,
                &componentLength,
                &systemFlags,
                fsName,
                CBuffSize
                );

            if (!ok) return false;
        }

        // volume id -> md5 -> base64
        std::string volumeIdHash;
        Utils::MD5::Digest(&volumeID, sizeof(volumeID), volumeIdHash);
        Utils::Base64::Encode(volumeIdHash, info);

        return true;
    }
    
}  // namespace Pcid
