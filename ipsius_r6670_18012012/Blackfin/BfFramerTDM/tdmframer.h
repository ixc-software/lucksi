#ifndef __TDMFRAMER__
#define __TDMFRAMER__

#include "Platform/Platform.h"
#include "iUart/IUartIO.h"
#include "BfDev/BfUartSimple.h"
#include "BfTdm/tdm.h"
#include "DS2155/boardds2155.h"
#include "Utils/MemoryPool.h"
#include "DevIpTdm/DevIpTdmSpiMng.h"
#include "BfDev/BfUartLoader.h"
#include "BfDev/BfUartLoaderManagement.h"
#include "BfDev/BfSpiLoader.h"
#include "ExtFlash/M25P128.h"



namespace BFTdmFramer
{
    using namespace Platform;
    using namespace Ds2155;            

    
	enum
    {
    	CLoadBlockSize = 512,
    	CSlaveBinFlashOffset = 0xc800,
    	CSlaveBinSize = 30000
    	
//    	CSlaveBinFlashOffset = 0x800000,
//    	CSlaveBinSize = 550000
    };	

    
    /*
        TdmFramer class
        .....
    */
        
	class TdmFramer : public boost::noncopyable,
		BfTdm::ITdmObserver,
		ILogHdlc
	{
        static const bool CPoolDebug = true;
        static const bool CPoolThreadSafe = false;
        typedef Utils::Detail::ThreadStrategyClass<CPoolThreadSafe> 
        	PoolThreadStrategy;
	public:
        	
	    TdmFramer();
        void Process();
        void TdmTest();        
        void HdlcTest();        
        static void Run();
        
           
	// ILogHdlc impl
    private:    
		virtual void Add(const std::string str);
		
		

	// ITdmObserver impl
    private:
    	virtual void NewBlockProcess(
    	    word sportNum,
    	    BfTdm::IUserBlockInterface &Ch0,
    	    BfTdm::IUserBlockInterface &Ch1,
    	    bool collision);
    	    
        virtual void BlocksCollision(word sportNum);    	    

    
    private:        
        void InitASYNC();
    	void OnRcvPack(Utils::BidirBuffer *buffer);
    	void SecondBFLoad();    	
    	void SecondBFLoadSpi();
	    void DS28CN01ShowInfo();    
	    

    private:
    
		BfDev::BfUartSimple m_uart0Obj;
		iUart::IUartIO 		&m_iUart0Obj;
	    Utils::TimerTicks	m_timer0Obj;				
	    
        BfDev::BfUartSimple     m_uart1Obj;        
		iUart::IUartIO 			&m_iUart1Obj;
        DevIpTdm::DevIpTdmSpiMng m_spiBusMng;
        ExtFlash::M25P128		m_extFlash;                
        
        byte	m_buff[CLoadBlockSize];
        int		m_CheckSyncBuff[100];
	    	    	    

        word            m_processedBlocks;
		int 			m_errSize;
		int 			m_errContent;		
		int 			m_packSize;				
		byte 			buff[CLoadBlockSize];		
		bool            m_started;
		
		Ds2155::Hdlc *m_h0;
		Ds2155::Hdlc *m_h1;		

	};

} //namespace BFTdmFramer

#endif

