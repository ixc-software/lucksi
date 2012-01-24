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
#include "TdmFramerT1.h"

namespace BFTdmFramerT1
{    

	
    TdmFramerT1::TdmFramerT1():
        m_uart0Obj(BfDev::SysProperties::Instance().getFrequencySys(), 0, 115200),
        m_iUart0Obj(m_uart0Obj),
		m_uart1Obj(BfDev::SysProperties::Instance().getFrequencySys(), 1, 312500),
		m_iUart1Obj(m_uart1Obj),
//        m_spiBusMng(),
		m_extFlash(m_spiBusMng.CreatePoint( m_spiBusMng.GetFlashBusLock() ), true ),        
        m_processedBlocks(0),        
		m_h0(0),
		m_h1(0),
		m_errSize(0),
		m_errContent(0),
		m_packSize(0),
		m_master(false)
    {         

    }

    //-------------------------------------------------------------------            
    
    void TdmFramerT1::Run()
    {
        
		iVDK::VdkInitDone();    	    	
//        BfDev::SysProperties::InitSysFreq118750kHz();
        BfDev::SysProperties::InitSysFreq133000kHz();
//        BfDev::SysProperties::InitFromProjectOptions();
        
        try
        {
        	BFTdmFramerT1::TdmFramerT1 TdmFramerObj;    
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
    
    //-------------------------------------------------------------------                   

  	void TdmFramerT1::NewBlockProcess(
        word sportNum,
        BfTdm::IUserBlockInterface &m_Ch0,
        BfTdm::IUserBlockInterface &m_Ch1,
        bool collision)
    {        
    	    	
    	
//    	*pPORTFIO_TOGGLE	= 1<<10;    

        word tssize = m_Ch0.GetTSBlockSize();
        byte TSBuff[200];
/*        
        if(m_processedBlocks < 15)
        {
	        m_Ch0.CopyRxTSBlockTo(5, TSBuff, 0, 1);
	        m_CheckSyncBuff[m_processedBlocks *3] = TSBuff[0];
        
	        m_Ch0.CopySyncTxTo(5, TSBuff, 0, 1);
	        m_CheckSyncBuff[m_processedBlocks *3 +1] = TSBuff[0];
	        m_CheckSyncBuff[m_processedBlocks *3 +2] = m_processedBlocks;        
        
	        for(word i = 0; i < tssize; i++) TSBuff[i] = m_processedBlocks;
			m_Ch0.PutTxTSBlockFrom(5, &TSBuff[0], 0, tssize);        
        }
*/
        
                

        for (word timeSlot=0; timeSlot < 24; timeSlot++)
        {
        	
        	m_Ch0.CopyRxTSBlockTo(timeSlot, &TSBuff[0], 0, tssize);
			m_Ch0.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0, tssize);
//        	m_Ch1.CopyRxTSBlockTo(timeSlot, &TSBuff[0], 0,tssize);
//			m_Ch1.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0,tssize);
						
        }
//        for(word i = 0; i < tssize; i++) TSBuff[i] = 0xaa;
//		m_Ch0.PutTxTSBlockFrom(0, &TSBuff[0], 0, tssize);        
		

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
        m_processedBlocks++;
//    	*pPORTFIO_TOGGLE	= 1<<10;            
    }		    	

    //-------------------------------------------------------------------                        
        
    void TdmFramerT1::BlocksCollision(word sportNum)
    {
        while(1);
    }
    
    //-------------------------------------------------------------------                        

    void TdmFramerT1::OnRcvPack(Utils::BidirBuffer *buffer)
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
    
    void TdmFramerT1::SecondBFLoadSpi()
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
    
    void TdmFramerT1::Process()
    {           	        

//        m_master = true;
		iVDK::Sleep(2000);
    	    	    	
        m_uart0Obj.SendMsg("T1 Debug \n");
		        
        PriStatus pstatus;
        BoardDs2155 *isBoard;                       
        
        try
        {
            
             Ds2155::LiuProfile liuProfile;
             liuProfile.SetAsT1();             
             if(m_master) liuProfile.SyncMaster(true);
            
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


        if(m_master)
        {
    		for(byte i=1; i<24; ++i) isBoard->BERT().SetChannelToTest(i);
//            isBoard->BERT().SetChannelToTest(2);
    		isBoard->BERT().Start(isBoard->BERT().LINE);
            
        }
        
		

		{
			std::string boardStr;			
			m_uart0Obj.SendMsg(isBoard->ToString(boardStr).c_str());
		}
		
		{
			m_uart0Obj.SendMsg(BfDev::SysProperties::Instance().
				ToString().c_str());
		}
		
        BfTdm::Tdm  TdmObj(*this, 0, BfTdm::TdmProfile(3,160,1,BfTdm::CNoCompanding));		

		TdmObj.Enable();        		
		
		SecondBFLoadSpi();
				

		BfDev::Leds ledsObj;
			
        		
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
	        keys++;
			if(keys>=200)
			{
				keys = 0;
				DevIpTdm::BfLed::Set(1, DevIpTdm::BfKeys::Get(DevIpTdm::KEY1));
				if (DevIpTdm::BfKeys::Get(0))
				{
//				    SecondBFLoadSpi();
				}
				{
					pstatus = isBoard->LIU().PeekStatus();
				    std::ostringstream log;			        
					log << pstatus.ToString(false) <<  std::endl;				    
				    std::string temp = log.str();
				    m_uart0Obj.SendMsg(temp.c_str());
				}
												
			}
	        

			show++;
			if(show>=500)
			{
				show = 0;
				pstatus = isBoard->LIU().PeekStatus();
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
			    
			}	        	        		
	    }                
    }
            
    //-------------------------------------------------------------------    
        
	void TdmFramerT1::Add(const std::string str)
	{
		m_uart0Obj.SendMsg(str.c_str());
	}
	    
    
            
} //namespace BFLeds
