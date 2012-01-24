#ifndef __IPPULT_SPI_BUS_MNG__
#define __IPPULT_SPI_BUS_MNG__

#include "stdafx.h"
#include "Platform/platformtypes.h"
#include "BfDev/SpiBusMng.h"
#include "BfDev/IBfSpiBusLock.h"
#include "BfDev/BfSpi.h"


namespace DevIpPult
{

    
	class DevIpPultSpiMng : public BfDev::SpiBusMng
	{									
    public:

        enum
        {
            CDefaultSpiBusSpeed = 2 * 1000 * 1000,
        };

        // static DevIpPultSpiMng& Instance();

		DevIpPultSpiMng(int busSpeedHz = CDefaultSpiBusSpeed);
		~DevIpPultSpiMng();

        BfDev::IBfSpiBusLock &GetPsb4851BusLock();
        BfDev::IBfSpiBusLock &GetPsb2170BusLock();
        BfDev::IBfSpiBusLock &GetFlashBusLock();
        		
	private:

        boost::scoped_ptr<BfDev::IBfSpiBusLock> m_psb4851BusLock;
        boost::scoped_ptr<BfDev::IBfSpiBusLock> m_psb2170BusLock;        
        boost::scoped_ptr<BfDev::IBfSpiBusLock> m_flashBusLock;
								
	};
	
};

#endif
