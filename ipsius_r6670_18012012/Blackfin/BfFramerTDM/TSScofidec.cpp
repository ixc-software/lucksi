#include "stdafx.h"

#include <ccblkfn.h>
#include <cdefBF537.h>
#include "BfDev/SysProperties.h"
#include "bftdm/tdm.h"
#include "Utils/BidirBuffer.h"
#include "Utils/TimerTicks.h"
#include "iVDK/VdkCheckInit.h"
#include "BfDev/BfTimerCounter.h"
#include "DevIpTdm/BfLed.h"
#include "DevIpTdm/BfKeys.h"
#include "ExtFlash/flashmap.h"
#include "iUart/IUartIO.h"
#include "E1App/AppPlatformHook.h"
#include "Utils\IntToString.h"

#include "leds.h"
#include "TSScofidec.h"


namespace BfCofidec
{    

	enum	TdmSource
	{        
		TdmSilent,
		TdmGenerator,
		TdmLoop,
	};	

	enum
	{        
		CmdSourceLoop = '1',
		CmdSourceGenerator = '2',
		CmdSourceSilent = '3',
		CmdModeOff = '0',
		CmdModeHandset = '9',
		CmdModeHansFree = '8',
		CCmdModeRing = 'r',
		CmdModeCrossHandsetSpeak = '7',
		CmdModeCrossLoudSpeak = '6',
		CmdCurVolInc = 'q',
		CmdCurVolDec = 'a',
		CmdCurSensInc = 'w',
		CmdCurSensDec = 's',
		CCmdFreqInc = 'e',
		CCmdFreqDec = 'd',
		CCmdLoadRegisterSet = 'l',
		CCmdGetACEState = 'i',
	};	


	TdmSource CurrentTdmSource;
	bool loudMode = true;
	byte loopMode = 0;
	iPult::CofidecState currentState;
	byte AfeSource = 0;
	bool isGenerator = false;
	int GeneratorFreq = 420;
	int GeneratorLevel = -7;


	std::string ACERegisterSet = (std::string)"SCTL = 0xc000 \n\r" + 
												"SATT1 = 0x2c6a \n\r" +
												"SATT2 = 0x0aff \n\r" +
												"SLGA = 0x4040 \n\r" +
												"SAELEN = 0x0200 \n\r" +
												"SAEATT = 0x1400 \n\r" +
												"SAEGS  = 0x0 \n\r" +
												"SAEPS  = 0x0 \n\r" +
												"SAEBL  = 0x0 \n\r" +
												"SAEWFL = 0x5000 \n\r" +
												"SAEWFT = 0x4000 \n\r" +
												"LECCTL = 0x8000 \n\r" +
												"LECLEV = 0x64ff \n\r" +
												"LECATT = 0x03fc \n\r" +
												"LECMGN = 0x06a40 \n\r";
		;							

    PsbCofidecTest::PsbCofidecTest():
        m_uart0Obj(BfDev::SysProperties::Instance().getFrequencySys(), 0, 115200),
		m_cofidec(this),
        m_processedBlocks(0)
    {         
    }

    
    //-------------------------------------------------------------------                
    
	const std::string BoolToState(bool val)
	{
		return val ? "Enabled" : "Disabled";
	}    
    
    //-------------------------------------------------------------------            
        
    
    void PsbCofidecTest::Run()
    {
        
		iVDK::VdkInitDone();    	    	
        BfDev::SysProperties::InitSysFreq118750kHz();
        
	    E1App::AppPlatformHookSetup(true, true, false); // UART + no reboot    
        
        
        try
        {
			PsbCofidec::PsbCofidecInit::InitSync();
        	PsbCofidecTest PsbCofidecObj;
        	PsbCofidecObj.Process();
        }
		catch(std::exception &e)
		{
			ESS_UNEXPECTED_EXCEPTION(e);
		}
/*
        catch(ExtFlash::M25P128Absent &e)
        {
        	ESS_ASSERT(0 && "M25P128 not found");
        }                                                      
*/
    }
    
    //-------------------------------------------------------------------                   

  	void PsbCofidecTest::NewBlockProcess(
        word sportNum,
        BfTdm::IUserBlockInterface &m_Ch0,
        BfTdm::IUserBlockInterface &m_Ch1,
        bool collision)
    {        
    	    	
    	
//    	*pPORTGIO_TOGGLE	= 1<<4;
    	*pPORTGIO_SET = 1<<5;

        word tssize = m_Ch0.GetTSBlockSize();
        byte TSBuff[CTDMBlockSize];

		for (word i = 0; i < CTDMBlockSize; i++)TSBuff[i] = 0;

        for (word timeSlot = 1; timeSlot < 32; timeSlot++)
        {
        	m_Ch0.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0, tssize);						
        }

  		byte table[] = {0x3a,0xd5,0xba,0xa3,0xba,0xd5,0x3a,0x23};   //1KHz    	
//  		byte table[] = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};   //1KHz    	

