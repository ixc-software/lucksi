#include "stdafx.h"
#include "DomainNamedObjectTests.h"
#include "Domain/DomainTestRunner.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/commanddriresult.h"

/*

 TreeTest diagram:

    (0)
   /    \
  (1)    (2)
 / |  \    \
(3) (4) (5)  (6)

1 - deleted

*/


// -----------------------------------------------------------

namespace
{
    using namespace Domain;

    enum
    {
        CTestTimeout = 2000
    };

    class TestParams
    {
    };

    // -----------------------------------------------------------

    class TreeTest
    {
    public:

        TreeTest(DomainClass &domain, TestParams &params)
        {
            // create NamedObject tree
            // constructor #2
            NamedObject* pParent1 = new NamedObject(&domain, "N1");
            NamedObject* pParent2 = new NamedObject(&domain, "N2");
            new NamedObject(&domain, "N1.N3");

            // constructor #3
            new NamedObject(&domain, "N4", pParent1);
            new NamedObject(&domain, "N5", pParent1);
            new NamedObject(&domain, "N6", pParent2);

            //check tree structure
            TUT_ASSERT(domain.FindFromRoot("N1"));
            TUT_ASSERT(domain.FindFromRoot("N2"));
            TUT_ASSERT(domain.FindFromRoot("N1.N3"));
            TUT_ASSERT(domain.FindFromRoot("N1.N4"));
            TUT_ASSERT(domain.FindFromRoot("N1.N5"));
            TUT_ASSERT( pParent2->FindObject( ObjectName("N6") ) );

            // delete node pParent0
            pParent1->Delete();

            // check tree structure
            //ensure: non-deleted object are in the tree
            TUT_ASSERT(domain.FindFromRoot("N2"));
            TUT_ASSERT(domain.FindFromRoot("N2.N6"));

            //ensure: deleted objects aren't in the tree 
            TUT_ASSERT(!domain.FindFromRoot("N1"));
            TUT_ASSERT(!domain.FindFromRoot("N1.N3"));
            TUT_ASSERT(!domain.FindFromRoot("N1.N4"));
            TUT_ASSERT(!domain.FindFromRoot("N1.N5"));

            // done
            domain.Stop(Domain::DomainExitOk);
        }

    };

    // -----------------------------------------------------------

    class DublicatesTest
    {
    public:

        DublicatesTest(DomainClass &domain, TestParams &params)
        {
            bool wasException = false;

            try
            {
                new NamedObject(&domain, "Object_1");                
                new NamedObject(&domain, "Object_1"); // must throw exception
            }
            catch(DRI::ExecuteException &e)
            {
                wasException = true;
            } 

            TUT_ASSERT(wasException);
            
            // done
            domain.Stop(Domain::DomainExitOk);
        }

    };

}  // namespace


// -----------------------------------------------------------

namespace DomainTests
{

    void NamedObjectTreeTest()
    {
        Domain::DomainTestRunner runner(CTestTimeout);
        TestParams params;
        bool res = runner.Run<TreeTest>(params);
        TUT_ASSERT(res);
    }

    void DublicateChildTest()
    {
        Domain::DomainTestRunner runner(CTestTimeout);
        TestParams params;
        bool res = runner.Run<DublicatesTest>(params);
        TUT_ASSERT(res);
    }

}  // namespace DomainTests
