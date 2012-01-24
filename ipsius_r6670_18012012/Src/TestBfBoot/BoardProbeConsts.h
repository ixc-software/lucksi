#ifndef BOARDPROBECONSTS_H
#define BOARDPROBECONSTS_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

/*
    Временный файл. 
    Параметры для проб реальной платы на PC.
*/

namespace TestBfBoot
{
    const std::string CBoardIP = "192.168.0.143";  // -- ?
    const std::string CBoardNetMask = "255.255.255.0";
    const std::string CBoardGateway = "192.168.0.35";
    const std::string CBoardPwd = "DefaultTestPwd";
    const Platform::dword CHwNumber = 0;
    const Platform::dword CHwType = 0;
    const std::string CMac = "00:e0:22:fe:55:80";
    //const int CBcListenPort = 2222; // udp-port listen by client    
} // namespace TestBfBoot

#endif
