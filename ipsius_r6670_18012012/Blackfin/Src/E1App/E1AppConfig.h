#ifndef __E1APPCONFIG__
#define __E1APPCONFIG__

#include "Utils/IBasicInterface.h"
#include "TdmMng/TdmManager.h"
#include "iLog/LogManager.h"
#include "Utils/IExitTaskObserver.h"
#include "Ds2155/boardds2155.h"

#include "MngLwip.h"

namespace E1App
{
    enum EchoModeEnum
    {
        emNone,
        emHardware,
        emHardwareThenSfx,        
    };

    // полный набор настроек для работы приложения
    struct E1AppConfig
    {
        std::string AppName;  // имя приложения, для работы на PC позволяет идентифицировать различные приложения
        std::string AppBuildInfo;

        // log    
        iLogW::LogSettings LogConfig;
        bool IsTraceOn;
            
        // Network
        NetworkSettings NetworkConfig;
        
        // other
        int ThreadRunnerSleepInterval;
        bool EnterTdmTestOnStartup;
        bool MonitorCpuUsage;
        int  AppPollTimeMs;
        int BroadcastSrcPort;
        
		// Rtp setting 
		int MinRtpPort;
		int MaxRtpPort;

        // Echo
        EchoModeEnum EchoMode; 

        // выставляет настройки для работы на Blackfin
        E1AppConfig();
        
		void SetAppExit(Utils::IExitTaskObserver *pAppExit)
        {
            m_pAppExit = pAppExit;
        }

        bool AppExitRequest() const
        {
            if (m_pAppExit == 0) return false;

            return m_pAppExit->IsTaskOver();
        }

    private:
        Utils::IExitTaskObserver *m_pAppExit;
    };    
    
    
    
}  // namespace E1App

#endif
