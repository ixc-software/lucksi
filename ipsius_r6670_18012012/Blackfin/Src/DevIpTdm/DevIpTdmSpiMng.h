#ifndef __IPTDM_SPI_BUS_MNG__
#define __IPTDM_SPI_BUS_MNG__

#include "stdafx.h"
#include "Platform/platformtypes.h"
#include "BfDev/SpiBusMng.h"
#include "BfDev/IBfSpiBusLock.h"
#include "BfDev/BfSpi.h"


namespace DevIpTdm
{		           
    
	class DevIpTdmSpiMng : public BfDev::SpiBusMng
	{									
    public:

        static DevIpTdmSpiMng& Instance();

		DevIpTdmSpiMng(int busSpeedHz = BfDev::CDefaultSpiBusSpeed);
		~DevIpTdmSpiMng();
        BfDev::IBfSpiBusLock &GetFlashBusLock();
        BfDev::IBfSpiBusLock &GetEchoLoaderBusLock();
        		
	private:
		
        boost::scoped_ptr<BfDev::IBfSpiBusLock> m_flashBusLock;
        boost::scoped_ptr<BfDev::IBfSpiBusLock> m_echoLoadBusLock;        
								
	};	
};

#endif
