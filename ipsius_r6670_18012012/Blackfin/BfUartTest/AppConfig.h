#ifndef __APPCONFIG__
#define __APPCONFIG__

    #include "TdmMng/TdmManager.h"
    #include "Utils/HostInf.h"    
    //#include "iLog/LogManager.h"


	namespace iLogW
	{
		class ILogSessionCreator;
    	class LogManager;
    	class LogSettings;		    	
	};
	
	namespace iCore 
	{
		class MsgThread;
	}

    namespace Ds2155
    {
        class HdlcProfile;
    }

    namespace LwIP
    {
        class NetworkSettings;
    }


    // ----------------------------------------------

    class AppConfig
    {
    public:

        static TdmMng::TdmManagerProfile CreateTdmManagerProfile(
            iLogW::ILogSessionCreator &log, iCore::MsgThread &msgThread);

        static iLogW::LogSettings GetLogSettings();
        
        static Utils::HostInf GetLogHost();
        
        static Ds2155::HdlcProfile CreateHdlcProfile();
        	
        static int GetThreadRunnerSleepInterval();

        static bool EnterTdmTestOnStartup();

        static bool EnterUartTestOnStartup();
        
        static void GetNetworkSettings(LwIP::NetworkSettings &cfg);

    };

#endif
