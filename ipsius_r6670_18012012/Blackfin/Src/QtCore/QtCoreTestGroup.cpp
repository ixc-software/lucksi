
#include "stdafx.h"

#include "QtCoreTestGroup.h"
#include "QtCoreTests.h"

namespace
{
    using namespace QtCoreTests;
	using namespace TestFw;

    class QtCoreTestsGroup : public TestGroup<QtCoreTestsGroup>
    {
        QtCoreTestsGroup(ILauncher &launcher) 
		: TestGroup<QtCoreTestsGroup>(launcher, "QtCore")             
        {
            Add("QChar tests", &QCharTests);
            Add("QByteArray tests", &QByteArrayTests);
            Add("QString tests", &QStringTests);
            Add("QHostAddress tests", &QHostAddressTests);
			// ... 
       }

    public:
        static void AddToLauncher(ILauncher &launcher)
        {
            new QtCoreTestsGroup(launcher);  // free at launcher
        }
    };
    
} // namespace

// ---------------------------------------------------

namespace QtCoreTests
{
	void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        QtCoreTestsGroup::AddToLauncher(launcher);
    }

} // namespace QtCoreTests
