#ifndef BOOTERSTARTUPPARAMS_H
#define BOOTERSTARTUPPARAMS_H

#include "stdafx.h"
#include "iLog/LogManager.h"
#include "RangeIOBase.h"

#include "Utils/IExitTaskObserver.h"
#include "BfBootCore/BroadcastSourcePort.h"
#include "IReload.h"


namespace BfBootSrv
{
    using boost::scoped_ptr;        

    // стартовые параметры загрузчика
    struct BooterStartupParams : boost::noncopyable
    {          
        IReload& Reload;
        RangeIOBase& StorageDevice;                
        bool TraceActive;
        std::string LogPrefix;
        iLogW::LogSettings LogParams;
        int BroadcastSrcPort;
        int COM;
        int ListenPort; //Порт для подключения удаленного клиента          
        Platform::dword ResetKeyHoldingMsec; // время удержания кнопки "Blackfin Reset" для сброса настроек (msec) > 1000        
        Utils::IExitTaskObserver* pKeyAbort; // nullPtr if unused
                
        BooterStartupParams(RangeIOBase& storageDevice, IReload& reload) 
            : Reload(reload),
            StorageDevice(storageDevice)
        {
            InitDefault();
        }

        bool IsValid() const
        {
            return 
                ResetKeyHoldingMsec >= 1000                                                                               
                && ListenPort <= 65535;
        }                    

    private:

        void InitDefault()
        {
            ResetKeyHoldingMsec = 10 * 1000;                        
            ListenPort = 0;       // any
            LogPrefix = "Default";
            TraceActive = false;
            BroadcastSrcPort = BfBootCore::CBroadcastSourcePort;
            COM = -1; // unused    
            pKeyAbort = 0;

            LogParams.TimestampInd(true);           
        }

    };

    
} // namespace BfBootSrv

#endif
