#include "stdafx.h"
#include "ServerTuneParametrs.h"

#include "BfBootCli/RealTasks.h"


namespace
{
    template<class T>
    void AddUserSettings(BfBootCli::ITaskOwner& mng, const T& val, const std::string& paramName)
    {
        new BfBootCli::TaskSetUserSettings(mng, val, paramName);
    }
};

namespace BfBootCore
{
    void UserParams::WriteTaskToMng( BfBootCli::ITaskOwner& mng ) const
    {
        // Log
        if (UdpLogHost.IsInited()) AddUserSettings(mng, UdpLogHost.get(), "UdpLogHost");
        if (UdpLogPort.IsInited()) AddUserSettings(mng, UdpLogPort.get(), "UdpLogPort");        
        if (UseTimestamp.IsInited()) AddUserSettings(mng, UseTimestamp, "UseTimestamp");
        if (CountToStore.IsInited()) AddUserSettings(mng, CountToStore, "CountToStore");

        // Network
        if (Network.IsInited())
        {
            AddUserSettings(mng, Network.get().DHCP(), "UseDHCP");
            AddUserSettings(mng, Network.get().Mask(), "NetMask");
            AddUserSettings(mng, Network.get().IP(), "IP");
            AddUserSettings(mng, Network.get().Gateway(), "Gateway");                
        }

        // User settings of server
        if (UserPwd.IsInited()) AddUserSettings(mng, UserPwd.get(), "UserPwd");
        if (WaitLoginMsec.IsInited()) AddUserSettings(mng, WaitLoginMsec.get(), "WaitLoginMsec");

        // AppPort
        if (CmpPort.IsInited()) AddUserSettings(mng, CmpPort.get(), "CmpPort");       
    }

    // ------------------------------------------------------------------------------------

    void DefaultParam::WriteTaskToMng( BfBootCli::ITaskOwner& mng ) const
    {
        new BfBootCli::TaskSetFactory(mng, HwNumber, HwType, Mac);
        OptionalSettings.WriteTaskToMng(mng);
    }    
} // namespace BfBootCore

