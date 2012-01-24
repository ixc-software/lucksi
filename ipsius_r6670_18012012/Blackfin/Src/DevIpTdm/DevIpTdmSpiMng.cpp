#include "stdafx.h"

#include "BfDev/SysProperties.h"

#include "DevIpTdmSpiMng.h"


namespace
{
	DevIpTdm::DevIpTdmSpiMng *GIpTdmSpi = 0;
} // namespace


namespace DevIpTdm
{
    using namespace BfDev;
            
	// --------------------------------------------

	//  ласс управл€ющий выбором загрузки чипа Echo-процессора
	class EchoLoaderBusLock : public BfDev::IBfSpiBusLock,
							  boost::noncopyable
	{
		BfSpiBusLock m_lock;
		
	// impl IBfSpiBusLock        
	private:		
	
		void ChipEnable()
		{
			m_lock.ChipEnable();
			
			*pSPI_CTL |= CPHA;
			ssync();						
		}	

		void ChipDisable()
		{
			m_lock.ChipDisable();
			
			*pSPI_CTL &= ~CPHA;
			ssync();			
		}						
		
	public:	    

		EchoLoaderBusLock() : m_lock(PortG, 0)
		{
			 ChipDisable();
		}

	};

	// -------------------------------------------

	DevIpTdmSpiMng::DevIpTdmSpiMng(int busSpeedHz):
		SpiBusMng(busSpeedHz),
        m_flashBusLock( new BfSpiBusLock(PortF, 10) )
	{
		ESS_ASSERT(!GIpTdmSpi);
		GIpTdmSpi = this;
	}

	// ------------------------------------------------------------------------------------    

	DevIpTdmSpiMng::~DevIpTdmSpiMng()
	{
		GIpTdmSpi = 0;
	}

	// ------------------------------------------------------------------------------------

	DevIpTdmSpiMng& DevIpTdmSpiMng::Instance()  // static
	{
		ESS_ASSERT(GIpTdmSpi);
		return *GIpTdmSpi;
	}

	// ------------------------------------------------------------------------------------    

	BfDev::IBfSpiBusLock &DevIpTdmSpiMng::GetFlashBusLock()
	{
		return *m_flashBusLock;
	}

	// ------------------------------------------------------------------------------------    

	BfDev::IBfSpiBusLock &DevIpTdmSpiMng::GetEchoLoaderBusLock()
	{
        if (m_echoLoadBusLock == 0) 
        {
            m_echoLoadBusLock.reset( new EchoLoaderBusLock() );
        }

		return *m_echoLoadBusLock;
	}                        
           
    // ------------------------------------------------------------------------------------    
	

} // namespace DevIpTdm

