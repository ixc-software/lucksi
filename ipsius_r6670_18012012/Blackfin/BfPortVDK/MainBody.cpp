
#include "stdafx.h"

#include "TestFw/TestFw.h"
#include "TestFw/TestFwGroup.h"
#include "TestFw/TestFwOutput.h"

#include "PlatformTests/PlatformTestsGroup.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/ErrorsSubsystemTests.h"
#include "Utils/VirtualInvokeTest.h"
#include "Utils/BidirBuffer.h"
#include "Utils/MemoryPool.h"
#include "Utils/TimerBase.h"
#include "Utils/TimerTicks.h"
#include "Utils/DateTimeCapture.h"
#include "Utils/DeleteIfPointer.h"

// #include "PlatformTests/PlatformThreadTest.h"
#include "PlatformTests/PlatformTests.h"

#include "iCore/MsgThread.h"
#include "iCoreTests/iCoreGroupBasic.h"
#include "iCoreTests/iCoreSimpleMsgTest.h"
#include "iCoreTests/iCoreTimerTest.h"
#include "iCoreTests/iCoreMsgBenchmarks.h"

#include "boost/detail/BoostTestGroup.h"
#include "iLog/LogTest.h"

#include "BfDev/SysProperties.h"
#include "BfDev/TicksVisualTest.h"

#include "../BfPort/SpecializationTest.h"
#include "../BfPort/RefTest.h"


// ------------------------------------------------------------

namespace
{
	
	class AppExceptionHook : public ESS::ExceptionHook
	{
	    int m_counter;

	public:

	    void Hook(const ESS::BaseException *pE)  // override
	    {
	        if (dynamic_cast<const ESS::Assertion*>(pE))
	        {
	            std::string info = pE->what();
	            m_counter++;
	        }
	    }
	};
	
	// ----------------------------------------
	
	class Test
	{
	public:
		ESS_TYPEDEF(Exception);
	
		void Throw()
		{
			ESS_ASSERT(0 && "Getcha!");
			// ESS_THROW(Exception);
		}
	};
	
	void TestThrow()
	{
        AppExceptionHook hook;
		
    	Test t;
    	t.Throw();
	}
	
	void TestTypeid()
	{
      const char *pName = typeid(Test::Exception).name();
      const char *pName2 = typeid(pName).name();
		
	}
	
	class X : public std::exception {};
	
	bool AnotherTestThrow()
	{
		try
		{
			TUT_ASSERT(0 && "X");
		}
		catch(std::exception &e)
		{
			return false;
		}
		
		return true;
	}
	
	void BoostTests()
	{
		boost::detail::shared_ptr_test(true);
    	boost::detail::scoped_ptr_test(true);		
	}
	
	int MallocOut(int size)
	{
		int count = 0;
		
		while(true)
		{
			void *p = std::malloc(size);
			if (p == 0) break;
			count++;
		}
		
		return count;
	}
	
	int NewOut(int size)
	{
		int count = 0;
		
		while(true)
		{			
			try
			{
				void *p = new char[size];
			}
			catch(...)
			{
				break;
			}		
			count++;	
		}			
				
		return size; 
	}
	
		
} // namespace

// ------------------------------------------------------------

namespace X
{
	
	
	class Y : boost::noncopyable
	{
		Y() {}
	};
	
	void Fn()
	{
		Y *p1, *p2;
		
		// *p1 = *p2;
	}
	
}

// ------------------------------------------------------------

void ExtTests()
{
    Utils::DeletePointerTest();
    UtilsTests::VirtualInvokeTest();

	BoostTests();   	
	
    PlatformTests::WaitConditionTest();
    PlatformTests::MutexTest();        
    PlatformTests::ThreadTest();	
}

void MsgThreadInstanceTest()
{
		iCore::MsgThread thread;
		thread.Run();
		Platform::ThreadSleep(10);
		thread.Break();	
}

void iCoreTestsRun()
{
	// Debug,     I   cache    -- ~15K	
	// Debug,     I+D cache    -- ~50K
	// Speed opt, I+D cache    -- ~150K
	std::string res = iCoreTests::iCoreBenchmarkSingle(250);	
	std::cout << res << std::endl;
	
	iCoreTests::iCoreTimerTest();
	
	iCoreTests::iCoreSimpleMsgTest();
	
	// MsgThreadInstanceTest();	
}

// ------------------------------------------------------------

BOOST_STATIC_ASSERT(sizeof(int) == sizeof(long));
BOOST_STATIC_ASSERT(sizeof(long long) == 8);


void MainBody()
{
	BfDev::SysProperties::InitFromProjectOptions();
	int freq = BfDev::SysProperties::Instance().getFrequencyCpu();
	std::cout << freq << std::endl;
	
	// iCoreTestsRun();
		        			
    TestFw::StringList sl;
    TestFw::StdOutput output;
    TestFw::TestLauncher launcher(output);

    {   
        TestFw::RegisterTestGroup(launcher);
        UtilsTests::ESSTests::RegisterTestGroup(launcher);
        iLog::RegisterTestGroup(launcher);        
        boost::detail::RegisterTestGroup(launcher);         
        PlatformTests::RegisterTestGroup(launcher);     	
        iCoreTests::RegisterTestGroup(launcher);        
    	 	                                
        // ...
    }

    launcher.Run(sl);
    
	ESS_ASSERT(0 && "Stop!");
	
    SpTest::Instance::Do();	
}

