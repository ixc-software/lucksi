#include "stdafx.h"

#include "iLog/LogWrapperInterfaces.h"
#include "BfBootCore/BroadcastSourcePort.h"

#include "E1AppConfig.h"


namespace
{

    enum
    {        
        // Log out
        CLogToCout              = false,
        CLogToUdp               = false
    };

	const char *CUdpLogIp = "127.0.0.1";
	const int CUdpLogPort = 56001;

    // -----------------------------------------------
}  // namespace


// -----------------------------------------------------

namespace E1App
{

    // отсюда нельзя вызывать E1App::Stack, т.к. он сам 
    // создается на основе поле NetworkConfig из этого класса
    E1AppConfig::E1AppConfig() :
        m_pAppExit(0)
    {
        AppName = "BfApplication";
		
        // LogSettings
        {
            LogConfig.StoreRecordsLimit(0);
            LogConfig.StoreCleanTo(0);
            LogConfig.TimestampInd(true);
            LogConfig.CountSyncroToStore(1);  // sync on every records
            LogConfig.UseRecordsReorder(false);
            LogConfig.DublicateSessionNamesDetection(false);
            LogConfig.out().Cout().TraceInd = CLogToCout;
			LogConfig.out().Udp().TraceInd = CLogToUdp;
			LogConfig.out().Udp().DstHost = Utils::HostInf(CUdpLogIp, CUdpLogPort);
			IsTraceOn = CLogToCout || CLogToUdp;
        }

        // Network
        NetworkConfig.AddresCfg.UseDHCP();

        // other
        ThreadRunnerSleepInterval = 1 * 1000;
        EnterTdmTestOnStartup = false;
        MonitorCpuUsage = true;
        AppPollTimeMs = 25;
        BroadcastSrcPort = BfBootCore::CBroadcastSourcePort;

		MinRtpPort = 0;
		MaxRtpPort = 0;
		
		EchoMode = emHardwareThenSfx;
    }

}  // namespace E1App

