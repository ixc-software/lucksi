#ifndef __PROTVERIFYSERIAL__
#define __PROTVERIFYSERIAL__

#include "Utils/IntToString.h"
#include "Utils/RawCRC32.h"

#include "ProtMagic.h"

namespace Pcid
{

    struct VerifySerial
    {
        static bool Verify(const std::string &serial)
        {
            // serial to dword
            Platform::dword serialValue; 
            if (!Utils::HexStringToInt(serial, serialValue, false)) return false;

            // machineID
            std::string id;
            if(!GetMachineID(id)) return false;
            if (id.size() == 0) return false;

            Platform::dword idCrc = Utils::UpdateCRC32(&id[0], id.size());
            return (idCrc ^ serialValue) == CProtMagic;
        }
    };
    
}  // namespace Pcid

#endif
