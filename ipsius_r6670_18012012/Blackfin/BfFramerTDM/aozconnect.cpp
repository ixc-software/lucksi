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
#include "Ds28cn01/DS28CN01.h"
#include "ds2155/boardds2155.h"
#include "iUart/IUartIO.h"

#include "leds.h"
#include "aozconnect.h"

namespace BfAOZConn
{    

	
    AOZConn::AOZConn():
        m_uart0Obj(BfDev::SysProperties::Instance().getFrequencySys(), 0, 115200),
        m_iUart0Obj(m_uart0Obj),
		m_uart1Obj(BfDev::SysProperties::Instance().getFrequencySys(), 1, 115200),
	#ifndef UART1CORE		
			m_iUart1Obj(m_uart1Obj),
	#endif		
		m_extFlash(m_spiBusMng.CreatePoint( m_spiBusMng.GetFlashBusLock() ), true),        
        m_processedBlocks(0),
        m_callA(0),
        m_callB(0)
    {         
	#ifndef ZL_INIT_WITHOUT_SYNC    	
        MakeSync();
	#endif        
        AOZInitBuffers();
    }

    
    //-------------------------------------------------------------------                
    
	const char* BoolToState(bool val)
	{
		return val ? "Enabled" : "Disabled";
	}    
    
    //-------------------------------------------------------------------            
    
    void AOZConn::Run()
    {
        
		iVDK::VdkInitDone();    	    	
        BfDev::SysProperties::InitSysFreq133000kHz();
        
        try
        {
        	BfAOZConn::AOZConn AOZObj;    
        	AOZObj.Process();
        }
        catch(ExtFlash::M25P128Absent &e)
        {
        	while(1)
        	{
	        	BfDev::Leds::SetLed(BfDev::RED);	
	        	iVDK::Sleep(300);
	        	BfDev::Leds::SetLed(BfDev::OFF);	        	
	        	iVDK::Sleep(300);	        	
        	}
        	ESS_ASSERT(0 && "M25P128 not found");
        }                                                      
    }
    
    //-------------------------------------------------------------------                   

  	void AOZConn::NewBlockProcess(
        word sportNum,
        BfTdm::IUserBlockInterface &m_Ch0,
        BfTdm::IUserBlockInterface &m_Ch1,
        bool collision)
    {        
    	    	
    	
//    	*pPORTGIO_TOGGLE	= 1<<4;
//    	*pPORTGIO_SET = 1<<4;

        word tssize = m_Ch0.GetTSBlockSize();
        byte TSBuff[CTDMBlockSize];
        
       	m_Ch1.CopyRxTSBlockTo(0, TSBuff, 0, 1);
       	m_signature = TSBuff[0];
       	m_Ch1.CopyRxTSBlockTo(3, TSBuff, 0, 1);       	
        m_version = TSBuff[0];
        m_Ch1.CopyRxTSBlockTo(1, TSBuff, 0, 1);       	
        m_hookA = TSBuff[0];                
        m_Ch1.CopyRxTSBlockTo(9, TSBuff, 0, 1);       	
        m_hookB = TSBuff[0];                
        
        if(m_hookA != 0xff || m_hookB != 0xff)
        {
            m_callA = 0;
            m_callB = 0;            
            *pPORTGIO_SET = 1<<4;
        }
        else  	*pPORTGIO_CLEAR	= 1<<4;            
        
        
        
        
        for(byte ts = 0; ts < 8; ts++)
        {        
            if(ts == 6)
            {
                for(word i = 0; i < tssize; i++) TSBuff[i] = m_callA;
            	m_Ch1.PutTxTSBlockFrom(ts, &TSBuff[0], 0, tssize);
                for(word i = 0; i < tssize; i++) TSBuff[i] = m_callB;            	
            	m_Ch1.PutTxTSBlockFrom(ts+8, &TSBuff[0], 0, tssize);
            }
            else
            {
                for(word i = 0; i < tssize; i++) TSBuff[i] = m_CTRLbuff[ts];
            	m_Ch1.PutTxTSBlockFrom(ts, &TSBuff[0], 0, tssize);
            	m_Ch1.PutTxTSBlockFrom(ts+8, &TSBuff[0], 0, tssize);
            }
        }
        
        for(byte ts = 0; ts < 32; ts++)
        {        
        	m_Ch0.PutTxTSBlockFrom(ts, m_DialBuff, 0, tssize);        		
        }

        m_Ch0.UserFree();
        m_Ch1.UserFree();        
        m_processedBlocks++;
//    	*pPORTGIO_TOGGLE	= 1<<4;            
//    	*pPORTGIO_CLEAR	= 1<<4;            
    }		    	

    //-------------------------------------------------------------------                        
        
