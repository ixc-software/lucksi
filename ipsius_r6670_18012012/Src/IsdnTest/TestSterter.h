#ifndef TESTSTERTER_H
#define TESTSTERTER_H

#include "Domain/NamedObject.h"

namespace IsdnTest
{    
    // base class for DRI-wrappers surround dss tests designed for run under TestCollector
    
    template<class TTest>
    class TestStarter : public Domain::NamedObject, ESS::ExceptionHook
    {
        typedef TestStarter<TTest> TMy;
        typename TTest::TNObjProfile* m_pProfile; // внутренний NObj
        scoped_ptr<TTest> m_test;

        void Hook( const ESS::BaseException *pE ) // override
        {            
            if(!AsyncActive()) return;         
            if (!dynamic_cast<const ESS::TutException*>(pE)) return;   

            AsyncComplete(false, pE->getTextAndPlace().c_str());
        }

        void LogDri(QString msg)
        {
            AsyncOutput(msg);
            AsyncFlush();
        }

        void AbortTest()  // esc prc
        {
            if (m_test) m_test->AbortTest();
            AsyncComplete(false, "Aborted by user");    
        }

    protected:

        TestStarter(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : NamedObject(pDomain, name)
        {
            m_pProfile = new TTest::TNObjProfile(this, "Profile");
        }

        void Run(DRI::IAsyncCmd *pAsyncCmd) // Q_Invokable?
        {
            AsyncBegin(
                pAsyncCmd,
                new Utils::VIMethod<TMy>(&TMy::AbortTest, *this)
                );

            try
            {
                m_test.reset(new TTest(getDomain(), *m_pProfile));                      
            }
            catch(const ESS::BaseException& e)
            {
                AsyncComplete(false, e.getTextMessage().c_str());
            }            
        }
    };
} // namespace IsdnTest

#endif
