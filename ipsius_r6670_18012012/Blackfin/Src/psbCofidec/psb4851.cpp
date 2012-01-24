#include "stdafx.h"

#include "Utils\IntToString.h"

#include "Psb4851.h"


namespace PsbCofidec
{
	Psb4851 *Psb4851Obj = 0;

	//-------------------------------------------------------------------

	Psb4851::Psb4851(const BfDev::SpiBusPoint &point, IPsbDebug *iDebug) :
        m_point(point),
        m_debug(iDebug),
    	m_spi( m_point.Spi() )
    {    	
		ESS_ASSERT(Psb4851Obj == 0 && "Psb4851 already created");
		
		m_lastHandsfreeSpeakerLevel = 0;
		m_lastHandsetMicLevel = 0;
		m_lastHandsetSpeakerLevel = 0;
		m_lastHandsfreeMicLevel = 0;
		
		Init();

		m_currentMode = "Undef";
        SetModeOff(); 				
		Psb4851Obj = this;
    }
    
    //-------------------------------------------------------------------            
    
	Psb4851::~Psb4851()
	{
		Psb4851Obj = 0;
	}

    //-------------------------------------------------------------------            
	// Handset Microphone sensitivity 1..8
	void Psb4851::SetHandsetMicLevel(byte val)
	{
		ESS_ASSERT(val <= CMaxHandSetMicSens && "Bad cofidec Mic sensitivity");
		m_lastHandsetMicLevel = val;
		std::string log;
		log = "SetHandsetMicLevel  " + Utils::IntToHexString(m_lastHandsetMicLevel,true,1) + "\n\r";

		byte temp = ReadReg(AIAR);
		temp &= 0x0F;
		temp |= (val << 4);
		log += "AIAR = " + Utils::IntToHexString(temp,true,1) + "\n\r";
        SendToLog(log);
		WriteReg(AIAR, temp);
	}

    //-------------------------------------------------------------------            
	// Handset speaker volume 1..9
	void Psb4851::SetHandsetSpeakerLevel(byte val)
	{
		ESS_ASSERT(val <= CMaxHandSetVol && "Bad cofidec volume");
		m_lastHandsetSpeakerLevel = val;
		std::string log;
		log = "SetHandsetSpeakerLevel  " + Utils::IntToHexString(m_lastHandsetSpeakerLevel,true,1) + "\n\r";
        SendToLog(log);
		if(val) val = CMaxHandSetVol - val + 1;
		byte temp = ReadReg(AOAR);
		temp &= 0x0F;
		temp |= (val << 4);
		WriteReg(AOAR, temp);
	}

    //-------------------------------------------------------------------            
	// HandsFree Microphone sensitivity 1..8
	void Psb4851::SetHandsfreeMicLevel(byte val)
	{
		ESS_ASSERT(val <= CMaxLoudMicSens && "Bad cofidec Mic sensitivity");
		m_lastHandsfreeMicLevel = val;
		std::string log;
		log = "SetHandsfreeMicLevel  " + Utils::IntToHexString(m_lastHandsfreeMicLevel,true,1) + "\n\r";
		byte temp = ReadReg(AIAR);
		temp &= 0x0F;
		temp |= val<<4; 
		log += "AIAR = " + Utils::IntToHexString(temp,true,1) + "\n\r";
        SendToLog(log);
		WriteReg(AIAR, temp);
	}

    //-------------------------------------------------------------------            
	// HandsFree speaker volume 1..12
	void Psb4851::SetHandsfreeSpeakerLevel(byte val)
	{
		ESS_ASSERT(val <= CMaxLoudVol && "Bad cofidec volume");
		m_lastHandsfreeSpeakerLevel = val;
		std::string log;
		log = "SetHandsfreeSpeakerLevel  " + Utils::IntToHexString(m_lastHandsfreeSpeakerLevel,true,1) + "\n\r";
        SendToLog(log);
		if(val) val = CMaxLoudVol - val + 1;
		byte temp = ReadReg(AOAR);
		temp &= 0xF0;
		temp |= val;
		WriteReg(AOAR, temp);
	}

    //-------------------------------------------------------------------

	void Psb4851::Init()
	{					
		*pPORTGIO_DIR |= 0x0002;	//Loud speaker control
		WriteReg(AIPR, 0x53);
		WriteReg(AOPR, 0xd0);
	}

	//-------------------------------------------------------------------

	byte Psb4851::ReadReg(AFEReg reg)
	{					
        BfDev::SpiBusLocker busLock(m_point);	    
		m_spi.Transfer(0x10 + reg);
		return m_spi.Transfer(0xFF);
	}

	//-------------------------------------------------------------------

	void Psb4851::WriteUnVerifyReg(AFEReg reg, byte val)
	{					
		BfDev::SpiBusLocker busLock(m_point);	    
		m_spi.Transfer(reg);
		m_spi.Transfer(val);
	}

	//-------------------------------------------------------------------

	void Psb4851::WriteReg(AFEReg reg, byte val)
	{					
		WriteUnVerifyReg(reg, val);
		if(val == ReadReg(reg)) return;
		ESS_THROW_MSG(PSB4851Error, "Reg Verify Error");
	}

    //-------------------------------------------------------------------            

