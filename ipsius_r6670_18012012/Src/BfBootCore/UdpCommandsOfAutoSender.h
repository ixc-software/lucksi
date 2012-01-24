#ifndef UDPRESPONSERELOAD_H
#define UDPRESPONSERELOAD_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "Utils/IBiRWStream.h"

namespace BfBootCore
{
    // возможный ответ на широковещательную рассылку из приложения
    struct CmdReload
    {        
        static void getBin(std::vector<Platform::byte>& bin);                
    };

    struct CmdInfoRequest
    {
        static void getBin(std::vector<Platform::byte>& bin);        
    };        


    


} // namespace BfBootCore

#endif
