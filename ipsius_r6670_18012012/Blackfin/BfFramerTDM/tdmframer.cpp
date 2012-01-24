#include "stdafx.h"

#include <ccblkfn.h>
#include <cdefBF537.h>
#include "BfDev/SysProperties.h"
#include "bftdm/tdm.h"
#include "bftdm/tdmtest.h"
#include "DS2155/hdlctest.h"
#include "ds2155/boardds2155.h"
#include "Utils/BidirBuffer.h"
#include "Utils/TimerTicks.h"
#include "iVDK/VdkCheckInit.h"
#include "BfDev/BfTimerCounter.h"
#include "DevIpTdm/BfLed.h"
#include "DevIpTdm/BfKeys.h"
#include "ExtFlash/flashmap.h"
#include "BfDev/BfUartLoader.h"
#include "Ds28cn01/DS28CN01.h"
#include "iUart/IUartIO.h"



#include "leds.h"
#include "TdmFramer.h"

namespace BFTdmFramer
{    

	
    TdmFramer::TdmFramer():
        m_uart0Obj(BfDev::SysProperties::Instance().getFrequencySys(), 0, 115200),
        m_iUart0Obj(m_uart0Obj),
		m_uart1Obj(BfDev::SysProperties::Instance().getFrequencySys(), 1, 312500),
		m_iUart1Obj(m_uart1Obj),
//        m_spiBusMng(),
		m_extFlash(m_spiBusMng.CreatePoint( m_spiBusMng.GetFlashBusLock() ), true),        
        m_processedBlocks(0),        
		m_h0(0),
		m_h1(0),
		m_errSize(0),
		m_errContent(0),
		m_packSize(0),
		m_started()
    {         
        InitASYNC();  
        
        /*
        // Uart probe
        while (true)
        {
            BfDev::Leds::SetLed(BfDev::GREEN);	
	        iVDK::Sleep(250);
	        
	        m_uart0Obj.SendMsg("Test uart. ");
	        
	        BfDev::Leds::SetLed(BfDev::OFF);	        	
	        iVDK::Sleep(250);                    
        } 
        */        
    }

    //-------------------------------------------------------------------            
    
