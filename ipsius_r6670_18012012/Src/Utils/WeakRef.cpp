#include "stdafx.h"

#include "WeakRef.h"

// ----------------------------------------------------

namespace
{
    using namespace Utils;

    class ITest : public Utils::IBasicInterface
    {
    public:
        virtual void Foo() = 0;
    };

    class A : public ITest
    {
        WeakRefHost m_host;
        int m_counter;

    // ITest impl
    private:

        void Foo()
        {
            ++m_counter;
        }

    public:

        A() : m_counter(0)
        {
            {
                m_host.Create<ITest&>(*this).Value().Foo();
                m_host.Create<ITest*>(this).Value()->Foo();

                TUT_ASSERT( m_counter == 2 );
                TUT_ASSERT( m_host.Empty() );
            }

            {
                WeakRef<ITest*> ref0 = m_host.Create<ITest*>(this);
                WeakRef<ITest*> ref1 = m_host.Create<ITest*>(this);
                WeakRef<ITest*> ref2;

                TUT_ASSERT(ref0 == ref1);
                TUT_ASSERT(ref0 != ref2);
            }
        }

    };

    void ClassTest()
    {

        A a;
    }

    void FullCycleTest()
    {
        boost::scoped_ptr<WeakRefHost> host(new WeakRefHost());

        WeakRef<std::string> refDoom = host->Create<std::string>("Doom"); 
        WeakRef<int> refInt16 = host->Create(16);
        WeakRef<int> refInt16Copy = refInt16;

        TUT_ASSERT( refDoom.Value() == "Doom" );
        TUT_ASSERT( refInt16.Value() == 16 );
        TUT_ASSERT( refInt16Copy.Value() == refInt16.Value() );
        TUT_ASSERT( refInt16 == refInt16Copy );

        host.reset();

        TUT_ASSERT( refDoom.Empty() );
        TUT_ASSERT( refInt16.Empty() );
        TUT_ASSERT( refInt16Copy.Empty() );
    }

}  // namespace

// ----------------------------------------------------

namespace Utils
{

    void WeakRefTest()
    {
        ClassTest();
        FullCycleTest();
    }

}  // namespace Utils
