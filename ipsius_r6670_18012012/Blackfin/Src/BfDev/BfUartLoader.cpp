#include "stdafx.h"

#include "BfUartLoader.h"

namespace 
{	
    //-------------------------------------------------------------------					
	
    enum
    {
    	CLoadRequestTimeOutMs = 10000,    	
    	CNextByteTimeOutMs = 500,    	
    	CSlaveNotReadyMaxTimeMs = 2000,    	
    	CLoadRequestSignature = 0xBF    	
    };	
};

namespace BfDev
{
    //-------------------------------------------------------------------				

    BfUartLoader::BfUartLoader(IBfUartLoaderIO &loaderIO):    	
    	m_loaderIO(loaderIO),
    	m_slaveLoadIsReady(false)
    {}

    //-------------------------------------------------------------------			    

    void BfUartLoader::WaitSlaveReady() 
    {
        if(m_loaderIO.isSlaveReady()) return;

        m_timer.Set(CSlaveNotReadyMaxTimeMs,0);				
        while (!m_loaderIO.isSlaveReady());
        {
            ESS_ASSERT(!m_timer.TimeOut() && "SlaveNotReadyMaxTimeOut");
        }
    }

    //-------------------------------------------------------------------			    
    
    bool BfUartLoader::LoadRequest()
    {
    	
		ESS_ASSERT(!m_slaveLoadIsReady && "Slave was ready for loading");
		
        WaitSlaveReady();

		m_loaderIO.Uart().Write('@');		
		
		m_timer.Set(CLoadRequestTimeOutMs,0);				
		
		while(m_loaderIO.Uart().AvailableForRead() == 0)
		{
			if (m_timer.TimeOut()) return false;			
		}
		
		for(int i=0; i < 4; i++)
		{					
			byte data;    				
			if(!m_loaderIO.Uart().Read(data))  return false;
			if (i==0  && data != CLoadRequestSignature) return false;
		}		
		
		m_slaveLoadIsReady = true;
		return true;		
    }
    
    //-------------------------------------------------------------------			    

    void BfUartLoader::LoadBlock(byte *data, dword count)
    {
		ESS_ASSERT(m_slaveLoadIsReady && "SlaveNotReady for loading");	    							    	    	
  
		for(dword i = 0; i < count; i++) 
		{
            WaitSlaveReady();
            byte par = *data++;
            while(!m_loaderIO.Uart().Write(par, false));
		}
    }
    
    //-------------------------------------------------------------------						    
       		                 
} ;//namespace BfDev
