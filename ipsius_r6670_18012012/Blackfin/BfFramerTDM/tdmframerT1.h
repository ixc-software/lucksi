#ifndef __TDMFRAMERT1__
#define __TDMFRAMERT1__

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


namespace BFTdmFramerT1
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
        
	class TdmFramerT1 : public boost::noncopyable,
		BfTdm::ITdmObserver,
		ILogHdlc
	{
        static const bool CPoolDebug = true;
        static const bool CPoolThreadSafe = false;
        typedef Utils::Detail::ThreadStrategyClass<CPoolThreadSafe> 
        	PoolThreadStrategy;
	public:
        	
	    TdmFramerT1();
        void Process();
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
    	void OnRcvPack(Utils::BidirBuffer *buffer);
    	void SecondBFLoadSpi();


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
		bool            m_master;
		byte 			buff[CLoadBlockSize];		
		
		Ds2155::Hdlc *m_h0;
		Ds2155::Hdlc *m_h1;		

	};

} //namespace BFTdmFramer

#endif

