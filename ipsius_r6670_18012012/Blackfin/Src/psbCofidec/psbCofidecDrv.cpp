
#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "DevIpPult/DevIpPultSpiMng.h"
#include "TdmMng/TdmException.h"

#include "Psb4851.h"
#include "Psb2170.h"
#include "PsbCofidecDrv.h"

namespace PsbCofidec
{	

	PsbCofidecDrv::PsbCofidecDrv(IPsbDebug *iDebug) 
    {    	
        m_spi.reset( new DevIpPult::DevIpPultSpiMng() );
        
        m_AFE.reset( 
            new Psb4851(m_spi->CreatePoint( m_spi->GetPsb4851BusLock()), iDebug) 
            );

        m_ACE.reset( 
            new Psb2170( m_spi->CreatePoint( m_spi->GetPsb2170BusLock()), iDebug) 
            );
    }

	// -------------------------------------------------------------------
    
    PsbCofidecDrv::~PsbCofidecDrv()
    {
        SetMode(CsEnum::mOff);
    }

    // -------------------------------------------------------------------

    void PsbCofidecDrv::SetState(const iPult::CofidecState &state)
    {

        m_AFE->SetHandsetMicLevel      ( Map(state.HandsetMicLevel,       CMaxHandSetMicSens) );
        m_AFE->SetHandsetSpeakerLevel  ( Map(state.HandsetSpeakerLevel,   CMaxHandSetVol) ); 

        m_AFE->SetHandsfreeMicLevel    ( Map(state.HandsfreeMicLevel,     CMaxLoudMicSens) );
        m_AFE->SetHandsfreeSpeakerLevel( Map(state.HandsfreeSpeakerLevel, CMaxLoudVol) );

        SetLoop(state.LoopMode);
        SetMode(state.Mode);

        if ( state.EchoMode.size() )
        {
            m_ACE->LoadExtRegsFromIniFile(state.EchoMode);
        }
    }

    // -------------------------------------------------------------------

    void PsbCofidecDrv::SetDebugMode(bool toHandsfree)
    {
        SetMode(toHandsfree ? CsEnum::mTdmToLoud : CsEnum::mTdmToHandset);

        m_AFE->SetHandsetMicLevel      (CMaxHandSetMicSens);
        m_AFE->SetHandsetSpeakerLevel  (CMaxHandSetVol); 

        m_AFE->SetHandsfreeMicLevel    (CMaxLoudMicSens);
        m_AFE->SetHandsfreeSpeakerLevel(CMaxLoudVol);
    }
    
  	// -------------------------------------------------------------------

    
    std::string PsbCofidecDrv::StateInfo() const
    {
        return m_AFE->GetState();        
    }

	// -------------------------------------------------------------------

    void PsbCofidecDrv::PsbCofidecDrv::SetMode(CsEnum::CsMode mode)
	{
        SetMode(mode, *m_AFE, *m_ACE);
	}

	// -------------------------------------------------------------------

	void PsbCofidecDrv::PsbCofidecDrv::SetLoop(CsEnum::LoopMode loopMode)
	{
		SetLoop(loopMode, *m_AFE, *m_ACE);
	}

	// -------------------------------------------------------------------

    void PsbCofidecDrv::SetMode(CsEnum::CsMode mode, Psb4851 &afe, Psb2170 &ace)
    {
        if (mode == CsEnum::mOff)
        {
            afe.SetModeOff();
            ace.SetModeOff();
            return;
        }

        if (mode == CsEnum::mTdmToHandset)
        {
            afe.SetModeHandSpeak();
            ace.SetModeHandSpeak();
            return;
        }

        if (mode == CsEnum::mTdmToLoud)
        {
            afe.SetModeLoudSpeak();
            ace.SetModeLoudSpeak();
            return;
        }

		if (mode == CsEnum::mTdmLoudRing)
		{
			afe.SetModeLoudSpeak();
			ace.SetModeLoudRing();
			return;
		}

		if (mode == CsEnum::mTdmCrossToLoud)
		{
			afe.SetModeCrossLoudSpeak();
            ace.SetModeHandSpeak(true);
			return;
		}

		if (mode == CsEnum::mTdmCrossToHandset)
		{
			afe.SetModeCrossHandSpeak();
			ace.SetModeHandSpeak(true);
			return;
		}

        ThrowError("Bad mode");
    }

	// -------------------------------------------------------------------
    void PsbCofidecDrv::SetLoop(CsEnum::LoopMode loopMode, Psb4851 &afe, Psb2170 &ace)
	{
		afe.SetLoopMode(loopMode);
		ace.SetLoopMode(loopMode);
	}

    // -------------------------------------------------------------------

    void PsbCofidecDrv::ThrowError(const std::string &msg)
    {
    	using namespace TdmMng;
        ESS_THROW_T( TdmException, TdmErrorInfo(terBadPultCofidecState, msg) );
    }
    
} //namespace Cofidec
