#ifndef __ECHO_CHIP__
#define __ECHO_CHIP__

#include "Platform/Platform.h"
#include "BfDev/BfUartSimple.h"
#include "iUart/IUartIO.h"
#include "Utils/TimerTicks.h"
#include "BfTdm/tdm.h"

namespace BFEchoChip
{
    using namespace Platform;

    /*
        EchoChip class
        класс работы BlackFin в режиме Echo Chip
    */
            
	class EchoChip : public boost::noncopyable,
		BfTdm::ITdmObserver
	{
		
	public:		
	
		EchoChip();
        void Process();
        static void Run();

	// ITdmObserver impl
    private:
    	virtual void NewBlockProcess(
    	    word sportNum,
    	    BfTdm::IUserBlockInterface &Ch0,
    	    BfTdm::IUserBlockInterface &Ch1,
    	    bool collision
    	    );    	    
        
        
    private:
       
		BfDev::BfUartSimple m_uart0Obj;
		iUart::IUartIO 		&m_iUart0Obj;		
		BfDev::BfUartSimple m_uart1Obj;
		iUart::IUartIO 		&m_iUart1Obj;				
	    Utils::TimerTicks	m_timer;		
                                		
	};

} //namespace BFEchoChip

#endif

