#ifndef __BOOTMAIN__
#define __BOOTMAIN__

#include "Platform/Platform.h"
#include "DevIpTdm/SpiBusMng.h"
#include "BfDev/BfUartSimple.h"
#include "BfDev/BfUartLoader.h"
#include "BfDev/BfUartLoaderManagement.h"
#include "ExtFlash/M25P128.h"

namespace BFLoader
{
    using namespace Platform;    

    
	class BFSlaveUartLoader : public boost::noncopyable
	{
		
	    enum
	    {
	    	CLoadBlockSize = 256,
	    	CSlaveBinFlashOffset = 0x800000,
	    	CSlaveBinSize = 500000
	    };	
				
	public:
        	
	    BFSlaveUartLoader();
        void Process();
        static void Run();                  
        

    private:    

        BfDev::BfUartSimple     m_uart0Obj;
        BfDev::BfUartSimple     m_uart1Obj;        
		iUart::IUartIO 			&m_iUart1Obj;
        DevIpTdm::SpiBusMng		m_spiBusMng;
        ExtFlash::M25P128	    m_extFlash;                
        
        byte	m_buff[CLoadBlockSize];
		
	};

    // --------------------------------------------------------
    
        
	class ExtFlashLoader : public boost::noncopyable
	{

	public:
        	
	    ExtFlashLoader();
        void Process();
        static void Run();                  
    
    private:        
    
		void WaitSYNC();
		word ReadAddr();
		void SendAnswer(byte c);

    private:

        Platform::byte ReadByte()
        {
            while(!m_iUart0Obj.AvailableForRead());

            Platform::byte val;
            ESS_ASSERT( m_iUart0Obj.Read(val) );
            return val;
        }
    
        BfDev::BfUartSimple     m_uart0Obj;
		iUart::IUartIO 			&m_iUart0Obj;        
 		
        DevIpTdm::SpiBusMng		m_spiBusMng;
        ExtFlash::M25P128	m_extFlash;
        
		byte 			m_buff[256];		        
		byte 			m_flashbuff[2560];
		word			m_page_capacity;
		byte 			m_boot_ver[4];
		byte 			m_boot_id[4];
		
	};

} //namespace BFLoader

#endif

