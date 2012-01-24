#include "stdafx.h"
#include "IpTdmBoardEmul.h"

namespace 
{
    void WriteFakeSoftware(BfBootSrv::RangeIOBase& flash, Platform::dword rev)
    {
        BfBootSrv::BoardSetup setup(flash);
        setup.OpenWriteImg(1);
        Platform::byte b = 1;        
        setup.WriteImgChunk(&b, 1);
        BfBootCore::CRC crc;
        crc.ProcessBlock(&b, 1);
        setup.CloseNewImg("FakeE1App", crc.Release());        
        setup.CloseTransaction(rev);
        BfBootCore::ScriptList list;
        list.Add( BfBootCore::NamedScript("BOOT", "Load(MAIN, 0)") );       
        setup.SetUserParam("StartScriptList", BfBootCore::StringToTypeConverter::toString(list) );
        setup.SaveChanges();
    }

    class CloseE1AppImpl :
        public E1App::ICloseApp
    {                  
        void Reboot(Utils::AtomicBool &breakFlag)
        {
            breakFlag.Set(true); // pc impl
            //reloader.Reboot();// bf impl
        }        
    };  
} // namespace 

namespace BfEmul
{
    // todo: вызывать RunBooter, RunE1 через сообщения (получить свой трид, IpTdmBoardEmul:MsgObject)
    
    IpTdmBoardEmul::IpTdmBoardEmul( boost::shared_ptr<IpTdmBoardEmulProfile> profile ) 
        : m_e1AppConfig(profile->m_e1AppConfig),
        m_startBooterCounter(0),
        m_startE1AppCounter(0)
    {		

        // create flash emulation
        QString fileName(profile->m_e1AppConfig.AppName.c_str());
        fileName.replace(".", "_");            
        m_flash.reset(new BfRangeStorageDevEmul(fileName + ".bin", true) );        

        if (profile->ExistBoardCfg()) 
            BfBootSrv::ConfigSetup(m_flash->getRangeIO(), profile->getBoardCfg());           

        if (profile->m_softVer >= 0) WriteFakeSoftware(m_flash->getRangeIO(), profile->m_softVer);

        if (!profile->m_useBooter)
			ESS_ASSERT(profile->ExistBoardCfg());
		else
        {			
            m_booterParams.reset(new BfBootSrv::BooterStartupParams(m_flash->getRangeIO(), *this) );
            m_booterParams->LogParams = profile->m_e1AppConfig.LogConfig;
            m_booterParams->LogParams.out().Udp().TraceInd = false; 
            m_booterParams->LogPrefix = profile->m_e1AppConfig.AppName;
            m_booterParams->ResetKeyHoldingMsec = 1000;
            m_booterParams->ListenPort = 0;
            m_booterParams->pKeyAbort = profile->PAppExit;
            m_booterParams->TraceActive = false;
            m_booterParams->COM = 0;
            m_booterParams->BroadcastSrcPort = 0; // Bind any
			

            //if (profile->m_softVer > 0) m_script.reset(new ScriptCmdList(profile->m_script.toStdString()));
        }

        
        Reboot();

        while (!profile->PAppExit->IsTaskOver())
        {     
            if (!m_rebootFlag) continue;
            m_rebootFlag = false;
            if (m_booterParams) RunBooter();
            else                RunE1App();
        }     
        
    }

    // ------------------------------------------------------------------------------------

    void IpTdmBoardEmul::RunBooter()
    {
        Log("RunBooter");
        m_startBooterCounter++;

        m_stateBootSrv = true;
        BfBootSrv::BoardSetup boardSetup(m_flash->getRangeIO());                   
        BfBootSrv::LaunchBooter(*m_booterParams, boardSetup);                        
        Log("BooterFinished");
    }

    void IpTdmBoardEmul::RunE1App()
    {
        Log("Run E1App");  
        m_startE1AppCounter++;

        m_stateBootSrv =false;
        BfBootSrv::BoardSetup boardSetup(m_flash->getRangeIO());    
        CloseE1AppImpl closeImpl;
        E1App::RunE1Application(*this, closeImpl, m_e1AppConfig, boardSetup.CreateBroadcastForApp(), boardSetup.getCmpPort());
        Log("E1App finished");
    }

    void IpTdmBoardEmul::RunEcho()
    {
        m_stateEcho = true;
    }

    void IpTdmBoardEmul::Reboot()
    {
        Log("Reboot...");
        m_stateEcho = false;
        m_rebootFlag = true; 

        /*if (m_booterParams) RunBooter();
        else                RunE1App();*/
    }

    void IpTdmBoardEmul::LoadToMain( BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img )
    {
        ESS_ASSERT(storage.getOffset(img));
        Log("StartMainApp");
        RunE1App();
    }

    void IpTdmBoardEmul::LoadToSpi( BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img )
    {
        ESS_ASSERT(storage.getOffset(img));
        Log("Echo");
        RunEcho();
    }

    void IpTdmBoardEmul::LoadToSpi( Platform::dword size, const Platform::byte *data )
    {
        ESS_UNIMPLEMENTED;
    }
} // namespace BfEmul

