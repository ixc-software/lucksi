#include "stdafx.h"

#include <ccblkfn.h>
#include <cdefBF537.h>

#include "BfDev/SysProperties.h"
#include "Utils/TimerTicks.h"
#include "iVDK/VdkCheckInit.h"
#include "BfDev/BfTimerCounter.h"
#include "DevIpTdm/BfLed.h"
#include "ExtFlash/flashmap.h"
#include "BfDev/BfUartLoader.h"
#include "iUart/IUartIO.h"


#include "bootmain.h"

namespace BFLoader
{    

    //-------------------------------------------------------------------            		
	
    enum
    {
		COM_SYNC					= 0xFF,
		COM_EXIT                    = 0xFE,
		COM_ERROR                   = 0x80,

		COM_READ_BUFF               = 0x01,
		COM_READ_TO_BUFF            = 0x02,
		COM_WRITE_BUFF              = 0x03,
		COM_WRITE_FROM_BUFF         = 0x04,

		COM_GET_ID                  = 0x10,
		COM_SET_SPEED               = 0x11,
		COM_BOOT_VER                = 0x12,
    
		COM_RESET                   = 0x20,

		COM_EF_CHECK                = 0x30,
		COM_EF_READ_TO_BUFF         = 0x31,
		COM_EF_WRITE_FROM_BUFF      = 0x32,

		COM_TRANSFER_ON             = 0x40,
		COM_TRANSFER_OFF            = 0x41,
		COM_EXT_TRANSF_NOPE         = 0x45,
		COM_EXT_TRANSF_MODE1        = 0x46,
		COM_EXT_TRANSF_MODE2        = 0x47,
  
		ECOM_GET_INFO               = 0x60,
		ECOM_DEVELOP                = 0x61,
		ECOM_WRITE_TO_BUFF          = 0x63,
		ECOM_WRITE_FROM_BUFF        = 0x64,
		ECOM_WRITE_EEPROM           = 0x65    	
    };       	
           

    //-------------------------------------------------------------------                
	
    BFSlaveUartLoader::BFSlaveUartLoader():
        m_uart0Obj(BfDev::SysProperties::Instance().getFrequencySys(), 0, 115200),
		m_uart1Obj(BfDev::SysProperties::Instance().getFrequencySys(), 1, 599040),
		m_iUart1Obj(m_uart1Obj),
        m_spiBusMng(BfDev::SysProperties::Instance().getFrequencySys(), 10000),
		m_extFlash(m_spiBusMng.GetBfSpi(), m_spiBusMng.GetFlashBusLock())
    {                 
    	
    }    
    
    //-------------------------------------------------------------------            
    
    void BFSlaveUartLoader::Run()
    {        
		iVDK::VdkInitDone();    	
        BfDev::SysProperties::InitSysFreq133000kHz();
       
        try
        {
	        BFLoader::BFSlaveUartLoader secLoaderObj;		
        	secLoaderObj.Process();	        
        }
        catch(ExtFlash::M25P128Absent &e)
        {
        	while(1)
        	{
	        	DevIpTdm::BfLed::SetColor(DevIpTdm::RED);	
	        	iVDK::Sleep(300);
	        	
	        	DevIpTdm::BfLed::SetColor(DevIpTdm::OFF);		        	
	        	iVDK::Sleep(300);	        	
        	}
        	ESS_ASSERT(0 && "M25P128 not found");
        }       
    }               
    
    //-------------------------------------------------------------------                            
        
    void BFSlaveUartLoader::Process()
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
		
