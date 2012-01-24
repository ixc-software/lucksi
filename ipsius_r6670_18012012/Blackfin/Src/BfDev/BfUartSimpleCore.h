#ifndef __BFUARTSIMPLECORE__
#define __BFUARTSIMPLECORE__

#include "Platform\PlatformTypes.h"

// must be included outside here
/*
#include <cdefBF537.h>
#include <ccblkfn.h> */

namespace BfDev
{
    using Platform::byte;
    using Platform::word;        
    using Platform::dword;
            
    class BfUartSimpleCore
    {
        volatile word *m_pLSR;
        volatile word *m_pTHR;
        volatile word *m_pRBR;
        
        void InitUart0(int divider)
        {
        	*pPORTF_FER |= 0x0003;
			*pPORT_MUX &= ~PFDE;                 	
        	ssync();			
        	
	    	*pUART0_GCTL = UCEN;	
	    	*pUART0_LCR  = DLAB;
	    	*pUART0_DLL  = divider;
	    	*pUART0_DLH  = divider >> 8;
	    	*pUART0_LCR  = 0x03;
	    	
	    	m_pLSR = pUART0_LSR;
	    	m_pTHR = pUART0_THR;
	    	m_pRBR = pUART0_RBR;
 	
	    	int temp = *pUART0_RBR;
	    	temp = *pUART0_LSR;
	    	temp = *pUART0_IIR;                        
        }
        
        void InitUart1(int divider)
        {            	    	
        	*pPORTF_FER |= 0x000C;
			*pPORT_MUX &= ~PFTE;        	
        	ssync();			
        	
	    	*pUART1_GCTL = UCEN;	
	    	*pUART1_LCR  = DLAB;
	    	*pUART1_DLL  = divider;
	    	*pUART1_DLH  = divider >> 8;
	    	*pUART1_LCR  = 0x03;
	    	
	    	m_pLSR = pUART1_LSR;
	    	m_pTHR = pUART1_THR;
	    	m_pRBR = pUART1_RBR;
	    		    		    	 	
	    	int temp = *pUART1_RBR;
	    	temp = *pUART1_LSR;
	    	temp = *pUART1_IIR;            	            
	    	
        }
        
    public:
        
        BfUartSimpleCore(dword sysFreq, byte portNum, dword bitrate)
        {
  	        ESS_ASSERT(portNum < 2);
            
            int divider = sysFreq / (16 * bitrate);
	        
            if (portNum == 0) InitUart0(divider);
                         else InitUart1(divider);
            
        }
        
        bool ReadyToSend()
        {        	
        	return (*m_pLSR & THRE) ? true : false;             
        }
        
        void Send(byte val)
        {
            while(!ReadyToSend());
            *m_pTHR = val;
        }
        
        void Send(const char *p)
        {
            while(char c = *p++) Send(c);
        }
        
        void Send(const void *p, int count)
        {
            char *pC = (char*)p;
            while(count--)
            {
                Send(*pC++);
            }
        }
        
        bool ReadyToRead()
        {
            return (*m_pLSR & DR) ? true : false;
        }
        
        byte Read()
        {
            while(!ReadyToRead());
            return *m_pRBR;
        }
        
        void Read(void *p, int count)
        {
            char *pC = (char*)p;
            while(count--)            
            {
                *pC++ = Read();
            }
        }
                
    };
    
}  // namespace BfDev

#endif

