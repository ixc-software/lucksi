#include "stdafx.h"

#include "BfDev/SysProperties.h"

#include "DevIpPultSpiMng.h"

namespace
{

	DevIpPult::DevIpPultSpiMng *GIpPultSpi = 0;

} 


namespace DevIpPult
{
    using namespace BfDev;        


	// -----------------------------------------------------------------------

	DevIpPultSpiMng::DevIpPultSpiMng(int busSpeedHz):
		SpiBusMng(busSpeedHz),
		m_flashBusLock( new BfSpiBusLock(PortF, 10) )
	{
		ESS_ASSERT(!GIpPultSpi);
		GIpPultSpi = this;
	}

	// -----------------------------------------------------------------------

	DevIpPultSpiMng::~DevIpPultSpiMng()
	{
		GIpPultSpi = 0;
	}

	// -----------------------------------------------------------------------

	BfDev::IBfSpiBusLock &DevIpPultSpiMng::GetPsb4851BusLock()
	{
        if (m_psb4851BusLock == 0)
        {
            m_psb4851BusLock.reset( new BfSpiBusLock(PortG, 2) );
        }

		return *m_psb4851BusLock;
	}

	// -----------------------------------------------------------------------

	BfDev::IBfSpiBusLock &DevIpPultSpiMng::GetPsb2170BusLock()
	{
        if (m_psb2170BusLock == 0)
        {
            m_psb2170BusLock.reset( new BfSpiBusLock(PortG, 11) );
        }

		return *m_psb2170BusLock;
	}                        

	// -----------------------------------------------------------------------

	BfDev::IBfSpiBusLock &DevIpPultSpiMng::GetFlashBusLock()
	{
		return *m_flashBusLock;
	}

} // namespace DevIpPult
