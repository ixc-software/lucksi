#include "stdafx.h"
#include "keys.h"


namespace
{

    EX_INTERRUPT_HANDLER(PORTF_IntA_ISR)
    {
		*pPORTFIO_SET	= 1<<10;				    	
		BfDev::Keys::PressedInterrupt();
	 	*pPORTFIO_CLEAR = 1<<10;		
    }
    
	int GetKeyPort()    
	{
    	Platform::word port = *pPORTFIO;
//		*pPORTFIO_CLEAR = 0x003C;    	    	    	
		*pPORTFIO_CLEAR = 0x002C;    	    	    	
        return (port>>2) & 0x000f;
	}
	
	
	Platform::byte BufKeys[sizeof(BfDev::Keys)];	
	BfDev::Keys *KeysSelf = 0;
};


namespace BfDev
{
	
	void Keys::Init(IKeyObserver &observer)
	{
    	ESS_ASSERT(KeysSelf == 0);
    			
        KeysSelf = new(BufKeys) Keys(observer);
	}	
	
	//-------------------------------------------------------------	
		
	Keys::Keys(IKeyObserver &observer) :
		m_observer(observer),
        m_lastKey(0)
    {
    	  	            	
//    	*pPORTFIO_INEN		|= 0x003c;		// Pushbuttons 
//    	*pPORTFIO_EDGE		|= 0x003C;
//    	*pPORTFIO_MASKA		|= 0x003C;
    	
    	*pPORTFIO_INEN		|= 0x002c;		// Pushbuttons 
    	*pPORTFIO_EDGE		|= 0x002C;
    	*pPORTFIO_MASKA		|= 0x002C;
    	
        
        // assign core IDs to interrupts
        *pSIC_IAR3 &= 0xFFFF0FFF;
//        *pSIC_IAR3 |= 0x00008000;

                      
//        register_handler(ik_ivg15,PORTF_IntA_ISR);	// PORTF_IntA_ISR -> IVG 12	                       
        register_handler(ik_ivg7,PORTF_IntA_ISR);	// PORTF_IntA_ISR -> IVG 12	                       
        *pSIC_IMASK |= IRQ_PFA_PORTFG;                                
        
    }               
    
	//-------------------------------------------------------------	
	
    void Keys::PressedInterrupt()
    {    	
    	ESS_ASSERT(KeysSelf != 0);

    	int port = GetKeyPort();
        if (port==0) return;        
        
        byte key_num=0;

        while(port)
        {
            key_num++;
            port>>=1;
        }
        
        Instance().Pressed(key_num);
    }    
    
	//-------------------------------------------------------------	
	    
    void Keys::Pressed(int key)
    {
    	m_lastKey = key;
    }
    
	//-------------------------------------------------------------	    
	
	Keys &Keys::Instance()
	{
		ESS_ASSERT(KeysSelf);

		return *KeysSelf;
	}
            
	//-------------------------------------------------------------	    
		
	void Keys::OnTimer()
	{
		if(m_lastKey)
			m_observer.KeyPressed(m_lastKey);
		m_lastKey = 0;
	}	

};//namespace BFKeys









