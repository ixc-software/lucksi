
#include "stdafx.h"

#include "TelnetTestGroup.h"
#include "TelnetTests.h"


namespace 
{
    using namespace TelnetTests;
    
	const bool CSilentMode = true;
	// TelnetTests::TestLogger GTestLog(CSilentMode);

    // --------------------------------------
    
    class TestLogger : public ITelnetTestLogger
    {
        QString m_log;
        bool m_silentMode;
        
    // ITelnetTstLogger impl
    public:
        void Add(const QString &data, bool asLine)
        {
            QString newData(data);
            if (asLine) newData += "\n";

            m_log += newData;

            if (!IsSilentMode()) std::cout << newData;
        }
        
        QString Get() const { return m_log; }
        bool IsSilentMode() const { return m_silentMode; }
        
    public:
        TestLogger(bool silentMode) : m_silentMode(silentMode)
        {
            if (!IsSilentMode()) std::cout << "Telnet tests log ..." << std::endl;
        }
        
    };
    
} // namespace 

// --------------------------------------

namespace
{
	using namespace TelnetTests;
	using namespace TestFw;

    class TelnetTestsGroup : public TestGroup<TelnetTestsGroup>
    {
        boost::scoped_ptr<ITelnetTestLogger> m_log;
        // TestLogger m_log;
        
        TelnetTestsGroup(ILauncher &launcher) 
		: TestGroup<TelnetTestsGroup>(launcher, "Telnet"), 
            m_log(new TestLogger(CSilentMode))
        {
            Add("Telnet data collector test",		&TelnetDataCollectorTest,	*m_log);
            Add("Telnet option list test", 			&TelnetOptionListTest, 		*m_log);
			Add("Telnet helpers test",				&TelnetHelpersTest, 		*m_log);
            Add("Telnet utils test", 				&TelnetUtilsTest, 			*m_log);
			Add("Telnet client-server system test", &TelnetClientServerTest, 	*m_log);
       }

    public:

        static void AddToLauncher(ILauncher &launcher)
        {
            new TelnetTestsGroup(launcher);  // free at launcher
        }
    };

} // namespace

// ------------------------------------------------

namespace TelnetTests
{
    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        TelnetTestsGroup::AddToLauncher(launcher);
    }

} // namespace TelnetTests

