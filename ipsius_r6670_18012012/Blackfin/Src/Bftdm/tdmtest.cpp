#include "stdafx.h"
#include "tdmtest.h"

// ------------------------------------------

enum
{
	CLssTimeout		= 3000,					// timeout for pattern syncronization
	CErrorsInserted	= 3,					// self inserted errors count
	CMinTestTime	= CLssTimeout + (CErrorsInserted+1)*1000,	// minimal test time	
};


// ------------------------------------------

namespace BfTdm
{
	    
	TdmTest::TdmTest(dword baseAddr, int msec, int channelsCount, TdmTestResult &result):
		m_board("TdmTest", baseAddr, Ds2155::LiuProfile(),Ds2155::HdlcProfile(256,10,0,256)),
        m_tdmObj(*this, 0, TdmProfile(3, 160, 0,BfTdm::CNoCompanding)),
        m_tdmObj1(*this, 1, TdmProfile(3, 160, 1,BfTdm::CNoCompanding)),
		m_channelsCount(channelsCount),
		m_result(result),		
		m_testtime(msec),
		m_boardFounded(false),
		m_wasSync(false),
		m_LSScount(0),
		m_passed(false),
		m_errorsInserted(0)		
	{

		ESS_ASSERT(m_testtime > CMinTestTime && "Not enough time for test ");
		ESS_ASSERT(m_channelsCount < 32 && "Only 31 channels can be used for test");						
		
		if (m_board.isDevicePresent())
		{						
			for(byte i=1; i<=m_channelsCount; ++i) m_board.BERT().SetChannelToTest(i);
			m_board.BERT().Start(m_board.BERT().TDM);
			m_boardFounded = true;
		}
		else 
		{
			Shutdown();
		}
		
		m_tdmObj.Enable();
		m_tdmObj1.Enable();				
		m_startTime = GetSystemTickCount();		
	}

    //---------------------------------------------------------------------        	
	
	TdmTest::~TdmTest()
	{
		Shutdown();		
	}
		        
    //---------------------------------------------------------------------        
        
	bool TdmTest::Polling()
	{
		if (!m_board.BERT().isTesting()) return true;
		dword time = GetSystemTickCount() - m_startTime;
		m_board.BERT().UpdateErrBitCount();
		
		byte state = m_board.LIU().UpdateBitsState();		
				
		if (state & (1 << m_board.LIU().L_STATE_LSS)) 
		{
			if (m_wasSync)	m_LSScount++;
			m_wasSync = false;						
		}
		else if (!m_wasSync)
		{
			m_wasSync = true;
			m_board.BERT().ClearErrBitCount();
		}			

		
		if (!m_wasSync && (time > CLssTimeout)) 
		{
			
			Shutdown();
			return true;
		}
			
		if (m_wasSync && !m_errorsInserted)
		{
			m_board.BERT().OneErrInsert();
			m_errorsInserted++;
		}
				
		if (time > m_testtime)
		{
			m_ErrCount = m_board.BERT().UpdateErrBitCount();												
			m_ErrCount -= m_errorsInserted;								
		
			if (!(m_LSScount || m_ErrCount)) m_passed = true;				
			
			Shutdown();				
		}
		return false;			
		
	}
	
    //---------------------------------------------------------------------        		
	
	void TdmTest::Shutdown()
	{
		m_board.BERT().Stop();		
		LoadResult();
	}

    //---------------------------------------------------------------------        			
	    
	void TdmTest::LoadResult()
	{	
		std::ostringstream res;		
		
		if (m_passed) 
		{
			res << "	Test passed" << std::endl;
			m_result.IsPassed = true;
		}
		else
		{
			res << "	Test is not passed"  << std::endl;
			m_result.IsPassed = false;			
			if (!m_boardFounded)
				res << "		Device not found, check board connection and restart the test"  << std::endl;
			else if (!m_wasSync)
				res << "  		Pattern was not syncronized"  << std::endl;
			else
			{				
				res << "		LSS errors = " << m_LSScount << std::endl;								
				res << "		BERT errors = " << m_ErrCount  << std::endl;
			}
		}
										
		m_result.AsString = res.str();
	}
	
    //-------------------------------------------------------------------                   

  	void TdmTest::NewBlockProcess(
        word sportNum,
        BfTdm::IUserBlockInterface &m_Ch0,
        BfTdm::IUserBlockInterface &m_Ch1,
        bool collision)
    {        

    	*pPORTFIO_SET	= 1<<10;        	
    	
        word tssize = m_Ch0.GetTSBlockSize();
        byte TSBuff[200];
        	        	               
        for (word timeSlot=1; timeSlot <= m_channelsCount; timeSlot++)
//        for (word timeSlot=1; timeSlot < 32; timeSlot++)
        {
        	
        	m_Ch0.CopyRxTSBlockTo(timeSlot, &TSBuff[0], 0, tssize);
			m_Ch0.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0, tssize);
        	m_Ch1.CopyRxTSBlockTo(timeSlot, &TSBuff[0], 0,tssize);
			m_Ch1.PutTxTSBlockFrom(timeSlot, &TSBuff[0], 0,tssize);				
						
        }            	        
                	
        m_Ch0.UserFree();
        m_Ch1.UserFree();        
        *pPORTFIO_CLEAR	= 1<<10;        
        
    }		    	

    void TdmTest::BlocksCollision(word sportNum)
    {
        while(1);
    }
    
    			
    //---------------------------------------------------------------------        	
	
    
}    
    