    void TdmFramer::Run()
    {
        
		iVDK::VdkInitDone();    	    	
        BfDev::SysProperties::InitSysFreq118750kHz();
        //BfDev::SysProperties::InitSysFreq133000kHz();
//        BfDev::SysProperties::InitFromProjectOptions();
        
        try
        {
        	BFTdmFramer::TdmFramer TdmFramerObj;    
        	TdmFramerObj.Process();
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
        
        
                       
//        TdmFramerObj.TdmTest();
//        TdmFramerObj.HdlcTest();
        
    }
    
   
    
    //----------------------------------------------------------------------------------

  	void TdmFramer::NewBlockProcess(
        word sportNum,
        BfTdm::IUserBlockInterface &m_Ch0,
        BfTdm::IUserBlockInterface &m_Ch1,
        bool collision)
    {        
    	    	
    	
//    	*pPORTFIO_TOGGLE	= 1<<10;    

/*
        word tssize = m_Ch0.GetTSBlockSize();        
        byte TSBuff[200];
        
        
        if(m_processedBlocks < 15 && m_started)
        {
	        m_Ch0.CopyRxTSBlockTo(5, TSBuff, 0, 1);
	        m_CheckSyncBuff[m_processedBlocks *3] = TSBuff[0];
        
	        m_Ch0.CopySyncTxTo(5, TSBuff, 0, 1);
	        m_CheckSyncBuff[m_processedBlocks *3 +1] = TSBuff[0];
	        m_CheckSyncBuff[m_processedBlocks *3 +2] = m_processedBlocks;        
        
	        for(word i = 0; i < tssize; i++) TSBuff[i] = m_processedBlocks;
			m_Ch0.PutTxTSBlockFrom(5, &TSBuff[0], 0, tssize);        
            m_processedBlocks++;			        			
        }
        
*/
       
/*                

        for (word timeSlot=1; timeSlot < 32; timeSlot++)
        {
        	
        	m_Ch0.CopyRxTSBlockTo(timeSlot, &TSBuff[0], 0, tssize);
			m_Ch0.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0, tssize);
//        	m_Ch1.CopyRxTSBlockTo(timeSlot, &TSBuff[0], 0,tssize);
//			m_Ch1.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0,tssize);
						
        }
        for(word i = 0; i < tssize; i++) TSBuff[i] = 0xaa;
		m_Ch0.PutTxTSBlockFrom(0, &TSBuff[0], 0, tssize);        
*/		

/*        

  		byte table[] = {0x3a,0xd5,0xba,0xa3,0xba,0xd5,0x3a,0x23};   //1KHz    	

        word tssize = m_Ch0.GetTSBlockSize();
        word timeSlot = 2;
        byte TSBuff[200];
        byte sin_num=0;
        for (word i=0; i<200; i++)
        {
			TSBuff[i] = table[sin_num];
			sin_num++;
			if (sin_num>7) sin_num =0;
        }
        	        	               
		m_Ch0.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0, tssize);				        	        	
                
*/

        m_Ch0.UserFree();
        m_Ch1.UserFree();        
//    	*pPORTFIO_TOGGLE	= 1<<10;            
    }		    	

    //-------------------------------------------------------------------                        
        
    void TdmFramer::BlocksCollision(word sportNum)
    {
        while(1);
    }
    
    //-------------------------------------------------------------------                        

    void TdmFramer::OnRcvPack(Utils::BidirBuffer *buffer)
    {

		if (buffer->Size() != m_packSize) 
		{
			std::ostringstream log;			
		    log<< "BAD Size =  " << (int)buffer->Size() << std::endl;                			
			m_uart0Obj.SendMsg(log.str().c_str());
			m_errSize++;
		}
		else 
		{
			bool contentErr = false;
/*		
			for(int i=0; i<buffer->Size(); ++i)
				if ((*buffer)[i] != i) contentErr = true;                
			
			if (contentErr)
			{			
				std::ostringstream log;			
			    log<< "BAD Content" << std::endl;                			
				m_uart0Obj.SendMsg(log.str());				
				m_errContent++;
			}
			
*/			
		}



		std::ostringstream log;						

		log<< "		Dump: ";                		
		while (buffer->Size())
		{
			log<< "0x" <<std::hex << (int)buffer->PopFront() << " ";                
		}
		log << std::endl;
		m_uart0Obj.SendMsg(log.str().c_str());									

		
		delete buffer;				

    }

    //-------------------------------------------------------------------                        
    
    void TdmFramer::HdlcTest()
    {
        BfTdm::Tdm  TdmObj(*this, 0, BfTdm::TdmProfile(3,160,0,BfTdm::CNoCompanding));
        BfTdm::Tdm  TdmObj1(*this, 1, BfTdm::TdmProfile(3,160,1, BfTdm::CNoCompanding));
        
		BfDev::Leds ledsObj;        

		TdmObj.Enable();        		
		TdmObj1.Enable();        					   		            	
    	
    	
		RunHdlcTests(this);    	
    }        
    
    //-------------------------------------------------------------------                        
    
    void TdmFramer::TdmTest()
    {
    	
		int CTestRunMs = 10;	
		std::cout << "Start TDM Test (" << CTestRunMs << "sec )"<< std::endl;	
		CTestRunMs *= 1000;	

		BfTdm::TdmTestResult res;

		BfTdm::TdmTest test(CBaseAddress, CTestRunMs, 8, res);

		while(true)
		{
			iVDK::Sleep(1000);
			if (test.Polling()) break;
		}

		std::cout << res.AsString << std::endl;
		ESS_ASSERT(res.IsPassed);    	    	
    }
    
    //-------------------------------------------------------------------                            
    
    void TdmFramer::SecondBFLoadSpi()
    {
    	    	    
		BfDev::BfSpiLoader spiLoader(m_spiBusMng.GetBfSpi(), m_spiBusMng.GetEchoLoaderBusLock());

		m_uart0Obj.SendMsg("Start Loading\n");						
		
		dword sended = 0;		
	
		for (dword i=0; i < CSlaveBinSize; i += CLoadBlockSize)
		{
			while(m_extFlash.isBusy());
			m_extFlash.Read(CSlaveBinFlashOffset + sended, m_buff, CLoadBlockSize);
			iVDK::Sleep(1);			
			if(!spiLoader.LoadBlock(m_buff, CLoadBlockSize)) 
			{
			    m_uart0Obj.SendMsg("Error BfSpi Load \n");
			    return;
			}
			sended += CLoadBlockSize;
		}										

		m_uart0Obj.SendMsg("End of Loading\n");		
    		
    }                        
    
    //-------------------------------------------------------------------                            
    
    void TdmFramer::SecondBFLoad()
    {
    	    	    
        m_uart0Obj.SendMsg((m_extFlash.GetMap().ToString(true)).c_str());                                    
        m_uart0Obj.SendMsg((m_extFlash.GetSignature().ToString(true)).c_str());        

		BfDev::BfUartLoaderManagement  uartLoaderManager(m_uart1Obj);
		BfDev::BfUartLoader uartLoader(uartLoaderManager);

		if (uartLoader.LoadRequest()) m_uart0Obj.SendMsg("Loader Ack\n");
		else 
		{
			m_uart0Obj.SendMsg("Slave Not Answer\n");
			ESS_ASSERT(0 && "Slave Not Answer");	    	
		}
		
		m_uart0Obj.SendMsg("Start Loading\n");				
		
		dword sended = 0;
		
		for (dword i=0; i < CSlaveBinSize; i += CLoadBlockSize)
		{
			while(m_extFlash.isBusy());
			m_extFlash.Read(CSlaveBinFlashOffset + sended, m_buff, CLoadBlockSize);
			uartLoader.LoadBlock(m_buff, CLoadBlockSize);
			sended += CLoadBlockSize;
		}								
		
		m_uart0Obj.SendMsg("End of Loading\n");		
    		
    }
    
    //-------------------------------------------------------------------                                
    
    void TdmFramer::Process()
    {           	        
        
		iVDK::Sleep(5000);        	     	
    	    	    	
        m_uart0Obj.SendMsg("E1 30 channels loopback \n");
		        
        PriStatus pstatus;
        BoardDs2155 *isBoard;                       
        
        try
        {
            
             Ds2155::LiuProfile liuProfile;          
//             liuProfile.SyncMaster(true);
             liuProfile.Crc4(true);             
             
             
            
			 isBoard = new BoardDs2155(
			 	"Board",
	        	CBaseAddress, 
//				LiuProfile::getDefault(), 
				liuProfile,
				HdlcProfile(256,10,0,256),
				this); 
        }
        catch(Ds2155::DS2155Absent &e)
        {
        	m_uart0Obj.SendMsg("DS2155 BOARD IS ABSENT \n");        	
        	ESS_ASSERT(0 && "DS2155 BOARD IS ABSENT");
        }



        //DS28CN01ShowInfo();

//        m_uart0Obj.SendMsg((m_extFlash.GetMap().ToString(true)).c_str());                                    
//        m_uart0Obj.SendMsg((m_extFlash.GetSignature().ToString(true)).c_str());        
       

                        

//		isBoard->EnableLoopBack();

//		for(byte i=1; i<16; ++i) isBoard->BERT().SetChannelToTest(i);
//		isBoard->BERT().Start(isBoard->BERT().LINE);

		
		m_packSize = 3;
		
/*

		m_h0 = isBoard->AllocHdlc(16,
									HdlcLogProfile(true,
//									CTransivedTransActSize
//									CTxPollingEnabled|
									CTxUserLevelEnabled|
//									CTxFSMEnabled|
//									CRxPollingEnabled|
									CRxUserLevelEnabled //|
//									CRxFSMEnabled
									));

*/
/*
		m_h1 = isBoard->AllocHdlc(15,
									HdlcLogProfile(true,
									CTxPollingEnabled|
									CTxUserLevelEnabled|
									CTxFSMEnabled|
									CRxPollingEnabled|
									CRxUserLevelEnabled|
									CRxFSMEnabled
									));
																		
*/									
									
//		m_h1 = isBoard->AllocHdlc(15);								

		{
			std::string boardStr;			
			m_uart0Obj.SendMsg(isBoard->ToString(boardStr).c_str());
		}
		
		{
			m_uart0Obj.SendMsg(BfDev::SysProperties::Instance().
				ToString().c_str());
		}
		
        const int CBlockSize = 160;
        const int CBlockCount = 3;
        		
        BfTdm::Tdm  TdmObj(*this, 0, BfTdm::TdmProfile(CBlockCount, CBlockSize, 1, BfTdm::CNoCompanding));	                
        
                       
		
		TdmObj.Enable();        		
//		TdmObj1.Enable();        					   		        
		
//		SecondBFLoad();		
		SecondBFLoadSpi();				
		iVDK::Sleep(10000);
		m_started = true;
        m_uart0Obj.SendMsg("Check Sync Started \n");		

		BfDev::BfTimerCounter accessTimer(BfTimerCounter::GetFreeTimer());
		BfDev::Leds ledsObj;
		
//		BfDev::BfTwi tWIObj(BfDev::SysProperties::Instance().getFrequencySys());
		
        		
		ErrorFlags errStruct;
		int spackCounter = 0;
		int sended = 0;
		int show = 0;
		int keys = 0;
		dword errnum = 0;
		bool pattern = false;
		
        		        
		for(word i=0; i<256; ++i) buff[i]=i;						
		       
        while (1)
        {                               	
			iVDK::Sleep(5);        	     	
			spackCounter++;
//            ledsObj.LedToggle();	                           			
//            BfDev::Keys::Instance().OnTimer();            
	        isBoard->HdlcPolling();	        	        	        	        
	        
	        keys++;
			if(keys>=200)
			{
				keys = 0;
				DevIpTdm::BfLed::Set(1, DevIpTdm::BfKeys::Get(DevIpTdm::KEY1));
/*				
				if (DevIpTdm::BfKeys::Get(0))
				{
				    SecondBFLoadSpi();
	//				tWIObj.WriteData(0, 0x55);
//					tWIObj.ReadData(0x55);										
				}

				{
					pstatus = isBoard->LIU().PeekStatus();
				    std::ostringstream log;			        
					log << pstatus.ToString(false) <<  std::endl;				    
				    std::string temp = log.str();
				    m_uart0Obj.SendMsg(temp.c_str());
				}
*/				
												
/*				
				pstatus = isBoard->LIU().PeekStatus();				
				if(pstatus.rxLSS != !pattern) 
				{
					pattern = !pstatus.rxLSS;
					if(pattern)	m_uart0Obj.SendMsg("Sync OK");
					else m_uart0Obj.SendMsg("Sync Fail");
				}
				
				dword temp = isBoard->BERT().UpdateErrBitCount();
				if(errnum!=temp)
				{
					errnum = temp;
				    std::ostringstream log;			        
					log<< "BERT ERR " << (unsigned int)errnum << std::endl;                							
				    m_uart0Obj.SendMsg(log.str().c_str());										
				}
    			m_uart0Obj.SendMsg(".");														
*/				
				
				if (DevIpTdm::BfKeys::Get(0))
				{	
									
					//TdmObj.Disable();
										
					iVDK::Sleep(200);	
					
					/*
				    						
				    */
				    
				    /*
					for(int i = 0; i<45; i=i+3) 
					{						
						//log << m_CheckSyncBuff[i] << "  " << m_CheckSyncBuff[i+1] << "  " << m_CheckSyncBuff[i + 2]<< std::endl;						
					}
					*/
					
					/*					
					m_uart0Obj.SendMsg(log.str().c_str());						
					m_uart0Obj.SendMsg("trace\n");					
					*/
				}
				
			}
	        

			show++;
			if(show>=500)
			{
				show = 0;
/*				
			    std::ostringstream log;			        
				log<< "Sended     " << sended << " packets \r" << std::endl;
			    m_uart0Obj.SendMsg(log.str().c_str());
*/
				pstatus = isBoard->LIU().PeekStatus();
//				if (pstatus.rxSignalLevel > 2)	BfDev::Leds::SetLed(BfDev::RED);
//				else 							BfDev::Leds::SetLed(BfDev::GREEN);
				if (pstatus.rxSignalLevel > 2)	
				{
					DevIpTdm::BfLed::Set(0, false);
					BfDev::Leds::SetLed(BfDev::RED);					
				}
				else
				{
					DevIpTdm::BfLed::Set(0, true);
					BfDev::Leds::SetLed(BfDev::GREEN);
				}
///*				
/*
			    std::ostringstream log;			        

				log<< pstatus.ToString(true) << std::endl;                
				log<< "BERT ERR " << isBoard->BERT().UpdateErrBitCount() << std::endl;                							
			    m_uart0Obj.SendMsg(log.str().c_str());
*/			    
//*/			    
			    
			}
	        	        		
	        if(m_h0)
	        {	        	
	        	if(spackCounter > 500)
	        	{

	        		spackCounter = 0;
					sended++;
									
/*	        		
			    	isBoard->LIU().UpdateBitsState();
				short state = isBoard->LIU().UpdateBitsState();

				
			    std::ostringstream log;			        

				log<< "LIU status is:" << std::endl;                

				log<< (bool)(state & 0x40) << "  " << (bool)(state & 0x10) << "  ";
			    													
				log<< (bool)(state & 0x04) << "  " << (bool)(state & 0x01) << "  " << std::endl;					
				log<< (bool)(state & 0x80) << "  " << (bool)(state & 0x20) << "  ";
				log<< (bool)(state & 0x08) << "  " << (bool)(state & 0x02) << "  " << std::endl;
			    m_uart0Obj.SendMsg(log);                            																				
					
*/			    
//					m_h0->SendPack(buff,m_packSize);
//					m_h0->SendPack(buff,m_packSize);
//					m_h0->SendPack(buff,m_packSize);
	        	}	        	

				while (m_h0->isRxPack())
					OnRcvPack(m_h0->GetPack());				
					
				errStruct = m_h0->PeekErrors();		
				if (errStruct.HasErrors())
					m_uart0Obj.SendMsg(errStruct.ToString().c_str());
					
	        }				
        
	        if(m_h1)
	        {	        
	        	
				while (m_h1->isRxPack())
					OnRcvPack(m_h1->GetPack());				
	        		        	
				errStruct = m_h1->PeekErrors();		
				if (errStruct.HasErrors())
					m_uart0Obj.SendMsg(errStruct.ToString().c_str());
					
				while (m_h0->isRxPack())
					OnRcvPack(m_h0->GetPack());									
	        }																									

	    }                
    }
            
    //-------------------------------------------------------------------
    
    void TdmFramer::InitASYNC()
    {
        
		*pEBIU_AMGCTL = 0x00FA; // Bank 0,1,2 for Flash memory, bank 3 for DS2155
		*pEBIU_AMBCTL1 = (*pEBIU_AMBCTL1 & 0x0000FFFF) | 0xffc20000;
		ssync();
      
    }

    //-------------------------------------------------------------------    
        
	void TdmFramer::Add(const std::string str)
	{
		m_uart0Obj.SendMsg(str.c_str());
	}
    
    //-------------------------------------------------------------------    

	void TdmFramer::DS28CN01ShowInfo()
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
    
            
} //namespace BFLeds
