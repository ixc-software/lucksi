#include "stdafx.h"

#include <ccblkfn.h>
#include <cdefBF537.h>
#include "BfDev/SysProperties.h"
#include "bftdm/tdm.h"
#include "Utils/BidirBuffer.h"
#include "Utils/TimerTicks.h"
#include "iVDK/VdkCheckInit.h"
#include "BfDev/BfTimerCounter.h"
#include "DevIpTdm/bfled.h"
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
    
    // -------------------------------------------------------------------                   

    /*
    // Для плат с маскированием
  	void EchoChip::NewBlockProcess(
        word sportNum,
        BfTdm::IUserBlockInterface &m_Ch0,
        BfTdm::IUserBlockInterface &m_Ch1,
        bool collision
        )
    {   
    	if (collision) return;
    	
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

        m_Ch0.UserFree();
        m_Ch1.UserFree();        
    }
    */
    
    	
    
    
    // x - copy
    void EchoChip::NewBlockProcess(
        word sportNum,
        BfTdm::IUserBlockInterface &m_Ch0,
        BfTdm::IUserBlockInterface &m_Ch1,
        bool collision)
    {   
    	if (collision) return;         
    		
        word tssize = m_Ch0.GetTSBlockSize();
        byte TSBuff0[200];
        byte TSBuff1[200];
        
        for (word timeSlot=1; timeSlot < 32; timeSlot++)
        {
        	m_Ch0.CopyRxTSBlockTo(timeSlot, &TSBuff0[0], 0, tssize);
        	m_Ch1.CopyRxTSBlockTo(timeSlot, &TSBuff1[0], 0, tssize);
        
        	m_Ch0.PutTxTSBlockFrom(timeSlot, &TSBuff1[0], 0, tssize);
        	m_Ch1.PutTxTSBlockFrom(timeSlot, &TSBuff0[0], 0, tssize);        	      	                        
        }
        

        m_Ch0.UserFree();
        m_Ch1.UserFree();        
    }
        	

    // -------------------------------------------------------------------                        
                                 
    
    void EchoChip::Process()
    {           	        
        
        
        m_uart1Obj.SendMsg("BlackFin Echo V1.0 \n");        
									
		{
			m_uart1Obj.SendMsg((BfDev::SysProperties::Instance().
				ToString()).c_str());
		}

		
		BfTdm::TdmProfile profile(3, 160, true, BfTdm::CNoCompanding);
        BfTdm::Tdm  TdmObj(*this, 0, profile);		

		TdmObj.Enable();        					       				        		      
		       
        while (true)
        {                               	
			DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN);        	
			iVDK::Sleep(500);        	     	
//			BfDev::Leds::SetLed(BfDev::RED);			
//			iVDK::Sleep(100);        	     	
			DevIpTdm::BfLed::SetColor(DevIpTdm::OFF);			
			iVDK::Sleep(300);        	     							
	    }                
    }
            
    // -------------------------------------------------------------------            
    
    void EchoChip::Run()
    {        
        EchoChip EchoChipObj;                            
        EchoChipObj.Process();
        
    }

                    
} //namespace BFEchoChip