    void AOZConn::BlocksCollision(word sportNum)
    {
        while(1);
    }
    
    //-------------------------------------------------------------------                                    

#ifndef UART1CORE    
        
    bool AOZConn::WriteRead(byte valWrite, byte &valRead)
    {
    	std::ostringstream log;
    	log<< "0x" <<std::hex << (int)valWrite << " <--->";    	
    	m_uart1Obj.WriteImpl(&valWrite, 1);
    	dword startWait = GetSystemTickCount();
    		
   	
    	while((GetSystemTickCount() - startWait) < 2)
    	{
    		if(m_uart1Obj.AvailableForRead())
    		{
    			m_uart1Obj.ReadImpl(&valRead, 1, false);
                log<< "0x" <<std::hex << (int)valRead << std::endl;
			#ifdef ZL_TRAFFIC_DEBUG
		        m_uart0Obj.SendMsg(log.str().c_str());    			
		    #endif
    			return true;
    		}
    	}
    	
	#ifdef ZL_TRAFFIC_DEBUG    	
		m_uart0Obj.SendMsg("--\n");    			    	
	#endif		
    	return false;    	
    	
    }
    
    //-------------------------------------------------------------------                                    
    
    void AOZConn::ClearRxBuff()
    {
		while(m_uart1Obj.AvailableForRead())
		{
			byte valRead;
			m_uart1Obj.ReadImpl(&valRead, 1, false);
		}    	    	    
	}

        
    //-------------------------------------------------------------------                                    	
	
#else

    bool AOZConn::WriteRead(byte valWrite, byte &valRead)
    {
    	std::ostringstream log;
    	log<< "0x" <<std::hex << (int)valWrite << " <--->";
    	m_uart1Obj.Send(valWrite);
    	dword startWait = GetSystemTickCount();
    		
   	
    	while((GetSystemTickCount() - startWait) < 2)
    	{
    		if(m_uart1Obj.ReadyToRead())
    		{
    			valRead = m_uart1Obj.Read();
                log<< "0x" <<std::hex << (int)valRead << std::endl;
			#ifdef ZL_TRAFFIC_DEBUG                
		        m_uart0Obj.SendMsg(log.str().c_str());    			
		    #endif		        
    			return true;
    		}
    	}
    	
	#ifdef ZL_TRAFFIC_DEBUG    	
		m_uart0Obj.SendMsg("--\n");    			    	
	#endif
    	return false;    	
    	
    }
    
    //-------------------------------------------------------------------                                    
    
    void AOZConn::ClearRxBuff()
    {
		while(m_uart1Obj.ReadyToRead())
		{
			m_uart1Obj.Read();
		}    	    	    
	}

        
    //-------------------------------------------------------------------                                    	    

#endif    
                
    //-------------------------------------------------------------------                                
    
