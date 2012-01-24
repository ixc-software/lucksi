
#ifndef __MFUTILS__
#define __MFUTILS__

// MfUtils.h

#include "Platform/PlatformTypes.h"

namespace MiniFlasherUtils
{
    using Platform::dword;
    using Platform::int64;
    
    QString NumberFormat(int64 number);

    QString OperationHead(const QString &opName, int comPort, int baudRate,
                          const QString &fileName = "", dword totalBytes = 0);
    
    QString OperationInfo(const QString &opName, dword totalBytes, dword opTimeMs);
    QString OperationInfo(const QString &opName, dword opTimeMs);    
    
} // namespace MiniFlasherUtils

#endif
