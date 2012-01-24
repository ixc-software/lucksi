#include "stdafx.h"

#include "iLog/LogBinaryUtils.h"
#include "iLog/LogWrapperLoggers.h"
#include "SbpPackInfo.h"

namespace SBProto
{
    void LogSbpProtoMonitor(iLogW::LogSession &log,
		const SBProto::SbpPackInfo &data, int countPar, int binaryMaxBytes)
    {
        if(data[0].Type().TypeID == SBProto::TypeString)
        {
            log << data[0].AsString();
        }
	
		if(countPar == 0 || data.Count() < countPar) countPar = data.Count();

        for(int i = 1; i < countPar; ++i)
        {
            SBProto::TypeInfoDesc typeInfo = data[i].Type().TypeID;

            if (typeInfo == SBProto::TypeBinary) 
            {
                log << iLogW::LogBinaryShort(data[i].DataPtr(), data[i].Length(), binaryMaxBytes);
                continue;
            }

            if (typeInfo == SBProto::TypeString) 
            {
                log << " \"" << data[i].Value() << "\""; 
                continue;
            }
            log << " " << data[i].Value();  
        }
    }

};


