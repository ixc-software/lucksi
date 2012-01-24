#include <cdefBF537.h>
#include <services/services.h>	// for types u32, u16, etc
#include <stdio.h>
#include "led.h"
#include "sys.h"
#include "Timer0.h"
#include "LanTest.h"
#include "UartSimple.h"
#include "flash.h"
#include "liu.h"
#include "bert.h"
#include "tdm.h"
#include "ds28cn01Test.h"
#include "macros.h"


	enum
	{
		CLedTime = 100,
		CPRIStatus = 3000,
		CBERTLineTestTime = 65000,
		CBERTLineStep = 1000,		
		CWaitEchoResult = 5000,				
	};			
	
	char buff[512];		

#ifdef ECHO_CHIP_MAIN

	int main()
	{
	
		bool error = false;
	
		DevIpTdm::PllInit();
		DevIpTdm::SDRAMInit();
		TimerInit();
	
		DevIpTdm::SetColorLed(DevIpTdm::YELLOW);

		Uart0Init();
		Send0Msg("Echo Board test \n");	

	  	if(DevIpTdm::SDRAMTest()) 
	  	{
	  		Send0Msg("Memory OK \n");
			DevIpTdm::SetColorLed(DevIpTdm::GREEN);	  		
	  	}
		else
		{
			DevIpTdm::SetColorLed(DevIpTdm::RED);
			Send0Msg("Memory test FAIL \n");		
			error = true;
		}
		
		
		Init_Sport0();	
		Init_SPORT0_DMA();
		Init_DMA_Interrupts();	
		Enable_DMA_Sport0();			
		
		if(error) while(1);
		
		
		
		dword LedTimeOut = GetTimerCounter();		
		while(1)
		{
			dword counter = GetTimerCounter();				
			if(counter - LedTimeOut >= CLedTime)
			{
				LedTimeOut = counter;			
				DevIpTdm::ColorLedToggle();
//				Send0Msg("ECHO \n");						
			
			}
		}		
		
	}

#endif		

#ifdef TWI_DEBUG_MAIN	
		
	int main()
	{	
	    
		DevIpTdm::PllInit();
		DevIpTdm::SDRAMInit();
		DevIpTdm::InitASYNC();
		SpiInit();	
		FlashInit();
		TimerInit();
	
		DevIpTdm::SetColorLed(DevIpTdm::OFF);
		Uart0Init();		
	
		Delay(1000);								

		dword LedTimeOut = GetTimerCounter();		
		Send0Msg("TWI DEBUG\n");	
						
		DevIpTdm::LedSet(0, true);
		DevIpTdm::LedSet(1, false);
		
		while(1)
		{
			dword counter = GetTimerCounter();
				
			if(counter - LedTimeOut >= CLedTime)
			{
				LedTimeOut = counter;			
			
				if(!DevIpTdm::GetKey())
				{
					DevIpTdm::LedToggle(1);
					DevIpTdm::LedToggle(0);			
				}
				else
				{
                    if (DevIpTdm::DS28CN01Test()) Send0Msg("TEST OK\n");
                    else Send0Msg("TEST ERROR\n");           

                    DevIpTdm::DS28CN01ShowInfo();
			        while(DevIpTdm::GetKey());
				}
			}

		}
	
	}
	
#endif			


