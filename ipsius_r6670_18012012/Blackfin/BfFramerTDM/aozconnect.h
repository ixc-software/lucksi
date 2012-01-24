#ifndef __AOZCONN__
#define __AOZCONN__

#include "Platform/Platform.h"
#include "iUart/IUartIO.h"
#include "BfDev/BfUartSimple.h"
#include "BfDev/BfUartSimpleCore.h"
#include "BfTdm/tdm.h"
#include "Utils/MemoryPool.h"
#include "DevIpTdm/DevIpTdmSpiMng.h"
#include "DevIpTdm/ZL38065.h"
#include "BfDev/BfSpiLoader.h"
#include "ExtFlash/M25P128.h"


//#define UART1CORE
//#define ZL_INIT_WITHOUT_SYNC
//#define ZL_TRAFFIC_DEBUG

namespace BfAOZConn
{
    using namespace Platform;

    
	enum
    {
        
        CDialToneBlockSize = 8,        
        CAOZCTRLBlockSize = 8,
        
        
        CTDMBlockSize = 160,
        
        
    	CLoadBlockSize = 512,
    	CSlaveBinFlashOffset = 0xc800,
    	CSlaveBinSize = 30000
    	
//    	CSlaveBinFlashOffset = 0x800000,
//    	CSlaveBinSize = 550000
    };	
    
        
	class AOZConn : public boost::noncopyable,
		BfTdm::ITdmObserver,
		DevIpTdm::IZl38065Transport
	{
	public:
        	
	    AOZConn();
        void Process();
        static void Run();        
           

	// IZl38065Transport impl
    private:    
        virtual bool WriteRead(byte valWrite, byte &valRead);        
        virtual void ClearRxBuff();        
        
        
	// ITdmObserver impl
    private:
    	virtual void NewBlockProcess(
    	    word sportNum,
    	    BfTdm::IUserBlockInterface &Ch0,
    	    BfTdm::IUserBlockInterface &Ch1,
    	    bool collision);
    	    
        virtual void BlocksCollision(word sportNum);    	    
    
    private:        
    	void SecondBFLoadSpi();
	    void DS28CN01ShowInfo();
	    void MakeSync();	    
        void AOZInitBuffers();
        void ShowStatus();

    private:
    
		BfDev::BfUartSimple m_uart0Obj;
		iUart::IUartIO 		&m_iUart0Obj;
	    Utils::TimerTicks	m_timer0Obj;				

	#ifndef	 UART1CORE
	    	    
        BfDev::BfUartSimple     m_uart1Obj;        
		iUart::IUartIO 			&m_iUart1Obj;
	#else

        BfDev::BfUartSimpleCore m_uart1Obj;        
    #endif
        

        DevIpTdm::DevIpTdmSpiMng m_spiBusMng;
        ExtFlash::M25P128		m_extFlash;                
        
        byte	m_buff[CLoadBlockSize];
        int		m_CheckSyncBuff[100];
	    	    	    

        word            m_processedBlocks;
		byte 			m_CTRLbuff[CAOZCTRLBlockSize];
		byte 			m_DialBuff[CTDMBlockSize];		
        byte 			m_signature;
        byte 			m_version;
        byte 			m_hookA;        
        byte 			m_hookB;                
        byte            m_callA;
        byte            m_callB;

	};

} //namespace BfAOZConn

#endif

