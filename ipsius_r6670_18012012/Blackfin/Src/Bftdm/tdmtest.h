#ifndef __TDMTEST_H__
#define __TDMTEST_H__

#include "Platform/Platform.h"
#include "ds2155/boardds2155.h"
#include "Tdm.h"

namespace BfTdm
{
                       
    using namespace Platform;   
    
	struct TdmTestResult 
	{			
    	std::string AsString;		
		bool		IsPassed;		
	};
        
        
    // TdmTest class
	class TdmTest  : 
		boost::noncopyable,
		BfTdm::ITdmObserver
	{	                  	    	        	
	public:

	    TdmTest(dword baseAddr, int msec, int channelsCount, TdmTestResult &result); // begin testing with "msec" testtime
		~TdmTest();
		
		bool Polling();				// test processing (polling), true on test complete
		
            
	// ITdmObserver impl
    private:
    
    	virtual void NewBlockProcess(
    	    word sportNum,
    	    BfTdm::IUserBlockInterface &Ch0,
    	    BfTdm::IUserBlockInterface &Ch1,
    	    bool collision);	// test pattern loopBack method
    	    
        virtual void BlocksCollision(word sportNum);	
            
    private:

		void Shutdown();			// Test stoping before end of testtime    
		void LoadResult();			// string with test results		    
        	
    	Ds2155::BoardDs2155	m_board;	// Ds2155 object
    	
        
        Tdm  m_tdmObj;					// Tdm object for testing
        Tdm  m_tdmObj1;    				// Tdm object for testing
    	
        TdmTestResult &m_result;
        
        byte 		m_channelsCount;
        byte 		m_errorsInserted;        
//		bool		m_isSync;			// is pattern syncronized
		bool		m_wasSync;		    // was pattern syncronized
		dword		m_LSScount;			// Loss Pattern Syncronization counter
		dword 		m_testtime;			// Test time in mseconds
		dword 		m_startTime;		// Test StartTime		
		dword		m_ErrCount;			// Pattern errors counter
		bool		m_boardFounded;		// is Ds2155 board founded
		bool		m_passed;			// is test passed				    	                        
	};		
				
} //namespace BFTdm

#endif

