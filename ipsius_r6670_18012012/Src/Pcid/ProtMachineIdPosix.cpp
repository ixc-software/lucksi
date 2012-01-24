#include "stdafx.h"

#include "ProtMachineId.h"
#include "DmiDecodeWrap.h"


namespace Pcid
{
    bool GetMachineID(std::string &info)
    {
		QProcess dmiDecode;
		dmiDecode.start("dmidecode", QIODevice::ReadOnly);
		if(!dmiDecode.waitForFinished()) return false;
		
        QTextStream stream(&dmiDecode);
        return DmiDecodeWrap::GetMachineID(stream, info);
    }
}  // namespace Pcid
