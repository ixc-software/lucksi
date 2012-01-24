#include "stdafx.h"

#include "NamedObjectTestType.h"

// ----------------------------------

namespace DriTests
{
    // ----------------------------------
    // NObjTestTypeInner impl
    
    NObjTestTypeInner::NObjTestTypeInner( NObjTestType *pParent, const ObjectName &name, int number ) : 
        NamedObject(&pParent->getDomain(), name, pParent),
        m_pParent(pParent),
        m_parentReqDelete(false)
    {
        Number(number, false);
    }

    // ----------------------------------

    NObjTestTypeInner::~NObjTestTypeInner()
    {
        if (m_parentReqDelete)
        {
            m_pParent->ChildDeleteDone();
        }
        
    }

    // ----------------------------------
    // NObjFlushTest impl

    NObjFlushTest::NObjFlushTest(IDomain *pDomain, const ObjectName &name) : 
        NamedObject(pDomain, name), 
        m_runTimeMs(0),
        m_flushTimer(this, &NObjFlushTest::OnFlushTimer),
        m_addTimer(this, &NObjFlushTest::OnAddTimer),
        m_isRunning(false),
        m_counter(0)
    {
        m_flushTimeoutMs = 400; 
        m_addTimeoutMs = 200;
    }

    // ----------------------------------

    void NObjFlushTest::OnFlushTimer(iCore::MsgTimer *p)
    {
        if (!m_isRunning) return;
        
        if (IsStopTime() && m_isRunning)
        {
            Stop(true);
            return;
        }
        
        AsyncFlush();
    }

    // ----------------------------------

    void NObjFlushTest::OnAddTimer(iCore::MsgTimer *p)
    {
        if (!m_isRunning) return;
        
        if (IsStopTime() && m_isRunning)
        {
            Stop(true);
            return;
        }
        
        // AsyncOutput(QString("some data (part%1)").arg(QString::number(m_counter)));

        /*
        AsyncOutput("1", false);
        AsyncFlush();
        AsyncOutput("2", true);
        AsyncFlush();

        AsyncOutput("34", true);
        AsyncFlush();
        */

        if (m_counter == 0)
        {
            AsyncOutput(QString("Line: *"), false);
        }

        if ((m_counter != 0) && ((m_counter % 10) == 0))
        {
            AsyncOutput(" [line_end]", true);
            AsyncOutput(QString("Line: *"), false);
        }

        AsyncOutput("*", false);
        
        ++m_counter;
    }

    // ----------------------------------

    void NObjFlushTest::RunInner(DRI::IAsyncCmd *pCmd, int runTimeMs, boost::function<void (void)> abortFn)
    {
        ESS_ASSERT(!m_isRunning);

        AsyncBegin(pCmd, abortFn);

        if (runTimeMs <= 0) AsyncComplete(false, "Invalid run time");

        m_startTime = QDateTime::currentDateTime();
        m_isRunning = true;
        m_runTimeMs = runTimeMs;
        m_flushTimer.Start(m_flushTimeoutMs, true);
        m_addTimer.Start(m_addTimeoutMs, true);
    }

    // ----------------------------------
    
    void NObjFlushTest::Stop(bool ok, const QString &err)
    {
        ESS_ASSERT(m_isRunning);
        m_flushTimer.Stop();
        m_addTimer.Stop();
        m_isRunning = false;
        
        AsyncComplete(ok, err);
    }

    // ----------------------------------

    void NObjFlushTest::AbortAsync() // override
    {
        Stop(false, "Aborted!");
    }

    // ----------------------------------

    void NObjFlushTest::AbortAsyncFake()
    {
        // Do nothing
    }

    // ----------------------------------

    NObjFlushTest::~NObjFlushTest()
    {
        // ...
    }

    // ----------------------------------
    
    Q_INVOKABLE void NObjFlushTest::RunInfinite(DRI::IAsyncCmd *pCmd)
    {
        AsyncBegin(pCmd);
        m_isRunning = true;
    }

    // ----------------------------------

    Q_INVOKABLE void NObjFlushTest::RunWithError(DRI::IAsyncCmd *pCmd)
    {
        AsyncBegin(pCmd);
        AsyncComplete(false, "Error in RunWithError(): OK");
        
    }

    // ----------------------------------
    
    Q_INVOKABLE void NObjFlushTest::Run(DRI::IAsyncCmd *pCmd, int runTimeMs)
    {
		RunInner(pCmd, runTimeMs, boost::bind(&T::AbortAsync, this));
    }
        
    // ----------------------------------

    Q_INVOKABLE void NObjFlushTest::RunWithFakeAbort(DRI::IAsyncCmd *pCmd, int runTimeMs)
    {
        RunInner(pCmd, runTimeMs, boost::bind(&T::AbortAsyncFake, this));
    }

    // ----------------------------------

    Q_INVOKABLE void NObjFlushTest::RunWithAsyncException(DRI::IAsyncCmd *pCmd)
    {
        AsyncBegin(pCmd);
        ESS_THROW(FlushTestException);
    }

    // ----------------------------------

    Q_INVOKABLE void NObjFlushTest::RunWithSyncException(DRI::IAsyncCmd *pCmd)
    {
        // check all here, throw exception if need 
        ESS_THROW(FlushTestException);

        // start
        AsyncBegin(pCmd);

        try
        {
            // some code with exception
            
        }
        catch (const std::exception &e)
        {
             AsyncComplete(false, DRI::DriUtils::ExceptionToString(e));
        }
    }

    // ----------------------------------

    Q_INVOKABLE void NObjFlushTest::RunSafeWithAsyncException(DRI::IAsyncCmd *pCmd)
    {
        // start
        AsyncBegin(pCmd);

        try
        {
            // some code with exception
            ESS_THROW(FlushTestException);
        }
        catch (const std::exception &e)
        {
            AsyncComplete(false, DRI::DriUtils::ExceptionToString(e));
        }
    }
    
}  // namespace DriTests

