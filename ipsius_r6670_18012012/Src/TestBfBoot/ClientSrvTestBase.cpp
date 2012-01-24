#include "stdafx.h"
#include "BfBootSrv/BooterStartupParams.h"

#include "BfBootCore/GeneralBooterConst.h"

#include "ClientSrvTestBase.h"
#include "BfBootSrv/IReload.h"


namespace TestBfBoot
{
    class ReloadImplForTest : public BfBootSrv::IReload
    {    
        void Reboot()
        {}

        void LoadToMain(BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img)         
        {}
        void LoadToSpi(BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img)
        {}
        void LoadToSpi(Platform::dword size, const Platform::byte *data)
        {}
    };

    using iLogW::EndRecord;

    bool ClientSrvTestBase::GeneralSettings::IsValid() const
    {
        if (!UseFlashContent && ServerIp.Empty()) return false;
        //... 
        return true;
    }

    // ------------------------------------------------------------------------------------

    ClientSrvTestBase::GeneralSettings::GeneralSettings()
    {
        EmulName = "ClientServerTests";
        ServerIp.Address("127.0.0.1"); // как указать локальный ?
        CmpPort = 3333;               
        UseFlashContent = false;        
        
        TraceClient = false;
        TraceServer = false;
        TraceTest = false;
    }

    // ------------------------------------------------------------------------------------    

    /*iLogW::ILogSessionCreator& ClientSrvTestBase::getLogCreator()
    {
        return m_logMng;
    }*/

    // ------------------------------------------------------------------------------------

    iLogW::LogSession& ClientSrvTestBase::getLog()
    {
        return *m_log;
    }

    // ------------------------------------------------------------------------------------

    ClientSrvTestBase::ClientSrvTestBase( Domain::DomainClass& domain, const GeneralSettings& prof ) 
        : iCore::MsgObject(domain.getMsgThread()),
        m_dummy(prof.IsValid()),        
        m_domain(domain),
        m_prof(prof),        
        m_log(getLogCreator().CreateLogSesion("ClientSrvTestBase", prof.TraceTest, prof.EmulName.toStdString())),// todo resolve test name
        m_tagTaskEvent(m_log->RegisterRecordKind(L"TaskEvent"))
    {
        *m_log << "ClientSrvTestBase created." << EndRecord;
        PutMsg(this, &ClientSrvTestBase::OnRun);
    }

    // ------------------------------------------------------------------------------------

    
    void ClientSrvTestBase::AllTaskFinished (const BfBootCli::TaskResult& result)
    {        
        if (result.Ok)
        {
            *m_log << "Client tasks completed." << EndRecord;
            m_srvHolder.WaitAll();
            TUT_ASSERT(PostValidateFlash(m_flash.getRangeIO()));           

            m_domain.Stop(Domain::DomainExitEsc);
        }
        else
        {
            *m_log << "Client tasks was failed. Additional info: " << result.Info << EndRecord;
            TUT_ASSERT(0 && "Client tasks was failed");
        }        
    }

    // ------------------------------------------------------------------------------------

    void ClientSrvTestBase::Info( QString eventInfo, int progress )
    {
        *m_log << m_tagTaskEvent << eventInfo;
        if (progress != -1) *m_log << " - " << progress;
        *m_log << iLogW::EndRecord;
    }

    // ------------------------------------------------------------------------------------

    void ClientSrvTestBase::MsgReceived( const BfBootCore::BroadcastMsg& msg)
    {
        if (!m_client)
        {
            *m_log << "BootServer found. Ip = " << msg.CbpAddress.Address()
                << "; Cbp-port = " << msg.CbpAddress.Port() << EndRecord;
            m_client = SetTasks(msg);            
            *m_log << "Client side was created."  << EndRecord;
        }           
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<BfBootSrv::BooterStartupParams> ClientSrvTestBase::CreateSrvProf()
    {
        boost::shared_ptr<BfBootSrv::BooterStartupParams> srvProfile;
        ReloadImplForTest reload;
        srvProfile.reset(new BfBootSrv::BooterStartupParams(m_flash.getRangeIO(), reload));
        srvProfile->LogParams = m_domain.Params().LogSettings();// .Log().Settings();// m_prof.LogParams; // use domain logSettings -- ?
        srvProfile->LogPrefix = m_prof.EmulName.toStdString();
        srvProfile->TraceActive = m_prof.TraceServer;
        srvProfile->ResetKeyHoldingMsec = 1000;
        srvProfile->ListenPort = m_prof.CmpPort;       
        //srvProfile->UartTransport.reset( new SpbUartTransportFactoryForTest );  
        srvProfile->COM = 4;
        
        return srvProfile;
    }

    // ------------------------------------------------------------------------------------

    void ClientSrvTestBase::OnRun()
    {
        *m_log << "Start test." << EndRecord;
              
        if (m_prof.UseFlashContent) ESS_ASSERT( PreValidateFlash(m_flash.getRangeIO()) );
        else                        PrepareFlash(m_flash.getRangeIO());

        shared_ptr<BfBootSrv::BooterStartupParams> srvProf = CreateSrvProf();
        m_rxBrodcast.reset( new BfBootCli::BroadcastReceiver(getMsgThread(), *this, BfBootCore::CBroadcastClientPort) );
        *m_log << "Server will be started." << EndRecord;
        m_srvHolder.Add<LaunchSrv>(srvProf);
    }

    
} // namespace TestBfBoot

