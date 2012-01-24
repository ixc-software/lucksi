#include "stdafx.h"
#include "NObjBoardEmul.h"
#include "IpTdmBoardEmul.h"


#include "BfBootCore/ServerTuneParametrs.h"

namespace 
{
    const int CPoolTimerMs = 1000;
    const int CRtpPortBlockSize = 100;
} // namespace 

namespace BfEmul
{               
    // Вызывается из нити эмулируемого приложения!
    bool NObjBoardEmul::IsTaskOver() const
    {                
        return !m_enabled;
    }

    // ------------------------------------------------------------------------------------

    void NObjBoardEmul::Start( Utils::ThreadTaskRunner& runner, const iLogW::LogSettings &log )
    {
        if (m_enabled) return;

        if (!m_pSettings->Validate()) ThrowRuntimeException("Wrong settings. Can`t enabled.");  

        m_pSettings->getParams().HwNumber = m_boardId;

        boost::shared_ptr<IpTdmBoardEmulProfile> profile(new IpTdmBoardEmulProfile);

        profile->SetBoardCfg(m_pSettings->getParams());
        profile->m_softVer = m_softVer;
        profile->m_useBooter = m_useBooter;        
        profile->m_e1AppConfig.AppName = StringId();
        profile->m_e1AppConfig.MinRtpPort = MinRtpPort();
        profile->m_e1AppConfig.MaxRtpPort = MaxRtpPort();        
        profile->m_e1AppConfig.LogConfig = log;
        profile->m_e1AppConfig.ThreadRunnerSleepInterval = 200;
        profile->m_e1AppConfig.IsTraceOn = m_traceE1App;
        profile->m_e1AppConfig.MonitorCpuUsage = false;
        profile->m_e1AppConfig.SetAppExit(this);
        profile->PAppExit = this;

        PropertyWriteEventAbort("Board already started.");
        m_pSettings->LockChanges(true);
        m_enabled.Set(true);
        
        runner.Add<IpTdmBoardEmul>(profile);            
        //E1App::PcEmul::Instance().Add ?
    }

    // ------------------------------------------------------------------------------------

    void NObjBoardEmul::Stop()
    {
        if (!m_enabled) return;        
        m_enabled.Set(false);
        m_pSettings->LockChanges(false);        
    }

    // ------------------------------------------------------------------------------------

    NObjBoardEmul::NObjBoardEmul( Domain::NamedObject *pParent, const Domain::ObjectName &name, Platform::dword boardId, E1App::IRtpRangeCreator &rtp, bool setToDefaultCfg /*= false*/ ) : Domain::NamedObject(pParent, name, pParent),
        m_pSettings( new NObjBoardSettings(this, "BoardSettings") ),
        m_boardId(boardId)
    {
        m_rtpRange = rtp.AllocRtpPortRange(CRtpPortBlockSize);
        InitDefault();            

        if (setToDefaultCfg)
        {
            const int CType = 1;            
            m_pSettings->SetDefaultParams(CType, boardId, "");
            m_pSettings->SetNetwork(false, "127.0.0.1", "127.0.0.1", "255.255.255.0");
        }
    }

} // namespace BfEmul
