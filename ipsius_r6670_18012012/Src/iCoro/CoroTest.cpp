#include "stdafx.h"

#include "CoroGen.h"

#include "CoroTest.h"

// ----------------------------------------------------------

namespace
{
    using iCoro::Gen;
    using iCoro::Coro;

    class Counter : boost::noncopyable
    {
        Gen<int> m_gen;

        static void Body( Gen<int>::YieldFn yield, int startVal, int endVal )
        {
            int n = startVal;	

            while( (n < endVal) || (endVal < 0) )
            {
                yield( n++ );
            }
        }

    public:

        Counter(int startVal, int endVal = -1) : 
          m_gen( boost::bind( &Counter::Body, _1, startVal, endVal) )
        {
        }

        int Next() 
        {
            return m_gen.Next();
        }

        bool End()
        {
            return m_gen.End();
        }
    };

    // ----------------------------------------------------

    class FactTest
    {

        static int CalcFact(int n)
        {
            int c = 1;

            for(int i = 1; i < n; ++i)
            {
                c = c * i;
            }

            return c;            
        }

        static void Fact( Gen<int>::YieldFn yield )
        {
            int c = 1;
            Counter n(1);	

            while(true)
            {
                c = c * n.Next(); 
                yield(c);
            }
        }

    public:

        static void Run()
        {
            int CLimit = 5;

            iCoro::Gen<int> g(&Fact, CLimit);

            int count = 0;

            while( !g.End() )
            {
                int val = g.Next();
                count++;
                
                int verifyVal = CalcFact(count + 1);
                TUT_ASSERT(val == verifyVal);
            }

            TUT_ASSERT(count == CLimit);
        }

    };

    // ----------------------------------------------------
    
    int GInstCounter;
    int GConstrCounter;

    class ExitTest
    {
        ExitTest()  { GInstCounter++; GConstrCounter++; }
        ~ExitTest() { GInstCounter--; }

        static void Body(Coro *pC)
        {
            ExitTest t;
            pC->Return();
        }

    public:

        static void Run()
        {
            GInstCounter = 0;
            GConstrCounter = 0;

            // case #1: never run
            {
                Coro c;
                c.SetBody( boost::bind(&ExitTest::Body, &c) );
                // there is no Run()
            }
            TUT_ASSERT(GConstrCounter == 0);

            // case #2: exit thru rethrow
            {
                Coro c;
                c.SetBody( boost::bind(&ExitTest::Body, &c) );
                c.Run();
                TUT_ASSERT( !c.Completed() );
                c.Abort();  // kill it
            }
            TUT_ASSERT(GConstrCounter == 1);
            TUT_ASSERT(GInstCounter == 0);

        }
    };

    // ----------------------------------------------------

    class GenTest
    {

        static void Run(int from, int to, int refCounter)
        {
            int counter = 0;

            Counter c(from, to);

            while( !c.End() ) 
            {
                c.Next();
                counter++;
            }

            TUT_ASSERT(counter == refCounter);
        }

    public:
    
        static void Run()
        {
            Run(1, 1, 0);  // 0 times
            Run(0, 5, 5);  // 5 times
        }
    };

}

// ----------------------------------------------------------

namespace iCoro
{

    void GenTest()
    {
        FactTest::Run();
        ExitTest::Run();
        GenTest::Run();
    }

}  // namespace iCoro