    void AOZConn::Process()
    {           	        

		iVDK::Sleep(2000);        	     	
    	    	    	
        m_uart0Obj.SendMsg("AOZ Debug Project \n");
        
#ifndef UART1CORE    
        
        m_uart0Obj.SendMsg("Uart1 UartSimple \n");	

#else

        m_uart0Obj.SendMsg("Uart1 UartSimpleCore \n");	

#endif    

#ifndef ZL_INIT_WITHOUT_SYNC
        
        m_uart0Obj.SendMsg("ZL init with TDM sync \n");	

#else

        m_uart0Obj.SendMsg("ZL init without TDM sync \n");	

#endif    

        
		               
        DS28CN01ShowInfo();
                

/*        
        m_uart0Obj.SendMsg((m_extFlash.GetMap().ToString(true)).c_str());                                    
        m_uart0Obj.SendMsg((m_extFlash.GetSignature().ToString(true)).c_str());        
                               		
		{
			m_uart0Obj.SendMsg(BfDev::SysProperties::Instance().
				ToString().c_str());
		}
*/		


	#ifndef ZL_INIT_WITHOUT_SYNC    	
        BfTdm::Tdm  TdmObj(*this, 0, BfTdm::TdmProfile(3,CTDMBlockSize,1, BfTdm::CNoCompanding));		

		TdmObj.Enable();        		
	#endif        

		
		if(DevIpTdm::ZL38065::TryDetect(*this)) m_uart0Obj.SendMsg("ZL38065 discovered \n");
		else m_uart0Obj.SendMsg("ZL38065 not found \n");
		

	    DevIpTdm::ZL38065 *zlEcho = 0;		
		
        try
        {
	        zlEcho = new DevIpTdm::ZL38065(*this);
/*        
	        for (byte i = 0; i< 8; ++i)
	        {
	        	if(!zlEcho.SetMode(i, false, false, false))
	        	{
	        		std::ostringstream log;						
	                log<< "SetMode Error of pair: " << (int)i << " " << std::endl;
			        m_uart0Obj.SendMsg(log.str().c_str());        		
	        	}
	        }
        
	        for (byte i = 8; i< 16; ++i)
	        {
	        	if(!zlEcho.SetMode(i, true, false, false))
	        	{
	        		std::ostringstream log;						
	                log<< "SetMode Error of pair: " << (int)i << " " << std::endl;
			        m_uart0Obj.SendMsg(log.str().c_str());        		
	        	}
	        }
*/	        
        }
        catch(DevIpTdm::ZL38065::DeviceError &e)
        {
			m_uart0Obj.SendMsg("ZL38065 not found \n");        	        		
        }                                                      
		
				        
        
		
//		SecondBFLoadSpi();				
        int verErrors = 0;		
        int writeOper = 0;		        
        int keys = 0;		
        int zlCount = 0;
        int currentChannel = 0;        
        bool cycle = false;
        bool zlChStates[DevIpTdm::CEchoCancellerCount];        
        for(byte i = 0; i< DevIpTdm::CEchoCancellerCount; ++i) zlChStates[i] = false;
        while (1)
        {                               	
			iVDK::Sleep(5);        	     	
	        
	        keys++;
	        zlCount++;
	        
			if(zlCount >= 100 && !cycle)
			{
				zlCount = 0;
				byte pair = currentChannel/2;				
				byte chHi = currentChannel - pair*2;
				bool setChHi;
				bool setChLo;

				if(zlChStates[currentChannel]) zlChStates[currentChannel] = false;
				else zlChStates[currentChannel] = true;				
								
				if(chHi)
				{
					setChLo = zlChStates[currentChannel-1];
					setChHi = zlChStates[currentChannel];					
				}
				else
				{
					setChHi = zlChStates[currentChannel+1];
					setChLo = zlChStates[currentChannel];					
				}
											
        		std::ostringstream log;						
                log<< "ch: " << (int)currentChannel << " "  << BoolToState(zlChStates[currentChannel]) << " ";				
//        		m_uart0Obj.SendMsg("______________________ \n");									
//				writeOper++;
				byte lastPair = pair-1;
				if(pair == 0) lastPair = 15;
//				if(!zlEcho->SetMode(lastPair, false, false, false)) 
//				{
//					verErrors++;
//				}
				
				std::string temp;
				writeOper++;				
				if(!zlEcho->SetMode(pair, false, setChLo, setChHi,temp)) 
				{
					verErrors++;
					log<<  " Verify Error" <<std::endl;
				}
				else log<<  " OK" <<std::endl;

//			#ifdef ZL_TRAFFIC_DEBUG
//		        m_uart0Obj.SendMsg(log.str().c_str());				
//			#endif		        
								
				currentChannel+=5;
				if(currentChannel >= DevIpTdm::CEchoCancellerCount) 
				{
					currentChannel = currentChannel - DevIpTdm::CEchoCancellerCount;
				}
			}	        	        
	        
	        
			if(keys>=100)
			{
				keys = 0;
				DevIpTdm::BfLed::Set(1, DevIpTdm::BfKeys::Get(DevIpTdm::KEY1));				
				
				if (DevIpTdm::BfKeys::Get(0)) 
				{
//					if(DevIpTdm::ZL38065::TryDetect(*this)) m_uart0Obj.SendMsg("ZL38065 discovered \n");
//					else m_uart0Obj.SendMsg("ZL38065 not found \n");
	        		std::ostringstream log;						
	                log<< "===============================" << std::endl;					        		
	                log<< "writeOperation  " << (int)writeOper << "     Verify Errors " << (int)verErrors << std::endl;				
	                log<< "===============================" << std::endl;					        			                
			        m_uart0Obj.SendMsg(log.str().c_str());
					
				}
				
/*				
				if (DevIpTdm::BfKeys::Get(0)) 
				{
				    ShowStatus();
				    if(m_hookA == 0xff && m_hookB == 0xff)
				    {
				        m_callA = 0xff;
				        m_callB = 0xff;				        
				    }
				}
			    else
			    {
			        m_callA = 0;
			        m_callB = 0;				        				        
			    }			
*/			    
			}	        
	    }                
    }
    
    //-------------------------------------------------------------------                            
    
    void AOZConn::SecondBFLoadSpi()
    {
    	    	    
		BfDev::BfSpiLoader spiLoader(m_spiBusMng.GetBfSpi(), m_spiBusMng.GetEchoLoaderBusLock());

		m_uart0Obj.SendMsg("Start Loading\n");						
		
		dword sended = 0;		
	
		for (dword i=0; i < CSlaveBinSize; i += CLoadBlockSize)
		{
			while(m_extFlash.isBusy());
			m_extFlash.Read(CSlaveBinFlashOffset + sended, m_buff, CLoadBlockSize);
			iVDK::Sleep(1);			
			if (!spiLoader.LoadBlock(m_buff, CLoadBlockSize))
			{
		        m_uart0Obj.SendMsg("Error of Loading\n");					    
		        return;
			}
			sended += CLoadBlockSize;
		}										

		m_uart0Obj.SendMsg("End of Loading\n");		
    		
    }                            
            
