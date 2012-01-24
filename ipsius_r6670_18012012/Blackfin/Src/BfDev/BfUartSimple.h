#ifndef __UART_SIMPLE__
#define __UART_SIMPLE__

#include "stdafx.h"
#include <string.h>
#include "Platform/platformtypes.h"
#include "utils/IBasicInterface.h"
#include "iUart/IUartIO.h"
#include "Utils/TimerTicks.h"



namespace BfDev
{
    using namespace Platform;

    
	class BfUartSimple : boost::noncopyable, public iUart::IUartIO
	{
		
	public:
	    BfUartSimple(dword sysFreq, byte portNum, dword bitrate);
	    ~BfUartSimple();
	    
 		static BfUartSimple* Instance(byte portNum);
	    
        void Init(unsigned long bitrate);    

        void SendMsg(const char *msg)
		{        	
		    while (*msg !=0) WriteImpl((byte*)msg++, 1);
		}

        // blocked read 
        Platform::byte ReadByte();
	    
    // IUartIOimpl
	public:

        int AvailableForRead() const;
        int AvailableForWrite() const;
        void ResetInboundData();
        void ResetOutboundData();

        int WriteImpl(const void *data, int size);
        int ReadImpl(void *data, int size, bool throwInd);

    private:
    
		void WaitRTS();
    	void WaitData();    
    	
    private:   
    
		byte	m_portNum;
        dword   m_sysFreq;        
		dword 	m_bitrate;        
		
		Utils::TimerTicks	m_timer;
		
	};
			
}; //namespace BfDev

#endif







