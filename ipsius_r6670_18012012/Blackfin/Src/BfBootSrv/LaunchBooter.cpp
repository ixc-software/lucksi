#include "stdafx.h"
#include "iCore/ThreadRunner.h"

#include "LaunchBooter.h"
#include "BooterStartupParams.h"
#include "Config.h"
#include "BootServer.h"
#include "BoardSetup.h"

#include "Platform/Platform.h"
#include "E1App/MngLwip.h"

#include "Utils/TimerTicks.h"

#include "DevIpTdm/BfKeys.h"
#include "DevIpTdm/BfLed.h"

#include "BfBootCore/GeneralBooterConst.h"
#include "BoardHardcodedConfig.h"

namespace 
{  
    using namespace BfBootSrv;    
    
    // блокирующий опрос кнопки сброса
    bool PollReset(Platform::dword timeOut)
    {        
        Utils::TimerTicks tick;
        tick.Set(timeOut, false);

        bool leadOn = true;
        while (DevIpTdm::BfKeys::Get(0))
        {
            if ( tick.TimeOut() ) return true;
            tick.Sleep(50);
            DevIpTdm::BfLed::Set(1, leadOn); 
            leadOn = !leadOn;
        }

        return false;    
    }

    // ------------------------------------------------------------------------------------

    void RunServer(const BooterStartupParams& params, BoardSetup& board)
    { 
        // todo включить BooterStartupParams в BootServerParams

        BootServerParams srvParams(board, params.Reload, params.LogParams);               
        srvParams.CbpPort = params.ListenPort;                
        srvParams.LogPrefix = params.LogPrefix;
        srvParams.CreteTraceActive = params.TraceActive;       
        board.TuneTrace( srvParams.LogSettings );
        srvParams.COM = params.COM;     
        srvParams.pKeyAbort = params.pKeyAbort;
        srvParams.BroadcastSrcPort = params.BroadcastSrcPort;

        //srvParams.SbpProf.setPacketMaxSizeBytes(BfBootCore::CChankSize);
        srvParams.SbpProf.setMaxReceiveSize(BfBootCore::CMaxChankSize + BfBootCore::CAditionalReservedSize);

        //srvParams.SbpProf. -- from BooterStartupParams ?

        iCore::ThreadRunner runner;
        runner.Run<BootServer>("BootServer", srvParams);                

        srvParams.Action.Run();

        //std::cout << "Warning! No main application loaded. Booter start newly." << std::endl;                                                                 
        //LaunchBooter(params, board, false);
    }    
        
} // namespace 


namespace BfBootSrv
{         

	void LaunchBooter(const BooterStartupParams& params, BoardSetup &boardSetup, bool initNetwork)
	{        
		ESS_ASSERT( params.IsValid() );		

        // reset to default
		if (!boardSetup.IsDefault() && PollReset(params.ResetKeyHoldingMsec)) 
		{
			boardSetup.ResetSettings();
		} 

        // init network
        if (initNetwork)
        {                                    
            E1App::NetworkSettings cfg;                                    
            if( boardSetup.ReadNetworkSettings(cfg.AddresCfg) ) 
            {                                
                const int CBuffSize = 1024 * 2;
                cfg.BuffCfg.rx_buff_datalen = CBuffSize;
                cfg.BuffCfg.rx_buffs = BfBootCore::CSplitSizeForSendToBooterByTcp / CBuffSize; 
                if (BfBootCore::CSplitSizeForSendToBooterByTcp % CBuffSize != 0) ++cfg.BuffCfg.rx_buffs;                

                E1App::Stack::Init(cfg, false);                                   
            }
        }                

		RunServer(params, boardSetup);
	}



    void LaunchBooter(const BooterStartupParams& params)
    {        
        ESS_ASSERT( params.IsValid() );
        BoardSetup boardConfigured(params.StorageDevice);                                                                
		LaunchBooter(params, boardConfigured, true);
        //LaunchBooter(params, boardConfigured, false);
    }
} // namespace BfBootSrv

