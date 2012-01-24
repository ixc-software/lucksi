#ifndef SERVERTUNEPARAMETRS_H
#define SERVERTUNEPARAMETRS_H

#include "Utils/InitVar.h"
#include "E1App/NetworkSettings.h"
#include "Platform/PlatformTypes.h"

namespace BfBootCli
{
    class ITaskOwner; // или перенести в ITaskOwner в Core
};

namespace BfBootCore
{
    using Platform::dword;
    using Platform::word;

    using Utils::InitProp;    

    struct UserParams
    {               
        // Log 
        InitProp<std::string> UdpLogHost;
        InitProp<word> UdpLogPort;
        InitProp<bool> UseTimestamp;
        InitProp<word> CountToStore;

        // Network 
        InitProp<E1App::BoardAddresSettings> Network;

        // User settings of server 
        InitProp<std::string> UserPwd;
        InitProp<dword> WaitLoginMsec;

        // AppPort
        InitProp<word> CmpPort;

        // Implemented only on PC-side
        void WriteTaskToMng(BfBootCli::ITaskOwner& mng) const;        
    };

    // ------------------------------------------------------------------------------------

    struct DefaultParam
    {   
        // Factory (mandatory params)        
        std::string Mac;
        dword HwType;
        dword HwNumber;  

        UserParams OptionalSettings;

        DefaultParam(dword hwType, dword hwNumber, const std::string& mac)            
        {
            Mac = mac;
            HwType = hwType;
            HwNumber = hwNumber;
        }

        // Implemented only on PC-side
        void WriteTaskToMng(BfBootCli::ITaskOwner& mng) const;              
        
    };

} // namespace BfBootCore

#endif
