#ifndef __LOGSBPPROTOMONITOR__
#define __LOGSBPPROTOMONITOR__

#include "SbpPackInfo.h"

namespace iLogW { class LogSession; };

namespace SBProto
{
    
    void LogSbpProtoMonitor(iLogW::LogSession &, 
		const SBProto::SbpPackInfo &data, int countPar, int binaryMaxBytes);
        
}  // namespace SBProto

#endif