#ifdef TEST_MAIN	

	int main()
	{
	
		bool error = false;
	
		DevIpTdm::PllInit();
		DevIpTdm::SDRAMInit();
		DevIpTdm::InitASYNC();
		SpiInit();	
		FlashInit();
		TimerInit();
		
	
		DevIpTdm::SetColorLed(DevIpTdm::OFF);
		Uart0Init();		
	
		Delay(1500);
		
		Send0Msg("TDMIp Board test \n");	
				
	#ifdef ECHO_CHIP_PRESENT
		DevIpTdm::SecondBFLoadSpi();

		Uart1Init();				
		Send0Msg("Echo board:\n");	
		dword WaitEchoResult = GetTimerCounter();		
		while(1)
		{
			dword counter = GetTimerCounter();
			if(counter - WaitEchoResult >= CWaitEchoResult)	break;
			if(ReadyData(1))
			{
				buff[0] = ReadByte(1);
				buff[1] = 0;				
				Send0Msg(buff);
			}
		}
	#endif			

		Send0Msg("Main board:\n");			
		
#ifndef LINE_BERT_FOREVER
		
	  	if(DevIpTdm::SDRAMTest()) Send0Msg("Memory OK \n");
		else
		{
			Send0Msg("Memory test FAIL \n");		
			error = true;
		}	

	  	if(LanTest()) Send0Msg("ETHERNET OK \n");
		else
		{
			Send0Msg("ETHERNET test FAIL \n");				
			error = true;
		}

	  	if(FlashTest()) Send0Msg("FLASH OK \n");
		else
		{
			Send0Msg("FLASH test FAIL \n");				
			error = true;
		}	
#endif		
					
	  	if(LiuInit()) Send0Msg("LIU OK \n");
		else
		{
			Send0Msg("LIU is ABSENT \n");				
			error = true;
		}
		
        if (DevIpTdm::DS28CN01Test()) Send0Msg("SecureChip OK\n");
        else 
        {
            Send0Msg("SecureChip ERROR\n");
        }
        if (!DevIpTdm::isDS28CN01Clear())
        {
            if (DevIpTdm::isSecretLoaded()) Send0Msg("Secret Loaded\n");        
            else Send0Msg("WARNING SecureChip is not clear & SecretError\n");        
        }
		
	
		if(error) 
		{
			DevIpTdm::SetColorLed(DevIpTdm::RED);		
			while(1);		
		}	
	
		Init_Sport0();	
		Init_SPORT0_DMA();
		Init_DMA_Interrupts();	
		Enable_DMA_Sport0();			

		for(byte i=1; i<=PSP_channels; ++i) BertSetChannelToTest(i);
		BertStart(LINE);	
	
		word BERTTestTime = 0;
		bool isBERTTestLine = true;	
		bool isBERTTestTDM = false;		
		bool wasBERTsync = false;		
		bool isPRIStatus = false;			
		dword lasterrors;
		dword allerrors;	
		bool pattern = false;
		Send0Msg("BER test Line-side started\n");
		
		dword PRIStatusTimeOut = GetTimerCounter();	
		dword BERTLineStep = GetTimerCounter();	

		dword LedTimeOut = GetTimerCounter();

						
		DevIpTdm::LedSet(0, true);
		DevIpTdm::LedSet(1, false);
		
		while(1)
		{
			dword counter = GetTimerCounter();
				
			if(counter - LedTimeOut >= CLedTime)
			{
				LedTimeOut = counter;			
			
				if(!DevIpTdm::GetKey())
				{
					DevIpTdm::LedToggle(1);
					DevIpTdm::LedToggle(0);			
				}
			}

			if((counter - BERTLineStep >= CBERTLineStep) && isBERTTestLine)
			{			
				BERTLineStep = counter;
				BERTTestTime += CBERTLineStep;			
				DevIpTdm::ColorLedToggle();
				bool lss = BertLSS();

				if(pattern)
				{
				
					dword temp = BertUpdateErrBitCount();
					if(temp!=lasterrors)
					{
						sprintf(buff, "ErrBitCount = %d\n", temp);  							
						Send0Msg(buff);													
						lasterrors = temp;
						allerrors += temp;
					}				
					Send0Msg(".");
				}
				
				
				if(lss == pattern)
				{
					pattern = !lss;
					if(pattern) 
					{
						if(!wasBERTsync)
						{
							wasBERTsync = true;					
							lasterrors = BertUpdateErrBitCount();
							Delay(100);
							lasterrors = BertUpdateErrBitCount();
							allerrors = lasterrors;
//							sprintf(buff, "allerrors = %d\n", allerrors);  		
//							Send0Msg(buff);
							
						}
						Send0Msg("Sync OK ");
					}
					else if(wasBERTsync) Send0Msg("Sync FAIL ");
				}

#ifdef LINE_BERT_FOREVER
				if(0)
#else				
				if(BERTTestTime >= CBERTLineTestTime)
#endif				
				{
					if((BertUpdateErrBitCount() - allerrors == 0) && wasBERTsync) Send0Msg(" OK\n");
					else 
					{
						Send0Msg(" FAIL\n");
						sprintf(buff, "ErrBitCount = %d\n", BertUpdateErrBitCount());  		
						Send0Msg(buff);
						DevIpTdm::SetColorLed(DevIpTdm::RED);
						error = true;					
					}
				
					if(!isBERTTestTDM)
					{
						isBERTTestTDM = true;
						wasBERTsync = false;
						pattern = false;						
						BERTTestTime = 0;
						BertStop();			
						BertStart(TDM);								
						Send0Msg("BER test TDM-side started\n");										
					}
					else 
				
					{
						DevIpTdm::SetColorLed(DevIpTdm::GREEN);					
						isBERTTestLine = false;
						isPRIStatus = true;
						if(error) 
						{
							DevIpTdm::SetColorLed(DevIpTdm::RED);		
							while(1);		
						}					
					}
				
				}
			}						

		
			if(counter - PRIStatusTimeOut >= CPRIStatus && isPRIStatus)
			{			
				PRIStatusTimeOut = counter;
				PriStatShow();
			}		
		}
	
	}	

