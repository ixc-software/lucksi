#ifndef __PROTSERIAL__
#define __PROTSERIAL__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/RawCRC32.h"
#include "Utils/IntToString.h"

#include "ProtMagic.h"
#include "ProtMachineId.h"
#include "ProtVerifySerial.h"

namespace Pcid
{
    
    struct ProtSerial
    {
		static const char *CPcidFileName() { return "pcid.txt"; }
		static const char *CSerialFileName() { return "serial.txt"; }

        static std::string MakeSerial(const std::string &machineID);

		static bool CheckSerialFile();

        static void Test();
    };
    
}  // namespace Pcid

#endif
