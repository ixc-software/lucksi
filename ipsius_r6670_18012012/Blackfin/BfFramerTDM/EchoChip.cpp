#include "stdafx.h"

#include <ccblkfn.h>
#include <cdefBF537.h>
#include "BfDev/SysProperties.h"
#include "bftdm/tdm.h"
#include "bftdm/tdmtest.h"
#include "leds.h"
#include "Utils/BidirBuffer.h"
#include "Utils/TimerTicks.h"
#include "iVDK/VdkCheckInit.h"
#include "BfDev/BfTimerCounter.h"
#include "EchoChip.h"

namespace BFEchoChip
{    
	
    EchoChip::EchoChip():
        m_uart0Obj(BfDev::SysProperties::Instance().getFrequencySys(), 0, 115200),
        m_iUart0Obj(m_uart0Obj),
        m_uart1Obj(BfDev::SysProperties::Instance().getFrequencySys(), 1, 115200),
        m_iUart1Obj(m_uart1Obj)        
    {         
    	
    }

    //-------------------------------------------------------------------            
    
    void EchoChip::Run()
    {
        
		iVDK::VdkInitDone();    	    	
        BfDev::SysProperties::InitSysFreq133000kHz();

        EchoChip EchoChipObj;                            
        EchoChipObj.Process();
        
    }
    
    //-------------------------------------------------------------------                   

  	void EchoChip::NewBlockProcess(
        word sportNum,
        BfTdm::IUserBlockInterface &m_Ch0,
        BfTdm::IUserBlockInterface &m_Ch1,
        bool collision)
    {        
    	

    	*pPORTFIO_TOGGLE	= 1<<10;    

        word tssize = m_Ch0.GetTSBlockSize();
        byte TSBuff[200];
        
        
		for(int k = 0; k < tssize; k++) TSBuff[k] = 0xaa;        
		m_Ch1.PutTxTSBlockFrom(0, &TSBuff[0], 0,tssize);				        			
        	        	               
        for (word timeSlot=1; timeSlot < 32; timeSlot++)
        {
        	
        	m_Ch0.CopyRxTSBlockTo(timeSlot, &TSBuff[0], 0, tssize);
			m_Ch1.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0,tssize);				        	
			
			for(int k = 0; k < tssize; k++) TSBuff[k] = 0xff;
//        	m_Ch1.CopyRxTSBlockTo(timeSlot, &TSBuff[0], 0,tssize);			
			m_Ch0.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0, tssize);				        	        	

						
        }            	                        

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
        
    void EchoChip::BlocksCollision(word sportNum)
    {
        while(1);
    }
    
    //-------------------------------------------------------------------                            
    
    void EchoChip::Process()
    {           	        
        m_uart1Obj.SendMsg("BlackFin Echo V1.0 \n");                                    
									
		{
			m_uart1Obj.SendMsg((BfDev::SysProperties::Instance().
				ToString()).c_str());
		}
		
        BfTdm::Tdm  TdmObj(*this, 0, BfTdm::TdmProfile(3,160,1,BfTdm::CNoCompanding));		

		TdmObj.Enable();        			
		
       				
        		        
		for(word i=0; i<256; ++i) buff[i]=i;						
		       
        while (1)
        {                               	
			BfDev::Leds::SetLed(BfDev::GREEN);        	
			iVDK::Sleep(100);        	     	
			BfDev::Leds::SetLed(BfDev::RED);			
			iVDK::Sleep(100);        	     	
			BfDev::Leds::SetLed(BfDev::YELLOW);			
			iVDK::Sleep(100);        	     							
	    }                
    }
    
    //-------------------------------------------------------------------
            
} //namespace BFEchoChip