#endif		

#ifdef T1_DEBUG	
		
	int main()
	{	
	    
		DevIpTdm::PllInit();
		DevIpTdm::SDRAMInit();
		DevIpTdm::InitASYNC();
		SpiInit();	
		FlashInit();
		TimerInit();
	
		DevIpTdm::SetColorLed(DevIpTdm::OFF);
		Uart0Init();		
	
		Delay(1000);								

		dword LedTimeOut = GetTimerCounter();		
		Send0Msg("T1 DEBUG\n");	

	#ifdef ECHO_CHIP_PRESENT
		DevIpTdm::SecondBFLoadSpi();
	#endif						
	
	  	if(LiuInitT1()) Send0Msg("LIU OK \n");
		else Send0Msg("LIU is ABSENT \n");				

		
		Init_Sport0();	
		Init_SPORT0_DMA();
		Init_DMA_Interrupts();	
		Enable_DMA_Sport0();			

		
//		for(byte i=1; i<=PSP_channels; ++i) BertSetChannelToTest(i);
        BertSetChannelToTest(2);
		BertStart(LINE);	
		

		dword PRIStatusTimeOut = GetTimerCounter();			
							
								
		DevIpTdm::LedSet(0, true);
		DevIpTdm::LedSet(1, false);
		
		while(1)
		{
			dword counter = GetTimerCounter();
				
			if(counter - LedTimeOut >= CLedTime)
			{
				LedTimeOut = counter;			
			
				if(!DevIpTdm::GetKey())
				{
					DevIpTdm::LedToggle(1);
					DevIpTdm::LedToggle(0);			
				}
			}
			if(counter - PRIStatusTimeOut >= CPRIStatus)
			{			
				PRIStatusTimeOut = counter;
				PriStatShow();
			}					

		}
	
	}
	
#endif			

#ifdef LOAD_SECRET	
		
	int main()
	{	
	    
		DevIpTdm::PllInit();
		DevIpTdm::SDRAMInit();
		DevIpTdm::InitASYNC();
		SpiInit();	
		FlashInit();
		TimerInit();
	
		DevIpTdm::SetColorLed(DevIpTdm::OFF);
		Uart0Init();		
	
		Delay(1000);								

		dword LedTimeOut = GetTimerCounter();		
		Send0Msg("Load Secret Procedure\n");				
								
		DevIpTdm::LedSet(0, true);
		DevIpTdm::LedSet(1, false);
		

        if (DevIpTdm::DS28CN01Test()) Send0Msg("TEST OK\n");
        else Send0Msg("TEST ERROR\n");           

//        DevIpTdm::DS28CN01ShowInfo();
        
        DevIpTdm::DS28CN01LoadSecret();

        
        if (DevIpTdm::isSecretLoaded()) Send0Msg("Secret verify OK\n");
        else
        {
            Send0Msg("Secret verify ERROR\n");                                   				
            while(1);
        }
		
		
		while(1)
		{
			dword counter = GetTimerCounter();
				
			if(counter - LedTimeOut >= CLedTime)
			{
				LedTimeOut = counter;			
			
				if(!DevIpTdm::GetKey())
				{
					DevIpTdm::LedToggle(1);
					DevIpTdm::LedToggle(0);			
				}
			}

		}
	
	}
	
#endif			

