#ifndef __DS2155_HAL__
#define __DS2155_HAL__

#include "stdafx.h"
#include "Platform\platformtypes.h"
#include "BfDev/BfTimerCounter.h"

namespace Ds2155
{
    using namespace Platform;
    using namespace BfDev;    

    /*
        HAL
		.....
    */

	enum
    {
		CBaseAddress = 0x20300000UL,
    };        
        
    //--------------------------------------------------------------------	        
    
    class IDs2155HAL : public Utils::IBasicInterface
    {
    	
    public:
    		
		virtual byte Read(byte *m_baseAddress, byte addr) = 0;
		virtual void Write(byte *m_baseAddress, byte addr, byte data) = 0;				
		virtual byte ReadStatus(byte *m_baseAddress, byte addr, byte mask) = 0;		
				
    };
    
    //--------------------------------------------------------------------	                
    
	class Ds2155hal  : boost::noncopyable,
		public IDs2155HAL
	{		
		
		BfTimerCounter	m_accessTimer;
		int m_sleepTicks;
									
	// impl IDs2155HAL
	private:		
		byte Read(byte *m_baseAddress, byte addr);
		void Write(byte *m_baseAddress, byte addr, byte data);				
		byte ReadStatus(byte *m_baseAddress, byte addr, byte mask);
		
		void AllDs2155BoardsResetSet();		
		
	public:
	
		Ds2155hal();
						
	};

};

#endif