		while(true)
		{			
	        	DevIpTdm::BfLed::SetColor(DevIpTdm::YELLOW);	
	        	iVDK::Sleep(300);
	        	
	        	DevIpTdm::BfLed::SetColor(DevIpTdm::OFF);	        	
	        	iVDK::Sleep(100);	        							
		}										
		
    }    
    		
    //-------------------------------------------------------------------                
	
    ExtFlashLoader::ExtFlashLoader():
        m_uart0Obj(BfDev::SysProperties::Instance().getFrequencySys(), 0,19200),
		m_iUart0Obj(m_uart0Obj),
        m_spiBusMng(BfDev::SysProperties::Instance().getFrequencySys(), 10000),
		m_extFlash(m_spiBusMng.GetBfSpi(), m_spiBusMng.GetFlashBusLock())
    {         
		m_page_capacity = 256;
    }        
    
    //-------------------------------------------------------------------            
    
    void ExtFlashLoader::Run()
    {
        
		iVDK::VdkInitDone();    	    	
        BfDev::SysProperties::InitSysFreq118750kHz();

        try
        {
	        BFLoader::ExtFlashLoader mainLoaderObj;		
        	mainLoaderObj.Process();	        
        }
        catch(ExtFlash::M25P128Absent &e)
        {
        	while(true)
        	{
	        	DevIpTdm::BfLed::SetColor(DevIpTdm::RED);	
	        	iVDK::Sleep(300);
	        	DevIpTdm::BfLed::SetColor(DevIpTdm::OFF);	        	
	        	iVDK::Sleep(300);	        	
        	}
        	
        	ESS_ASSERT(0 && "M25P128 not found");
        }       
    }
            
    //-------------------------------------------------------------------                            

	void ExtFlashLoader::WaitSYNC()
	{
 		byte c = ReadByte();
	    if (c == COM_SYNC) m_iUart0Obj.Write(c);	    
	}
	
    //-------------------------------------------------------------------                            	
	
	word ExtFlashLoader::ReadAddr()
	{
		byte data = ReadByte();
		word addr = data;
		data = ReadByte();		
		addr |= (word)data <<8;
		return addr;
	}
	
    //-------------------------------------------------------------------                            	
	
	void ExtFlashLoader::SendAnswer(byte c)
	{
		m_iUart0Obj.Write(c);
	}
    	
    //-------------------------------------------------------------------                                
        
    void ExtFlashLoader::Process()
    {           	        
/*										
		{
			m_uart0Obj.SendMsg((BfDev::SysProperties::Instance().ToString()).c_str());
		}
*/						

        m_uart0Obj.SendMsg((m_extFlash.GetMap().ToString(true)).c_str());                                    
        m_uart0Obj.SendMsg((m_extFlash.GetSignature().ToString(true)).c_str());                                            


		// BfDev::Leds ledObj;
//		BfDev::BfTimerCounter accessTimer(BfDev::BfTimerCounter::GetFreeTimer());        		
//		for(word i=0; i<256; ++i) m_buff[i]=i;						
		       
		WaitSYNC();  	  		
	  
		while(1)
		{			
			
			byte c = ReadByte();
      
			// answer on 'ping'
			if (c == COM_SYNC)
			{
				m_iUart0Obj.Write(c);
			}
          
			// send self id
			if (c == COM_GET_ID)
			{
				DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN);				
				SendAnswer(c);
				for (int i = 0; i < 4; i++) m_iUart0Obj.Write(m_boot_id[i]);

			}

			// boot version
			if (c == COM_BOOT_VER)
			{         
				SendAnswer(c);
				for (int i = 0; i < 4; i++) m_iUart0Obj.Write(m_boot_ver[i]);
			}
		
			// send buff to PC
			if (c == COM_READ_BUFF)
			{
				DevIpTdm::BfLed::SetColor(DevIpTdm::RED);				
				// echo command
				SendAnswer(c);

				// send buff
				int sum = 0;
				for (int j = 0; j < m_page_capacity; j++)
				{
					m_iUart0Obj.Write(m_buff[j]);
					sum+= m_buff[j];
				}
				m_iUart0Obj.Write(sum & 0xFF);
				DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN);				
			}		
		
			// UART -> buff
			if (c == COM_WRITE_BUFF)
			{
				DevIpTdm::BfLed::SetColor(DevIpTdm::RED);								
				// read 256 byte to buff
				int sum = 0;
				for (int j = 0; j < m_page_capacity; j++)
				{
					m_buff[j] = ReadByte();
					sum+= m_buff[j];
				}
				
				byte data = ReadByte();				
				if (data == (sum & 0xFF))
				{      
					m_iUart0Obj.Write(c);
				}
				else
				{
					m_iUart0Obj.Write(COM_ERROR);        
				}  
				DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN);				
			}
				    
			// do reset
			if (c == COM_RESET)
			{
				SendAnswer(c);
			}

			// set speed
			if (c == COM_SET_SPEED)
			{
			
				int my_id = ReadAddr();
				my_id += ((dword)ReadAddr() << 16);
         
				SendAnswer(c);

				iVDK::Sleep(5);			
				m_uart0Obj.Init(my_id); 				
			
			}
   
			if (c == COM_EF_CHECK)
			{
				SendAnswer(c);
				m_iUart0Obj.Write(!m_extFlash.isBusy());
				if (!m_extFlash.isBusy()) DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN);
			}
	
	        if (c == COM_EF_READ_TO_BUFF)
	        {
				DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN);	        	
				int addr = ReadAddr();
				m_extFlash.Read(addr*256, m_buff, 256);
	           	SendAnswer(c);
	        }
		
			if (c == COM_EF_WRITE_FROM_BUFF)
			{
				DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN);				
				int addr = ReadAddr();
				bool wrok=true;				
				if(!m_extFlash.Write(addr << 8, m_buff, 256, true)) wrok=false;
				if (wrok) SendAnswer(c);
				else SendAnswer(COM_ERROR);
			}
                          
			if ((c & 0xF0) == COM_TRANSFER_ON)
			{
				DevIpTdm::BfLed::SetColor(DevIpTdm::RED);
				m_extFlash.ChipErase();
//				m_extFlash.SectorErase(1);
				SendAnswer(COM_ERROR);        
			}           

		}        	        	
    }      
} //namespace BFLoader
