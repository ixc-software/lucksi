#include "stdafx.h"

#include <cdefBF537.h>
#include <ccblkfn.h>
#include "BfUartSimple.h"

namespace 
{
    enum
    {
    	CTransferMaxTimeMs = 1000,
    	CUartsCount = 2    	
    };
			
	std::vector<BfDev::BfUartSimple*> PUartVector(CUartsCount);
	
};

namespace BfDev
{	
    using Platform::byte;

    // -------------------------------------------------------------------		
		
    BfUartSimple::BfUartSimple(dword sysFreq, byte portNum, dword bitrate):
	m_portNum(portNum),
    m_sysFreq(sysFreq),
	m_bitrate(bitrate)    
    {
    	ESS_ASSERT(portNum < CUartsCount && "Port not exist");
    	
		ESS_ASSERT(!PUartVector[portNum] && "Port Busy");	    	
		PUartVector[portNum] = this;	    		    		    	
    	    	
    	Init(m_bitrate);
    	
    }
    
    // -------------------------------------------------------------------		    
    
    BfUartSimple::~BfUartSimple()
    {
		PUartVector[m_portNum] = 0;
		
    	if (m_portNum)
    	{
			*pPORTF_FER &= ~(word)0x000C;    		
        	*pPORT_MUX |= PFTE;		
    	}
    	else
    	{
			*pPORTF_FER &= ~(word)0x0003;    		    		
        	*pPORT_MUX |= PFDE;        			
    	}
    	    	
    }
    
    //-------------------------------------------------------------------		    
    
 	BfUartSimple* BfUartSimple::Instance(byte portNum)    //static
 	{
 		return PUartVector[portNum];		
 	}
	
    //-------------------------------------------------------------------		
	
    void BfUartSimple::Init(dword bitrate)
    {
        
        volatile int temp = 0;
        int divider = static_cast<int>(m_sysFreq/(16* bitrate));
	        
        if (m_portNum)
        {
        	*pPORTF_FER |= 0x000C;
			*pPORT_MUX &= ~PFTE;        	
        	ssync();			
	    	*pUART1_GCTL = UCEN;	
	    	*pUART1_LCR = DLAB;
	    	*pUART1_DLL = divider;
	    	*pUART1_DLH = divider>>8;
	    	*pUART1_LCR = 0x03;
 	
	    	temp = *pUART1_RBR;
	    	temp = *pUART1_LSR;
	    	temp = *pUART1_IIR;
        	
        }
        else
        {
        	*pPORTF_FER |= 0x0003;
			*pPORT_MUX &= ~PFDE;                 	
        	ssync();			
	    	*pUART0_GCTL = UCEN;	
	    	*pUART0_LCR = DLAB;
	    	*pUART0_DLL = divider;
	    	*pUART0_DLH = divider>>8;
	    	*pUART0_LCR = 0x03;
 	
	    	temp = *pUART0_RBR;
	    	temp = *pUART0_LSR;
	    	temp = *pUART0_IIR;
        }
    }

    // -------------------------------------------------------------------	

    Platform::byte BfUartSimple::ReadByte()
    {
        byte val;
        while(ReadImpl(&val, 1, true) != 1);

        return val;
    }


    // -------------------------------------------------------------------	

    int BfUartSimple::WriteImpl(const void *data, int size)
    {    	    	
        ESS_ASSERT(data != 0);
        ESS_ASSERT(size > 0);

        const byte *pData = static_cast<const byte*>(data);

        int bytesToSend = size;

        while(bytesToSend--)
        {
            // RTS
            WaitRTS();						

            // send
            byte val = *pData++;
            if (m_portNum == 1)	*pUART1_THR = val;
            else				*pUART0_THR = val;
        }
            	
		return size;
    }
        
    //-------------------------------------------------    
    
    int BfUartSimple::ReadImpl(void *data, int size, bool throwInd)
    {    	    	
        ESS_ASSERT(size > 0);

        byte *pData = static_cast<byte*>(data);
        int count = 0;

        while(AvailableForRead() > 0)
        {
        	byte val = (m_portNum == 1) ? *pUART1_RBR : *pUART0_RBR;
            *pData++ = val;    	    	
            ++count;
        }

	    return count;    	
    }    
            
    //-------------------------------------------------        
    
    int BfUartSimple::AvailableForWrite() const
    {    	
    	volatile unsigned short *reg;
    	reg = (m_portNum == 1) ? pUART1_LSR : pUART0_LSR;
    	
    	return (*reg & THRE) ? 1 : 0;    	
		
    }
    
    //-------------------------------------------------            
    
    int BfUartSimple::AvailableForRead() const
    {    	    	
    	volatile unsigned short *reg;
    	reg = (m_portNum == 1) ? pUART1_LSR : pUART0_LSR;
    	return (*reg & DR) ? 1 : 0;    		    	    
    }
        
    //-------------------------------------------------        
    
    void BfUartSimple::WaitRTS()
    {
		if(AvailableForWrite() == 1) return;    	
		m_timer.Set(CTransferMaxTimeMs,0);		
		
		while (AvailableForWrite() == 0)
		{
			ESS_ASSERT(!m_timer.TimeOut() && "UART RTS timeout");			
		}
				
    }

    //-------------------------------------------------        
    
    void BfUartSimple::WaitData()
    {
		if(AvailableForRead() == 1) return;
		m_timer.Set(CTransferMaxTimeMs,0);
		
		while (AvailableForRead() == 0)
		{
			ESS_ASSERT(!m_timer.TimeOut() && "UART WaitData timeout");								
		}
		
    }    
    
    //-------------------------------------------------        
    
    void BfUartSimple::ResetInboundData()
    {
        // nothing
    }

    //-------------------------------------------------        
    
    void BfUartSimple::ResetOutboundData()
    {    	
        // nothing 
    }               
    		                 
} ;//namespace Uart
