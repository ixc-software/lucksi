#ifndef __SPI_BUS_MNG__
#define __SPI_BUS_MNG__

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "Utils/SafeRef.h"
#include "BfSpi.h"
#include "IBfSpiBusLock.h"


namespace BfDev
{
	
	enum
	{
		CDefaultSpiBusSpeed = 10 * 1000 * 1000,
	};
	
	int GetSpiBaud(int speedHz = CDefaultSpiBusSpeed);

    // --------------------------------------------------------
    
    class SpiBusMng;

    class SpiBusPoint : public IBfSpiBusLock  // +copyable
    {
        Utils::SafeRef<SpiBusMng> m_spi; 
        IBfSpiBusLock &m_busLock;

    // IBfSpiBusLock impl
    public:
        void ChipEnable()   { m_busLock.ChipEnable(); }
        void ChipDisable()	{ m_busLock.ChipDisable(); }

    public:

        SpiBusPoint(Utils::SafeRef<SpiBusMng> spi, IBfSpiBusLock &busLock) :
          m_spi(spi), m_busLock(busLock)
        {
            ESS_ASSERT( !spi.IsEmpty() );
        }

        BfSpi& Spi();
        const BfSpi& Spi()const;

    };

    // --------------------------------------------------------
    
    class SpiBusMng : public Utils::SafeRefServer, boost::noncopyable
	{									
    public:
           				
        SpiBusMng(int busSpeedHz);        
        ~SpiBusMng();
        
        BfSpi& GetBfSpi();                             
		
        SpiBusPoint CreatePoint(IBfSpiBusLock &lock)
        {
            return SpiBusPoint(this, lock);
        }
		
	private:
		
		BfDev::BfSpi	m_spi;        
		Platform::dword m_speedHz;								
	};
	
};

#endif