    //-------------------------------------------------------------------    

	void AOZConn::DS28CN01ShowInfo()
	{
	    {
            BfDev::BfTwi TwiObj;

            try
            {
                Ds28cn01::DS28CN01 SecureChip(TwiObj);
                if(SecureChip.CheckSignature()) m_uart0Obj.SendMsg("Signature DS28CN01 OK\n");
                else m_uart0Obj.SendMsg("Signature DS28CN01 ERROR\n");
                    
                
                
        		std::ostringstream log;						
                log<< "UIN : 0x" <<std::hex << (unsigned long long)SecureChip.GetUIN() << " " << std::endl;
		        m_uart0Obj.SendMsg(log.str().c_str());
		        		                    
                if (SecureChip.isSecretClear()) m_uart0Obj.SendMsg("SecureChip is clear\n");
                else m_uart0Obj.SendMsg("SecureChip is not clear\n");                    

/*
                m_uart0Obj.SendMsg("Writing a secret\n");
                Ds28cn01::SecretBuffStruct secret;	    
                for(int i = 0; i< secret.Size; i++) secret[i]  = 0x00;
                SecureChip.WriteSecret(secret);
                m_uart0Obj.SendMsg("Secret OK\n");	                                

                if (SecureChip.isSecretClear()) m_uart0Obj.SendMsg("SecureChip is clear\n");
                else m_uart0Obj.SendMsg("SecureChip is not clear\n");                    
*/                

            }
            catch(Ds28cn01::DS28CN01::Absent &e)
            {
            	m_uart0Obj.SendMsg("!!!!DS28CN01 Absent!!!!\n");        	
            }
            catch(Ds28cn01::DS28CN01::Error &e)
            {
            	m_uart0Obj.SendMsg("!!!!DS28CN01 Error!!!!\n");        	
            }
            catch(BfDev::BfTwi::TransferTimeout &e)
            {
            	m_uart0Obj.SendMsg("!!!!TWI_TRANSFER_TIMEOUT!!!!\n");        	
            }
            
        } 
	}
	
    //-------------------------------------------------------------------    	
	
	void AOZConn::MakeSync()
	{
		*pTIMER_DISABLE		|= 0x0038;        	    	    
	    *pPORTFIO_INEN		|= 0x8000;
        *pPORTF_FER	|= 0x8070;
        
		*pTIMER4_CONFIG		= 0x00AD;   // 2048 KHz for TimeShift of TDM_CLK
		*pTIMER4_PERIOD		= 8;
		*pTIMER4_WIDTH		= 4;
		
		*pTIMER3_CONFIG		= 0x00A9;   // !4096 KHz for TimeShift of TDM_CLK
		*pTIMER3_PERIOD		= 4;
		*pTIMER3_WIDTH		= 2;

		*pTIMER5_CONFIG		= 0x00AD;  //AOZ_Frame
		*pTIMER5_PERIOD		= 2048;
		*pTIMER5_WIDTH		= 4;

		*pTIMER_ENABLE		|= 0x0038;        				
		
	}

    //-------------------------------------------------------------------    	
		
    void AOZConn::AOZInitBuffers()
    {        
        
  		byte CTRLtable[] = {0x1B,0x08,0xB5,0xCA,0xFF,0,0,0};
        for (word i=0; i<CAOZCTRLBlockSize; i++) m_CTRLbuff[i] = CTRLtable[i];  	        
        
        byte table[] = {0x3a,0xd5,0xba,0xa3,0xba,0xd5,0x3a,0x23};   //1KHz    	        
        
        byte sin_num=0;
        for (word i=0; i<CTDMBlockSize; i++)
        {
			m_DialBuff[i] = table[sin_num];
			sin_num++;
			if (sin_num>7) sin_num =0;
        }        
    }

    //-------------------------------------------------------------------    	
		
    void AOZConn::ShowStatus()
    {        

        std::ostringstream log;						
        log<< "Board Signature : 0x" <<std::hex << (int)m_signature << std::endl;
        log<< "Board Version : 0x" <<std::hex << (int)m_version << std::endl;        
        log<< "Board HookA : 0x" <<std::hex << (int)m_hookA << std::endl;                
        log<< "Board HookB : 0x" <<std::hex << (int)m_hookB << std::endl;                        
        m_uart0Obj.SendMsg(log.str().c_str());
                
    }        
    
            
}
