#ifndef CLIENTSRVTESTBASE_H
#define CLIENTSRVTESTBASE_H

//#include "iCore/ThreadRunner.h"
#include "Utils/threadtaskrunner.h"

#include "iLog/LogWrapper.h"
#include "iLog/LogManager.h"
#include "iLog/LogSettings.h"

#include "BfBootCli/BroadcastReceiver.h"
#include "BfBootCli/ITaskManager.h"
#include "BfBootCli/ITaskManagerReport.h"

#include "Domain/DomainClass.h"

#include "LaunchSrv.h"
#include "FlashEmul.h"


namespace TestBfBoot
{
    using boost::shared_ptr;
    using boost::scoped_ptr; 

    /*
        Базовый тип для конкретных клиентсерверных тестов-сценариев.
    */
    class ClientSrvTestBase : 
        public iCore::MsgObject,
        public BfBootCli::IBroadcastReceiverToOwner,
        public BfBootCli::ITaskManagerReport
    {  
    protected:

        struct GeneralSettings
        {
            QString EmulName; // Used as logPrexix, flash file-name
            Utils::HostInf ServerIp; // Board host address. Optional (use network settings stored in flash if empty).
            int CmpPort;            
            bool UseFlashContent; // if true use Config stored in flash, false - save new.

            bool TraceClient;
            bool TraceServer;
            bool TraceTest;            
            bool IsValid() const;
            GeneralSettings();

            // конфигурировать флешь здесь ?
        };

        const GeneralSettings& getProf() const
        {
            return m_prof;
        }

        bool getTraceSrv() const {return m_prof.TraceServer;}
        iLogW::LogSession& getLog();
        iLogW::ILogSessionCreator& getLogCreator(){return m_domain.Log();}
        ClientSrvTestBase(Domain::DomainClass& domain, const GeneralSettings& prof);
        Domain::IDomain& getDomain(){return m_domain;}

    // todo измнеить этот интерфейс
    // ITaskManagerReport impl
    private:
        void AllTaskFinished (const BfBootCli::TaskResult& result);   

        void Info(QString eventInfo, int progress);

    // IBroadcastReceivertoOwner impl
    private:
        void MsgReceived(const BfBootCore::BroadcastMsg& msg);

    // functions for override
    private:
        //virtual shared_ptr<BfBootSrv::BooterStartupParams> CreateSrvProf() = 0;
        virtual shared_ptr<BfBootCli::ITaskManager> SetTasks(const BfBootCore::BroadcastMsg& msg) = 0;
        virtual bool PostValidateFlash(BfBootSrv::RangeIOBase& flash) = 0;        
        virtual bool PreValidateFlash(BfBootSrv::RangeIOBase& flash) = 0;
        virtual void PrepareFlash(BfBootSrv::RangeIOBase& flash) = 0; // будет изменен при использовании класса интегрирующего flash+BfStorage+Config
        
    // own methods:
    private:
        shared_ptr<BfBootSrv::BooterStartupParams> CreateSrvProf();
        void OnRun();        

    // fields:
    private:
        const bool m_dummy;        
        
        Domain::DomainClass& m_domain;
        GeneralSettings m_prof;        
        shared_ptr<iLogW::LogSession> m_log;
        iLogW::LogRecordTag m_tagTaskEvent;
        BfRangeStorageDevEmul m_flash;        
        scoped_ptr<BfBootCli::BroadcastReceiver> m_rxBrodcast;
        shared_ptr<BfBootCli::ITaskManager> m_client;        
        Utils::ThreadTaskRunner m_srvHolder;
    };
} // namespace TestBfBoot

#endif
