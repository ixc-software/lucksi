#ifndef CLIENTSERVERSCENARIES_H
#define CLIENTSERVERSCENARIES_H

/*
    Клиент-серверные тесты-сценарии
*/

#include "BfBootSrv/BooterStartupParams.h"
#include "BfBootSrv/Config.h"
#include "BfBootCli/TaskComposer.h"
#include "ClientSrvTestBase.h"

#include "Domain/DomainClass.h"
#include "FirmwareEmul.h"

namespace TestBfBoot
{
    // предполагает наличие пользовательских сетевых настроек
    class ScnUpdateFw : public ClientSrvTestBase
    {      
        
    public:

        struct Settings : public GeneralSettings 
        {
            // add scenario specific fields here
            // ...            
        };
        
        ScnUpdateFw(Domain::DomainClass& domain, const Settings& prof);

    // ClientSrvTestBase overrides
    private:
        bool PreValidateFlash(BfBootSrv::RangeIOBase& flash);
        void PrepareFlash(BfBootSrv::RangeIOBase& flash);
        shared_ptr<BfBootCli::ITaskManager> SetTasks(const BfBootCore::BroadcastMsg& msg);
        bool PostValidateFlash(BfBootSrv::RangeIOBase& flash);

    // fields
    private:
                
        FirmwareEmul m_fw;
        bool m_usePrimRange; // useSecond if false   
        Platform::dword m_srvStorageSize;
    };
} // namespace TestBfBoot

#endif
