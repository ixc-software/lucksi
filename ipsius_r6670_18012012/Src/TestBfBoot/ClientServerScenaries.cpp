#include "stdafx.h"
#include "ClientServerScenaries.h"

#include "BfBootSrv/ConfigSetup.h"
#include "BfBootSrv/FactorySettings.h"

#include "BfBootCli/TaskComposer.h"


namespace 
{
    const std::string CDefaultPwd = "DefaultTestPwd";
    const Platform::dword CSrvWaitLoginMsec = 1000;
    
} // namespace 

namespace TestBfBoot
{    
    ScnUpdateFw::ScnUpdateFw( Domain::DomainClass& domain, const Settings& prof )
        : ClientSrvTestBase(domain, prof),
		m_usePrimRange(false),
        m_srvStorageSize(0)
    {
    }
    bool ScnUpdateFw::PreValidateFlash( BfBootSrv::RangeIOBase& flash )
    {
        ESS_HALT("todo");
        //Условие: сервер должен загружатся с prim/second
        //CfgManager cfgManager(flash);
        // cfgManager.getSrc()...

        // m_usePrimRange =...
        // m_pwd = cfgManager.getPwd()    
        // m_srvStorageSize = 
        return false;
    }

    void ScnUpdateFw::PrepareFlash( BfBootSrv::RangeIOBase& flash )
    {           
        
        flash.OutStreamRange(BfBootSrv::RangeIOBase::CfgSecond)->getWriter().WriteByte(0);

        shared_ptr<BfBootSrv::Config> cfg = BfBootSrv::Config::CreateCfg();

        std::string s;        
        
        cfg->IP = getProf().ServerIp.Address();        
        cfg->Gateway = "192.168.0.35"; // ???
        cfg->UseDHCP = false;
        cfg->UserPwd = CDefaultPwd;
        cfg->WaitLoginMsec = CSrvWaitLoginMsec;
        //cfg->HwNumber = 0;
        //cfg->HwType = 0;
        cfg->Save(flash, BfBootSrv::RangeIOBase::CfgPrim);         
        //cfg->Save(flash, BfBootSrv::RangeIOBase::CfgDefault);
        m_usePrimRange = true;

        flash.EraseRange(BfBootSrv::RangeIOBase::CfgDefault);
        shared_ptr<BfBootSrv::FactorySettings> factory = BfBootSrv::FactorySettings::ReadFromDefaultRange(flash);
        factory->Set(0, 0, "21:33:B1:78:45:22");
        factory->SaveToDefaultRange(flash);
                      
        BfBootSrv::IDebugRangeIOBase& dFlesh = flash;
        m_srvStorageSize = dFlesh.getRange(BfBootSrv::RangeIOBase::AppStorRange).Map().FullSize();
    }

    shared_ptr<BfBootCli::ITaskManager> ScnUpdateFw::SetTasks( const BfBootCore::BroadcastMsg& msg)
    {
        ESS_ASSERT(m_srvStorageSize != 0);
        QByteArray bin;
        bin.fill('d', m_srvStorageSize);               
        BfBootCli::TaskMngProfile clientProfile(getDomain(), *this, getProf().TraceClient, getProf().EmulName.toStdString());   
        
        m_fw.AddDataFromBin("d-filled", bin);
        m_fw.AddScript("TEST", "Load(main, 0); Load(second, 1)");
        m_fw.SetRelease(1);        

        shared_ptr<BfBootCli::ITaskManager> result = 
            BfBootCli::TaskComposer::UpdateFirmware(clientProfile, BfBootCli::SbpTcpCreator(msg.CbpAddress), CDefaultPwd, m_fw, getTraceSrv());
        TUT_ASSERT(result);        
        return result;
    }

    bool ScnUpdateFw::PostValidateFlash( BfBootSrv::RangeIOBase& flash )
    {
        return true;
        // todo        
        // 1. актуальный(обновленый) конфиг находится в ожидаемой области (prim/second)
        // 2. список и прошивок и скрипт соответствуют ожидаемым
    }
} // namespace TestBfBoot
