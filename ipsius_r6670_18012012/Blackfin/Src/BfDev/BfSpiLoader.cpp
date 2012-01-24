#include "stdafx.h"

#include "BfSpiLoader.h"

namespace 
{	
    //-------------------------------------------------------------------					
	
    enum
    {
    	CNextByteTimeOutMs = 500,    	
    	CSlaveNotReadyMaxTimeMs = 100,    	
        CResetTimeMs = 5
    };	
};

namespace BfDev
{
	
	BfSpiLoader *pSpiLoader=0;			
	
    //-------------------------------------------------------------------				

    BfSpiLoader::BfSpiLoader(BfDev::BfSpi &spi, BfDev::IBfSpiBusLock &echoLoaderBusLock):    	
    	m_spi(spi),
    	m_busLock(echoLoaderBusLock)
    {
		ESS_ASSERT(!pSpiLoader && "BfSpiLoader already created");	    	
		pSpiLoader = this;	    		    		    	
				
		*pPORTGIO_INEN		|= 0x0002;  //Slave Busy Pin
		
        ResetEcho();
    }
    
    //-------------------------------------------------------------------					
	
	BfSpiLoader::~BfSpiLoader()
	{
		*pPORTGIO_INEN	&= ~(word)0x0002;
		pSpiLoader =	0;
	}    

    //-------------------------------------------------------------------			    

    void BfSpiLoader::ResetEcho() 
    {
		*pPORTGIO_DIR |= 0x0080;		// ResetEcho
		*pPORTGIO_CLEAR	= 0x0080;                    	    			
        m_timer.Set(CResetTimeMs,false);
        while (!m_timer.TimeOut());
		*pPORTGIO_DIR &= ~0x0080;		
        m_timer.Set(CResetTimeMs,false);
        while (!m_timer.TimeOut());				
    }	
	
    //-------------------------------------------------------------------			    

    bool BfSpiLoader::WaitSlaveReady() 
    {
        if(isSlaveReady()) return true;
        m_timer.Set(CSlaveNotReadyMaxTimeMs, false);				
        while (!isSlaveReady())
        {
//            ESS_ASSERT(!m_timer.TimeOut() && "SlaveNotReadyMaxTimeOut");
            if(m_timer.TimeOut()) return false;
        }
        return true;
    }

    //-------------------------------------------------------------------			    

    bool BfSpiLoader::LoadBlock(const byte *data, dword count)
    {

        BfDev::SpiBusLocker busLock(m_busLock);
		for(dword i = 0; i < count; i++) 
		{
            if(!WaitSlaveReady()) return false;			
            m_spi.Transfer(*data++);
		}			
		return true;
    }        
    
    //-------------------------------------------------------------------				

	bool BfSpiLoader::isSlaveReady() const
	{
		return !(*pPORTGIO & 0x0002);
	}
    
       		                 
} ;//namespace BfDev