		if(CurrentTdmSource == TdmSilent)
			for (word i=0; i<CTDMBlockSize; i++) TSBuff[i] = 0x55;
		if(CurrentTdmSource == TdmLoop) 
			m_Ch0.CopyRxTSBlockTo(0, TSBuff, 0, tssize);


		if(CurrentTdmSource == TdmGenerator)
		{		
			m_generator->Fill(TSBuff, tssize);
/*
			byte sin_num=0;
			for (word i=0; i<CTDMBlockSize; i++)
			{
				else TSBuff[i] = table[sin_num];
				sin_num++;
				if (sin_num>7) sin_num =0;
			}
*/
		}

		m_Ch0.PutTxTSBlockFrom(0, TSBuff, 0, tssize);				        	        	

        m_Ch0.UserFree();
        m_Ch1.UserFree();        
        m_processedBlocks++;

//    	*pPORTGIO_TOGGLE	= 1<<4;            
    	*pPORTGIO_CLEAR	= 1<<5;
    }		    	

    //-------------------------------------------------------------------                        
        
    void PsbCofidecTest::BlocksCollision(word sportNum)
    {
        while(1)
		{
			BfDev::Leds::SetLed(BfDev::GREEN);	
			iVDK::Sleep(100);
			BfDev::Leds::SetLed(BfDev::OFF);	        	
			iVDK::Sleep(100);	        	
		}
    }

	//-------------------------------------------------------------------                        

	void PsbCofidecTest::WriteLn(const std::string &line)
	{
        m_uart0Obj.SendMsg(line.c_str());               
	}
                
    //-------------------------------------------------------------------                                
    
    void PsbCofidecTest::Process()
    {           	        

		iVDK::Sleep(100);
   	    	    	
        m_uart0Obj.SendMsg("PsbCofidec Debug Project \n\r");

        BfTdm::Tdm  TdmObj(*this, 0, BfTdm::TdmProfile(3, CTDMBlockSize, 1, BfTdm::CNoCompanding));
		TdmObj.Enable();        		

		word afeChModeCounter = 0;
		word LedCounter = 0;
		bool isGreen = true;
		CurrentTdmSource = TdmLoop;

		currentState.HandsfreeSpeakerLevel = 50;
		currentState.HandsfreeMicLevel = 70;
        currentState.HandsetMicLevel = 50;
        currentState.HandsetSpeakerLevel = 50;
        currentState.Mode = CsEnum::mTdmToHandset;
        currentState.LoopMode = CsEnum::lmLoopOff;
		m_cofidec.SetState(currentState);
		loudMode = false;
		
        while (1)
        {                               	
			iVDK::Sleep(5);
			afeChModeCounter++;
            LedCounter++;
			if(m_uart0Obj.AvailableForRead())
			{
                UartControl(m_uart0Obj.ReadByte());
			}
			if(afeChModeCounter > 1500)
			{
				afeChModeCounter = 0;
//				ChangeCofidecMode();
//				ChangeCofidecLoopMode();
			}
			if(LedCounter > 100)
			{			
				LedCounter = 0;
				if(isGreen)
				{
					isGreen = false;
					BfDev::Leds::SetLed(BfDev::OFF);					
				}
				else
				{
					isGreen = true;
					BfDev::Leds::SetLed(BfDev::GREEN);
				}
			}

	    }                
    }


	//-------------------------------------------------------------------    	

	void PsbCofidecTest::ChangeCofidecMode()
	{        
		if(loudMode)
		{
			loudMode = false;
			m_uart0Obj.SendMsg("-----------------------------------\n\r");
			m_uart0Obj.SendMsg("HandSet Mode \n\r");
			CurrentTdmSource = TdmLoop;			
			currentState.Mode = CsEnum::mTdmToHandset;
			m_cofidec.SetState(currentState);
		}
		else
		{
			loudMode = true;
			m_uart0Obj.SendMsg("-----------------------------------\n\r");
			m_uart0Obj.SendMsg("Loud Mode \n\r");
			CurrentTdmSource = TdmGenerator;			
			currentState.Mode = CsEnum::mTdmToLoud;
			m_cofidec.SetState(currentState);
		}
	}

	//-------------------------------------------------------------------    	

	void PsbCofidecTest::ChangeCofidecLoopMode()
	{        
		loopMode++;
		if(loopMode > 4) loopMode = 0;
		currentState.LoopMode = (CsEnum::LoopMode)loopMode;
		m_cofidec.SetState(currentState);
	}

	//-------------------------------------------------------------------    	

	void PsbCofidecTest::UartControl(byte command)
	{        
		switch(command)
		{
		case CmdSourceSilent:
			CurrentTdmSource = TdmSilent;
			m_uart0Obj.SendMsg("Source is silent\n\r");
			break;
		case CmdSourceGenerator:
			CurrentTdmSource = TdmGenerator;
			m_uart0Obj.SendMsg("Source is Generator\n\r");
			GenControl(0);
			break;
		case CmdSourceLoop:
			CurrentTdmSource = TdmLoop;
			m_uart0Obj.SendMsg("Source is Loop\n\r");
			break;
		case CmdModeOff:
			currentState.Mode = CsEnum::mOff;
			m_cofidec.SetState(currentState);
			break;
		case CmdModeHandset:
			currentState.Mode = CsEnum::mTdmToHandset;
			m_cofidec.SetState(currentState);
//			m_cofidec.GetACE().SetModeLoudSpeak();  //!!!!!!!!!!!!!!!!!!!!!!!!
			break;
		case CmdModeHansFree:
			currentState.Mode = CsEnum::mTdmToLoud;
			m_cofidec.SetState(currentState);
			break;
		case CmdModeCrossHandsetSpeak:
			currentState.Mode = CsEnum::mTdmCrossToHandset;
			m_cofidec.SetState(currentState);
			break;
		case CmdModeCrossLoudSpeak:
			currentState.Mode = CsEnum::mTdmCrossToLoud;
			m_cofidec.SetState(currentState);
			break;

		case CmdCurSensInc:
			SenseVolControl(command);
			break;
		case CmdCurSensDec:
			SenseVolControl(command);
			break;
		case CmdCurVolInc:
			SenseVolControl(command);
		case CmdCurVolDec:
			SenseVolControl(command);
			break;
		case CCmdFreqInc:
			GenControl(command);
			break;
		case CCmdFreqDec:
			GenControl(command);
			break;
		case CCmdLoadRegisterSet:
			m_cofidec.GetACE().LoadExtRegsFromIniFile(ACERegisterSet);
			break;
		case CCmdModeRing:
			m_cofidec.GetACE().SetModeLoudRing();
			break;
		case CCmdGetACEState:
			m_uart0Obj.SendMsg((m_cofidec.GetACE().GetState()).c_str());
			break;


		default:
			m_uart0Obj.SendMsg("Undefined Command\n\r");
		}
	}

    //-------------------------------------------------------------------    	

	void PsbCofidecTest::SenseVolControl(byte command)
	{
		int *volPtr;
		int *sensPtr;

		if(currentState.Mode == CsEnum::mOff) return;

		switch(currentState.Mode)
		{
			case CsEnum::mTdmToLoud:
				volPtr = &currentState.HandsfreeSpeakerLevel;
				sensPtr = &currentState.HandsfreeMicLevel;
				break;
			case CsEnum::mTdmToHandset:
				volPtr = &currentState.HandsetSpeakerLevel;
				sensPtr = &currentState.HandsetMicLevel;
				break;				
			case CsEnum::mTdmCrossToLoud:
				volPtr = &currentState.HandsfreeSpeakerLevel;
				sensPtr = &currentState.HandsetMicLevel;
				break;				
			case CsEnum::mTdmCrossToHandset:
				volPtr = &currentState.HandsetSpeakerLevel;
				sensPtr = &currentState.HandsfreeMicLevel;
				break;				
			default:
				return;
		}
		if(command == CmdCurVolInc && *volPtr <= 90) *volPtr += 10;
		if(command == CmdCurVolDec && *volPtr >= 10) *volPtr -= 10;

		if(command == CmdCurSensInc && *sensPtr <= 90) *sensPtr += 10;
		if(command == CmdCurSensDec && *sensPtr >= 10) *sensPtr -= 10;
		m_cofidec.SetState(currentState);
	}

    //-------------------------------------------------------------------    	

	void PsbCofidecTest::GenControl(byte command)
	{

		if(command == CCmdFreqInc) 
		{
			if(m_generator->Validate(1000, 0, GeneratorFreq + 100, GeneratorLevel))
				GeneratorFreq += 100;
		}
		if(command == CCmdFreqDec) 
		{
			if(m_generator->Validate(1000, 0, GeneratorFreq - 100, GeneratorLevel))
				GeneratorFreq -= 100;
		}
		std::string log;
//		log = "Freq = " + Utils::IntToString(GeneratorFreq) + " Hz, Lev = " + 
//			Utils::IntToString(GeneratorLevel) + "dBm \n\r";
		log = "Freq = " + Utils::IntToString(GeneratorFreq) + " Hz\n\r";
 
		m_uart0Obj.SendMsg(log.c_str());
		m_generator.reset(new iDSP::FullCicleGenOneTone(1000, 0, true, GeneratorFreq, GeneratorLevel));

	}

    //-------------------------------------------------------------------    	

    void PsbCofidecTest::ShowStatus()
    {        

        std::ostringstream log;						
		log<< "TSS Board Status" << std::endl; 
/*
        log<< "Board Signature : 0x" <<std::hex << (int)m_signature << std::endl;
        log<< "Board Version : 0x" <<std::hex << (int)m_version << std::endl;        
        log<< "Board HookA : 0x" <<std::hex << (int)m_hookA << std::endl;                
        log<< "Board HookB : 0x" <<std::hex << (int)m_hookB << std::endl;                        
*/
        m_uart0Obj.SendMsg(log.str().c_str());               
    }                        
}
    