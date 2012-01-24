#ifndef __PSB4851__
#define __PSB4851__

#include "stdafx.h"

#include "Platform/Platform.h"
#include "DevIpPult/DevIpPultSpiMng.h"
#include "iPult/CofidecState.h"

#include "IPsbDebug.h"


namespace PsbCofidec
{

    using namespace Platform;
    
	ESS_TYPEDEF(PSB4851Error);

	enum 
	{
		CMaxLoudVol         = 12,
		CMaxHandSetVol      = 9,
		CMaxLoudMicSens     = 8,
		CMaxHandSetMicSens  = 8,
	};
	               
	class Psb4851 : boost::noncopyable
	{				
		enum AFEReg
		{
			AIAR = 1,
			AIPR = 2,
			AOAR = 3,
			AOCR = 4,
			AOPR = 5,
			TFCR = 6,
			TMR  = 7
		};

	public:

        Psb4851(const BfDev::SpiBusPoint &point, IPsbDebug *iDebug);
		~Psb4851();

		std::string GetState();

		void SetLoopMode(CsEnum::LoopMode loopMode);

		void SetModeOff();
		void SetModeHandSpeak();
		void SetModeLoudSpeak();
		void SetModeCrossLoudSpeak();
		void SetModeCrossHandSpeak();
		
		void SetHandsetMicLevel(byte val);          // [0 .. CMaxHandSetMicSens]		
		void SetHandsetSpeakerLevel(byte val);      // [0 .. CMaxHandSetVol]
		void SetHandsfreeMicLevel(byte val);        // [0 .. CMaxLoudMicSens]
		void SetHandsfreeSpeakerLevel(byte val);    // [0 .. CMaxLoudVol]
					        
    private:
    
		void Init();

        byte ReadReg(AFEReg reg);
		void WriteReg(AFEReg reg, byte val);
		void WriteUnVerifyReg(AFEReg reg, byte val);

		void HandsetSpeakerOff();
		void HandsfreeSpeakerOff();

        void SendToLog(const std::string &line);

    private:
    	
        BfDev::SpiBusPoint m_point;
		IPsbDebug *m_debug;
		BfDev::BfSpi	&m_spi;
        
		byte m_lastHandsetMicLevel;
		byte m_lastHandsfreeMicLevel;
		byte m_lastHandsetSpeakerLevel;
		byte m_lastHandsfreeSpeakerLevel;

		std::string m_currentMode;
				
	};

} //namespace Cofidec

#endif

