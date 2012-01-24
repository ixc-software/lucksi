#ifndef BOARDHARDCODEDCONFIG_H
#define BOARDHARDCODEDCONFIG_H

#include "BfBootCore/GeneralBooterConst.h"
#include "Platform/PlatformTypes.h"

namespace BfBootSrv
{
    // Параметры трассировки по умолчанию.
//     const std::string CUdpLogHost = "192.168.0.144";
//     const Platform::word CUdpLogPort = 56001;
    const bool CUseTimestamp = true;
    const Platform::word CUdpContToStore = 1;

    // Сетевые настройки используемые при отсутствии конфигурации в регионе CfgDefault
    const bool CUseDefaultNetworkSettings = true;
    const std::string CIp = "192.168.0.143";        
    const std::string CGateway = "192.168.0.35"; 
    const std::string CMask = "255.255.255.0";
    const std::string CMac = "76.42.43.56.43.09";   

    //const int CCbpPort = BfBootCore::CCbpPort;

} // namespace BfBootSrv

#endif
