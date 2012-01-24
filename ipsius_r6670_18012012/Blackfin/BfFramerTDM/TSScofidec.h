#ifndef __TSS_COFIDEC__
#define __TSS_COFIDEC__

#include "Platform/Platform.h"
#include "iUart/IUartIO.h"
#include "BfDev/BfUartSimple.h"
#include "BfDev/BfUartSimpleCore.h"
#include "BfTdm/tdm.h"
#include "Utils/MemoryPool.h"
//#include "DevIpTdm/DevIpTdmSpiMng.h"
#include "DevIpPult/DevIpPultSpiMng.h"
#include "BfDev/BfSpiLoader.h"
#include "ExtFlash/M25P128.h"
#include "PsbCofidec/Psb4851.h"
#include "PsbCofidec/Psb2170.h"
#include "PsbCofidec/PsbCofidecDrv.h"
#include "PsbCofidec/PsbCofidecInit.h"
#include "iPult/CofidecState.h"
#include "iDSP\FullCicleGen.h"


namespace BfCofidec
{
    using namespace Platform;    
    using namespace  iPult;
	using PsbCofidec::PsbCofidecDrv;
    
	enum
    {        
        CTDMBlockSize = 160,
    };	
    
        
	class PsbCofidecTest : public boost::noncopyable,
		BfTdm::ITdmObserver,
		PsbCofidec::IPsbDebug
	{
	public:
        	
	    PsbCofidecTest();
        void Process();
        static void Run();                           
        
	// ITdmObserver impl
    private:
    	virtual void NewBlockProcess(
    	    word sportNum,
    	    BfTdm::IUserBlockInterface &Ch0,
    	    BfTdm::IUserBlockInterface &Ch1,
    	    bool collision);
    	    
        virtual void BlocksCollision(word sportNum);    	    

	// IPsbDebug impl
	private:
    	virtual void WriteLn(const std::string &line);
    
    private:        

		void ChangeCofidecMode();
		void ChangeCofidecLoopMode();
		void UartControl(byte command);
		void SenseVolControl(byte command);
		void GenControl(byte command);
        void ShowStatus();

    private:
    
		BfDev::BfUartSimple m_uart0Obj;
	    Utils::TimerTicks	m_timer0Obj;				
		PsbCofidec::PsbCofidecDrv	m_cofidec;
		boost::scoped_ptr<iDSP::FullCicleGenOneTone> m_generator;

        word	m_processedBlocks;

	};

} //namespace BfCofidec

#endif

