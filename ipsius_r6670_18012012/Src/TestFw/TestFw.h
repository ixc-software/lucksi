#ifndef __TESTFW__
#define __TESTFW__

// #include "TestFwOutput.h"
//#include "ITestFwOutputStream.h"
#include "ISingleTest.h"
#include "ITestGroup.h"
#include "TestFwOutputtersList.h"

#include "Utils/VirtualInvoke.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ArgStringList.h"

namespace TestFw
{
    using Utils::VirtualInvoke;
    using Utils::IVirtualInvoke;
    using Utils::ArgStringList;

    // --------------------------------------------------------------

    class ILauncher : public Utils::IBasicInterface
    {
    public:
        virtual void RegisterGroup(ITestGroup *pGroup) = 0;
    };

    // --------------------------------------------------------------

    // test launcher, singletone
    class TestLauncher : boost::noncopyable, public ILauncher
    {
        std::vector<ITestGroup*> m_tests;
        TestFwOutputtersList m_outputters;

        // for exception info
        std::string m_executedGroupName;
        std::string m_executedTestName;
        
        typedef TestLauncher T;

        ITestGroup* FindByName(const std::string &name)
        {
            for(size_t i = 0; i < m_tests.size(); ++i)
            {
                if (m_tests.at(i)->getName() == name) return m_tests.at(i);
            }

            return 0;
        }

        void RunSingleTest(ISingleTest &test);                  // handle single test events
        void RunGroup(ITestGroup *pGroup, ISingleTest *pTest);  // handle group events

        void RunSingleTestInGroup(ISingleTest *pTest);   // redirected call to RunGroup
        void RunAllInGroup(ITestGroup *pGroup);          // redirected call to RunGroup

        void RunAll()
        {
            for(size_t i = 0; i < m_tests.size(); ++i)
            {
                RunAllInGroup(m_tests.at(i));
            }            
        }

        void Run(IVirtualInvoke &fn)
        {
            m_outputters.RunMethodForEach(&ITestFwOutputStream::RunStarted, *this);

            fn.Execute();

            m_outputters.RunMethodForEach(&ITestFwOutputStream::RunCompleted, *this);
        }

        void ListHelp();
        void ListGroups(bool shortForm);
        void ProcessSingleParam(const std::string &param);
        void ProcessTwoParams(const std::string &group, const std::string &name);

    // ILauncher impl
    private:

        // Added group free in destructor, don't use shared_ptr for simplify porting
        void RegisterGroup(ITestGroup *pGroup)
        {
            ESS_ASSERT(pGroup != 0);

            // dublicates check
            ESS_ASSERT(FindByName(pGroup->getName()) == 0);

            // register
            m_tests.push_back(pGroup);
        }


    public:

        ESS_TYPEDEF(VerificationException);

        TestLauncher(ITestFwOutputStream &outputter)
        {
            m_outputters.Add(&outputter);
        }

        ~TestLauncher()
        {
            for(size_t i = 0; i < m_tests.size(); ++i)
            {
                delete (m_tests.at(i));
            }
        }

        void AddOutputter(ITestFwOutputStream &outputter)
        {
            m_outputters.Add(&outputter);
        }

        void Run(const ArgStringList &arg);
        
    };

    // --------------------------------------------------------------

    // single test
    class SingleTest : public ISingleTest, boost::noncopyable
    {
        ITestGroup &m_group;
        std::string m_name;
        Utils::IVirtualInvoke *m_pInvoke;
        int m_testNumber;

        void InitTestNumber()
        {
            for(int i = 0; i < m_group.getTestsCount(); ++i)
            {
                if (m_group.getTest(i) == this)
                {
                    m_testNumber = i + 1;
                    return;
                }
            }

            ESS_ASSERT(0 && "Can't find test in group");
        }

        // ISingleTest impl
    private:

        void Run()
        {
            m_pInvoke->Execute();
        }

        const std::string& getName() const
        {
            return m_name;
        }

        int getTestNumber()
        {
            if (m_testNumber < 0) InitTestNumber();
            return m_testNumber;
        }

        std::string getGroupName() const
        {
            return m_group.getName();
        }

        ITestGroup& getGroup()
        {
            return m_group;
        }


    public:

