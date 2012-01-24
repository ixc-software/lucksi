#include "stdafx.h"
#include "BfEmul/E1PcEmul.h"
#include "NObjBoardEmulPair.h"
#include "NObjEmulRunner.h"
#include "E1App/MngLwip.h"
#include "BfDev/SysProperties.h"

namespace 
{
	bool E1PcRunnerInd = false;
	const int CPoolTimerMs = 1000;
	
	enum {
		CSIZE_RTP_RANGE_BLOCK = 100
	};
		
	void InitStaticObj(const E1App::NetworkSettings &networkConfig)
	{
		static bool initInd = false;
		if(initInd) return;
		initInd = true;
		BfEmul::PcEmul::Init();
        BfDev::SysProperties::InitFromProjectOptions(); // move to BfEmul::PcEmul:: ?
		E1App::Stack::Init(networkConfig);
	}    
};

namespace BfEmul
{
	NObjEmulRunner::NObjEmulRunner(Domain::IDomain *pDomain, const Domain::ObjectName &name) :
		Domain::NamedObject(pDomain, name),
		m_logSettings(new Domain::NObjLogSettings(this, "Log")),
        m_timerCycle(0),
        m_timer(this, &T::OnTimer)
	{
		if (E1PcRunnerInd) ThrowRuntimeException("NObjE1AppPcRunner already created.");

		E1PcRunnerInd = true;
		m_logSettings->CountToSync(1);
		m_list.reset(new E1App::RtpPortRangeList(0, 0, CSIZE_RTP_RANGE_BLOCK));
		m_timer.Start(CPoolTimerMs, true);
	}

	// -----------------------------------------------------------------------------------------

	NObjEmulRunner::~NObjEmulRunner()
	{
		E1PcRunnerInd = false;
		
		EnableAll(false);             

		while(m_runner.Process() != 0);

        for(int i = 0; i < m_emulPair.size(); ++i)
        {
            delete m_emulPair.at(i);
        }
        for(int i = 0; i < m_emul.size(); ++i)
        {            
            delete m_emul.at(i);
        } 
	}

	// -----------------------------------------------------------------------------------------

	void NObjEmulRunner::InitRtpRange(int min, int max)
	{
		if(m_list != 0 && m_list->AllocatedBlock() != 0) ThrowRuntimeException("Rtp range already used.");

		if(min > max) ThrowRuntimeException("Min max error.");

		m_list.reset(new E1App::RtpPortRangeList(min, max, CSIZE_RTP_RANGE_BLOCK));
	}

	// -----------------------------------------------------------------------------------------

	Q_INVOKABLE void NObjEmulRunner::CreateEmulPair(const QString &name, int boardNumber1, int boardNumber2)
	{
		InitStaticObj(GetNetworkSettings());
        if (EmulExist(boardNumber1) || EmulExist(boardNumber2)) ThrowRuntimeException("This board id is already used.");		
		m_emulPair.push_back(new NObjBoardEmulPair(*this, name, boardNumber1, boardNumber2));
	}

    // -----------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjEmulRunner::CreateEmul( const QString &name, int boardNumber )
    {
        InitStaticObj(GetNetworkSettings());
        if (EmulExist(boardNumber)) ThrowRuntimeException("This board id is already used.");        
        m_emul.push_back(new NObjBoardEmul(this, name, boardNumber, RtpRangeCreator(), true));
    }

    // -----------------------------------------------------------------------------------------

    /*Q_INVOKABLE void NObjEmulRunner::DeleteAllEmul()
    {
        if (m_runner.Process() != 0) ThrowRuntimeException("StopAll first");
        m_emul.Clear();
        m_emulPair.Clear();
    }*/

	// -----------------------------------------------------------------------------------------

	void NObjEmulRunner::StartAllEmul(DRI::IAsyncCmd *pAsyncCmd)
	{
		EnableAll(true);
	}

	// -----------------------------------------------------------------------------------------

	iLogW::LogSettings NObjEmulRunner::LogSettings()
	{
		return m_logSettings->LogSettings();
	}

	// -----------------------------------------------------------------------------------------

	void NObjEmulRunner::StopAllEmul(DRI::IAsyncCmd *pAsyncCmd)
	{
		AsyncBegin(pAsyncCmd);
		EnableAll(false);
	}

	// -----------------------------------------------------------------------------------------

	E1App::NetworkSettings NObjEmulRunner::GetNetworkSettings() const
	{
		E1App::NetworkSettings settings;
		settings.AddresCfg.Set("127.0.0.1", "127.0.0.1", "255.255.255.0");
		return settings;
	}

	// -----------------------------------------------------------------------------------------

	void NObjEmulRunner::OnObjectDelete(DRI::IAsyncCmd *pAsyncCmd)
	{
		StopAllEmul(pAsyncCmd);
	}
	
	// -----------------------------------------------------------------------------------------

	void NObjEmulRunner::OnTimer(iCore::MsgTimer *pT)
	{
		if (m_runner.Process() != 0)  return;
		
		if(!AsyncActive()) return;

		AsyncComplete(true);
		AsyncDeleteSelf();
	}

	// -----------------------------------------------------------------------------------------

	void NObjEmulRunner::EnableAll(bool enabled)
	{
		for(int i = 0; i < m_emulPair.size(); ++i)
		{
			m_emulPair.at(i)->Enable(enabled);
		}

        for(int i = 0; i < m_emul.size(); ++i)
        {
            if (enabled) m_emul.at(i)->Start(m_runner, m_logSettings->LogSettings());
            else         m_emul.at(i)->Stop();
        }        
	}

    bool NObjEmulRunner::EmulExist( int boardNum )
    {
        for(int i = 0; i < m_emulPair.size(); ++i)
        {
            if(m_emulPair.at(i)->IsBoardPresent(boardNum)) return true;
        }

        for(int i = 0; i < m_emul.size(); ++i)
        {
            if(m_emul.at(i)->BoardNumber() == boardNum) return true;            
        }

        return false;
    }

    
};

