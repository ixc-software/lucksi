
#include "stdafx.h"
#include "ErrorsSubsystemTests.h"

// -------------------------------------------------------------
// -------------------------------------------------------------

namespace
{
    ESS_TYPEDEF(IoError);

    ESS_TYPEDEF_FULL(FileOpen,  IoError);
    ESS_TYPEDEF_FULL(FileWrite, IoError);

    // исключение, чтобы не довести проверку ESS_ASSERT() до завершения программы
    ESS_TYPEDEF_FULL(BreakAbort, ESS::HookRethrow);

}  // namespace


// -------------------------------------------------------------
// -------------------------------------------------------------

namespace UtilsTests
{
    using namespace ESS;
     
    void TestTutException()
    {
        bool m_ok = false;

        try
        {
            TUT_ASSERT(0 && "Just test throw ESS::TutException...");
        }
        catch (const TutException &e)
        {
        	m_ok = true;
        }

        assert(m_ok);  // must use TUT_ASSERT(), but seems it does not work :)
    }


    // ---------------------------------------------------------------

    void TestSimpleException()
    {
        bool m_ok = false;

        // Exception hierarchy check
        m_ok = false;

        try
        {
            ESS_THROW(FileOpen);
        }
        catch (const IoError &e)
        {
            m_ok = true;
        }

        TUT_ASSERT(m_ok);

        // Exception check
        m_ok = false;

        try
        {
            ESS_THROW(FileWrite);
        }
        catch (const FileWrite &e)
        {
            m_ok = true;
        }

        TUT_ASSERT(m_ok);

    }

    // ---------------------------------------------------------------

    void TestExceptionHook()
    {
        bool m_ok = false;

        ExceptionHookCounter<FileOpen> hook;

        try
        {
            ESS_THROW(FileOpen);
        }
        catch (const IoError &e)
        {
            m_ok = true;
        }

        TUT_ASSERT(m_ok && (hook.getCounter() == 1));        
    }

    // ---------------------------------------------------------------

    void TestTutHook()
    {
        bool m_ok = false;

        ExceptionHookCounter<TutException> hook;

        try
        {
            TUT_ASSERT(0 && "Test");
        }
        catch (const TutException &e)
        {
            m_ok = true;
        }

        TUT_ASSERT(m_ok && (hook.getCounter() == 1));                
    }

    // ---------------------------------------------------------------

    void TestAbortHook()
    {
        bool m_ok = false;

        ExceptionHookRethrow<BreakAbort> hook;

        try
        {
            ESS_ASSERT(0 && "This is the End of the World...");
        }
        catch (const BreakAbort &e)
        {
            m_ok = true;
        }

        TUT_ASSERT(m_ok && (hook.getCounter() == 1));
    }

    // ---------------------------------------------------------------

    ESS_TYPEDEF_T(NumericException, double);

    ESS_TYPEDEF_T_EXTEND(NumericValueOverflow, NumericException); 
    ESS_TYPEDEF_T_EXTEND(NumericNegativeSqrt, NumericException);   

    void RunTypedExceptionTest()
    {
        double val = 45.6;
        bool wasException = false;

        try
        {
            ESS_THROW_T(NumericNegativeSqrt, val);
        }
        catch(const NumericException &e)
        {
            wasException = true;
            TUT_ASSERT(e.getProperty() == val);
        }

        TUT_ASSERT(wasException);
    }


}  // namespace UtilsTests


// -------------------------------------------------------------
// -------------------------------------------------------------

namespace UtilsTests
{
    using namespace TestFw;

    class GroupEss : public TestGroup<GroupEss>
    {
        GroupEss(ILauncher &launcher) : TestGroup<GroupEss>(launcher, "ESS")             
        {
            Add("TUT exception test",       &TestTutException);
            Add("Simple exception test",    &TestSimpleException);
            Add("Test exception hook",      &TestExceptionHook);
            Add("Test TUT hook",            &TestTutHook);
            Add("Test Abort hook",          &TestAbortHook);
            Add("Typed exception test",     &RunTypedExceptionTest);
        }

    public:

        static void AddToLauncher(ILauncher &launcher)
        {
            new GroupEss(launcher);  // free at launcher
        }
    };

    // ------------------------------------------------

    namespace ESSTests
    {
        void RegisterTestGroup(TestFw::ILauncher &launcher)
        {
            GroupEss::AddToLauncher(launcher);
        }
    } // namespace ESSTests
    
}  // namespace UtilsTests