        // pInvoke free in destructor; don't use shared_ptr for simplify porting
        SingleTest(ITestGroup &group, const std::string &name, Utils::IVirtualInvoke *pInvoke) 
            : m_group(group), m_name(name), m_pInvoke(pInvoke), m_testNumber(-1)
        {
            ESS_ASSERT(m_pInvoke != 0);
            ESS_ASSERT(m_name.size() > 0);
        }

        ~SingleTest()
        {
            delete m_pInvoke;
        }

    };


    // --------------------------------------------------------------

    // basic test group
    template<class TBasic>
    class TestGroup : boost::noncopyable, public ITestGroup
    {
        std::string m_name;
        std::vector<ISingleTest*> m_tests;
        TBasic *m_pBase;

        void Register(ISingleTest *p)
        {
            ESS_ASSERT(FindByName(p->getName()) == 0);

            m_tests.push_back(p);
        }

        TBasic& getBase()
        {
            if (m_pBase == 0) 
            {
                m_pBase = dynamic_cast<TBasic*>(this);
                ESS_ASSERT(m_pBase != 0);
            }

            return *m_pBase;
        }

    // ITestGroup impl
    private:

        const std::string& getName() const
        {
            return m_name;
        }

        int getTestsCount() const
        {
            return m_tests.size();
        }

        ISingleTest* getTest(int index)
        {
            ESS_ASSERT( index < m_tests.size() );
            return m_tests.at(index);
        }

        void OnFullGroupCompleted()
        {
            // nothing
        }

        ISingleTest* FindByName(const std::string &testName)
        {
            for(size_t i = 0; i < m_tests.size(); ++i)
            {
                if (m_tests.at(i)->getName() == testName) return m_tests.at(i);
            }

            return 0;
        }



    protected:
        
        void Add(const std::string &name, void (*pFn)() )
        {
            IVirtualInvoke *pI = VirtualInvoke::Create(pFn);
            Register( new SingleTest(*this, name, pI) );
        }

        template<class T>
        void Add(const std::string &name, void (*pFn)(T), T param)
        {
            IVirtualInvoke *pI = VirtualInvoke::Create(pFn, param);
            Register( new SingleTest(*this, name, pI) );
        }

        template<class T>
        void Add(const std::string &name, void (*pFn)(T&), T &param)
        {
            IVirtualInvoke *pI = VirtualInvoke::Create(pFn, param);
            Register( new SingleTest(*this, name, pI) );
        }

        template<class T>
        void Add(const std::string &name, void (*pFn)(const T&), const T &param)
        {
            IVirtualInvoke *pI = VirtualInvoke::Create(pFn, param);
            Register( new SingleTest(*this, name, pI) );
        }

        void Add(const std::string &name, void (TBasic::*pFn)())
        {
            IVirtualInvoke *pI = VirtualInvoke::Create(pFn, getBase());
            Register( new SingleTest(*this, name, pI) );
        }

        template<class T>
        void Add(const std::string &name, void (TBasic::*pFn)(T), T param)
        {
            IVirtualInvoke *pI = VirtualInvoke::Create(pFn, getBase(), param);
            Register( new SingleTest(*this, name, pI) );
        }

        template<class T>
        void Add(const std::string &name, void (TBasic::*pFn)(T&), T &param)
        {
            IVirtualInvoke *pI = VirtualInvoke::Create(pFn, getBase(), param);
            Register( new SingleTest(*this, name, pI) );
        }

        template<class T>
        void Add(const std::string &name, void (TBasic::*pFn)(const T&), const T &param)
        {
            IVirtualInvoke *pI = VirtualInvoke::Create(pFn, getBase(), param);
            Register( new SingleTest(*this, name, pI) );
        }

        template<class TFunc>
        void Add(const std::string &name, TFunc f)
        {
            IVirtualInvoke *pI = VirtualInvoke::Create(f); //  new Utils::VIFunctor<TFunc>(f);
            Register( new SingleTest(*this, name, pI) );
        }

        typedef TBasic T;

        TestGroup(ILauncher &launcher, const std::string &name) 
            : m_name(name), m_pBase(0)
        {
            launcher.RegisterGroup(this);
        }

    public:

        virtual ~TestGroup()
        {
            for(size_t i = 0; i < m_tests.size(); ++i)
            {
                delete (m_tests.at(i));
            }
        }

    };

    
    
}  // namespace TestFw

#endif