	void Psb4851::SetModeOff()
	{
        SendToLog("SetModeOff\n\r");
		HandsetSpeakerOff();
		HandsfreeSpeakerOff();
		WriteReg(AIAR, 0);
		m_currentMode = "Off";
	}

    //-------------------------------------------------------------------            

	void Psb4851::SetModeHandSpeak()
	{
        SendToLog("SetModeHandSpeak\n\r");
		HandsfreeSpeakerOff();
		SetHandsetSpeakerLevel(m_lastHandsetSpeakerLevel);
		SetHandsetMicLevel(m_lastHandsetMicLevel);
		WriteReg(AIPR, 0x53);
		m_currentMode = "HandSet";
	}

	//-------------------------------------------------------------------            

	void Psb4851::SetModeCrossLoudSpeak()
	{
		*pPORTGIO_CLEAR = 0x0002;	
		SendToLog("SetModeCrossLoudSpeak\n\r");
		HandsetSpeakerOff();
		SetHandsfreeSpeakerLevel(m_lastHandsfreeSpeakerLevel);
		SetHandsetMicLevel(m_lastHandsetMicLevel);
		WriteReg(AIPR, 0x53);
		m_currentMode = "CrossLoudSpeak";
	}

	//-------------------------------------------------------------------            

	void Psb4851::SetModeCrossHandSpeak()
	{
		SendToLog("SetModeCrossHandSpeak\n\r");
		HandsfreeSpeakerOff();
		SetHandsetSpeakerLevel(m_lastHandsetSpeakerLevel);
		SetHandsfreeMicLevel(m_lastHandsfreeMicLevel);
		WriteReg(AIPR, 0x51);
		m_currentMode = "CrossHandSpeak";
	}

    //-------------------------------------------------------------------            

	void Psb4851::SetModeLoudSpeak()
	{
        SendToLog("SetModeLoudSpeak\n\r");
		*pPORTGIO_CLEAR = 0x0002;	
		HandsetSpeakerOff();
		SetHandsfreeMicLevel(m_lastHandsfreeMicLevel);
		SetHandsfreeSpeakerLevel(m_lastHandsfreeSpeakerLevel);
		WriteReg(AIPR, 0x51);
		m_currentMode = "HandsFree";
	}

    //-------------------------------------------------------------------            

	void Psb4851::HandsfreeSpeakerOff()
	{
		WriteReg(AOAR, ReadReg(AOAR) & 0xF0);
		*pPORTGIO_SET = 0x0002;	
        SendToLog("HandsfreeSpeakerOff\n\r");
	}

    //-------------------------------------------------------------------            

	void Psb4851::HandsetSpeakerOff()
	{
		WriteReg(AOAR, ReadReg(AOAR) & 0x0F);
        SendToLog("HandsetSpeakerOff\n\r");
	}		

    //-------------------------------------------------------------------            

	void Psb4851::SetLoopMode(CsEnum::LoopMode loopMode)
	{
		if (loopMode == CsEnum::lmLoopOff || loopMode == CsEnum::lmAACE) 
		{
			WriteReg(TFCR, 0);
			SendToLog("mLoopOff\n\r");
			return;
		}
		
		if (loopMode == CsEnum::lmAConverter) 
		{
			WriteReg(TFCR, 0x10); 
			SendToLog("mAConverter\n\r");
			return;
		}
		
		if (loopMode == CsEnum::lmA64K) 
		{
			WriteReg(TFCR, 0x18);
			SendToLog("mA64K\n\r");
			return;
		}			
		
		if (loopMode == CsEnum::lmAInterface) 
		{
			WriteReg(TFCR, 0x20);
			SendToLog("mAInterface\n\r");
			return;
		}
		
		ESS_THROW_MSG(PSB4851Error, "Invalid Loop Mode");
	}

    //-------------------------------------------------------------------            

	std::string Psb4851::GetState()
	{
		std::string result;
		result += "Current mode is " + m_currentMode + "\n\r";
		result += "m_lastHandsfreeMicLevel is " + Utils::IntToHexString(m_lastHandsfreeMicLevel) + "\n\r";
		result += "m_lastHandsfreeSpeakerLevel is " + Utils::IntToHexString(m_lastHandsfreeSpeakerLevel) + "\n\r";
		result += "m_lastHandsetMicLevel is " + Utils::IntToHexString(m_lastHandsetMicLevel) + "\n\r";
		result += "m_lastHandsetSpeakerLevel is " + Utils::IntToHexString(m_lastHandsetSpeakerLevel) + "\n\r";

		result += "AIPR = " + Utils::IntToHexString(ReadReg(AIPR))  + "\n\r";
		result += "AOPR = " + Utils::IntToHexString(ReadReg(AOPR))  + "\n\r";
		result += "AIAR = " + Utils::IntToHexString(ReadReg(AIAR))  + "\n\r";
		result += "AOAR = " + Utils::IntToHexString(ReadReg(AOAR))  + "\n\r";
		return result;
	}

	//-------------------------------------------------------------------            

    void Psb4851::SendToLog(const std::string &line)
	{
		if(!m_debug) return;
		m_debug->WriteLn("AFE -> " + line);
	}
    
} //namespace Cofidec
