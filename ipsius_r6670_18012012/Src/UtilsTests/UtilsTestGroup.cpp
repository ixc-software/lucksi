
#include "stdafx.h"

#include "UtilsTestGroup.h"

#include "Utils/QtEnumResolver.h"
#include "UtilsTests/QtEnumResolverTest.h"
#include "Utils/SafeRef.h"
#include "Utils/ProgramArg.h"
#include "Utils/DeleteIfPointer.h"
#include "Utils/StringUtils.h"
// #include "Utils/BidirBufferCreator.h"

#include "MutexTest.h"
#include "StringParserTest.h"
#include "SafeRefServerTest.h"
#include "FsmTest.h"
#include "TypeFactoryTest.h"
#include "MemoryPoolTests.h"
#include "Utils/ContainerConverter.h"
#include "Utils/MetaConfig.h"
#include "VirtualInvokeTest.h"
#include "ContainerMemFindTests.h"
#include "BinaryReaderWriterTests.h"
#include "MemReaderWriterStreamTests.h"
#include "StackStringTest.h"
#include "IntToStringTests.h"
#include "Utils/BidirBufferCreator.h"
#include "BidirBufferTest.h"
#include "FixedFifoTest.h"
#include "IntSetTests.h"
#include "UtilsDateTimeTests.h"
#include "DateTimeCaptureTests.h"
#include "StatisticBaseTest.h"
#include "HostInfTest.h"
#include "Utils/RawCRC32.h"
#include "ProgramOptionsTest.h"
#include "ManagedListTest.h"
#include "StringListTest.h"
#include "IniFileTest.h"
#include "Utils/RangeList.h"
#include "OutputFormattersTest.h"
#include "Utils/WeakRef.h"
#include "FullEnumResolverTest.h"

namespace 
{
    const int CTimeoutMS = 6000000;

} // namespace 

// ------------------------------------------

namespace 
{
    using namespace Utils;
    using namespace UtilsTests;
    using namespace TestFw;

    class UtilsTestsGroup : public TestGroup<UtilsTestsGroup>
    {

        UtilsTestsGroup(ILauncher &launcher) 
        : TestGroup<UtilsTestsGroup>(launcher, "Utils")             
        {
            bool showInfo = false;
            bool silentMode = true;
            
            Add("Mutex test", &MutexTest, showInfo);
            Add("String parsing test", &StringParserTest);

            if (Utils::SafeRefDebugModeEnabled())
            {
                Add("SafeRef test in debug mode", &RunSafeRefTest, showInfo);
            }
            else 
            {
                Add("SafeRef test in non-debug mode", &SafeRefServerTest);
            }
            
            Add("FSM",                                  &NewFsmTest);
            Add("QtEnumResolver test",                  &UtilsTests::TestQtEnumResolver);
            Add("ProgramArg test",                      &ProgramArg::ProgramArgTest);
            Add("Type factory test",                    &TypeFactoryTest);
            Add("Container converter test",             &TestConverter);
            Add("MetaConfig test",                      &MetaConfig::RunTest, silentMode);
            Add("VirtualInvoke test",                   &VirtualInvokeTest);
            Add("Container member find tests" ,         &ContainerMemFindTests);
            Add("DeleteIfPointer",                      &DeletePointerTest);
            Add("BinaryReader/Writer tets",             &BinaryReaderWriterTests, silentMode);
            Add("MemReader/WriterStream tests",         &MemReaderWriterStreamTests);
            Add("MemoryPool test",                      &MemBlockPoolTest);
            Add("MemoryPool sim",                       &MemBlockPoolTestSimulation);
            Add("StackString test",                     &StackStringTest);
            Add("BidirBufferCreator",                   &BidirBufferCreator<>::BidirCreatorTest);
            Add("IntToString tests",                    &IntToStringTests);
            Add("BidirBuffer tests",                    &BidirBufferTest);
            Add("FixedFifo tests",                      &FixedFifoTest);
            Add("IntSet tests",                         &IntSetTests);
            Add("Date, Time and DateTime tests",        &UtilsDateTimeTests);
            Add("MsDateTime test",                      &MsDateTimeCaptureTest);
            Add("QtDateTime test",                      &QtDateTimeCaptureTest);
            Add("StatisticBase test",                   &StatisticBaseTest, silentMode);
            Add("HostInf test",                         &HostInfTest, silentMode);
            Add("Raw CRC32",                            &Utils::TestRawCrc32);
            Add("ProgramOptions test",                  &ProgramOptionsTest, silentMode);
            Add("ManagedList test",                     &ManagedListTest, silentMode);
            Add("String case",                          &StringCaseConvTest);
            Add("StringList test",                      &StringListTest, silentMode);
            Add("IniFile test",                         &IniFileTest, silentMode);
            Add("ValueRange test",                      &ValueRange::Test);
            Add("RangeList test",                       &RangeList::Test);
            Add("TableOutput test",                     &TableOutputTest, silentMode);
            Add("WeakRef test",                         &Utils::WeakRefTest);
            Add("RunFullEnumResolver test",             boost::function<void ()>(RunFullEnumResolverTest) );
        }

    public:

        static void AddToLauncher(ILauncher &launcher)
        {
            new UtilsTestsGroup(launcher);  // free at launcher
        }
    };

} // namespace 

// ------------------------------------------------

namespace UtilsTests
{
    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        UtilsTestsGroup::AddToLauncher(launcher);
    }

} // namespace UtilsTests

