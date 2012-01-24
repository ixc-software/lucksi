#include "stdafx.h"
#include "NObjHwFinderTester.h"
#include "TaskWaitAlloc.h"

#include "IpsiusService/NObjHwFinder.h"

namespace HiLevelTests
{
    namespace HwFinder
    {    


        using IpsiusService::NObjHwFinder;      

        // ------------------------------------------------------------------------------------

        NObjHwFinderTester::NObjHwFinderTester(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : Domain::NamedObject(pDomain, name),            
            m_timer(this, &NObjHwFinderTester::OnPollTimer)
        {
        } 

        Q_INVOKABLE void NObjHwFinderTester::AddTaskWaitAlloc( QString taskName, int hwNum, int fwuVer, int timeout /*= -1*/ )
        {
            // todo недопускать дублирования 
            m_task.Add(new TaskWaitAlloc(taskName, hwNum, fwuVer, timeout)) ;
        }

        Q_INVOKABLE void NObjHwFinderTester::StartAllTask( DRI::IAsyncCmd* pAsyncCmd )
        {
            QString err;
            NObjHwFinder* pFinder = getDomain().FindFromRoot<NObjHwFinder>("HwFinder", &err);
            if (!pFinder) ThrowRuntimeException("Create HfFinder first.");

            AsyncBegin(pAsyncCmd);
            for(int i = 0; i < m_task.Size(); ++i)
            {
                if (!m_task[i]->Start(pFinder)) ThrowRuntimeException("Test failed - alias not found");
                Log(QString("Task with alias %1 started.").arg(m_task[i]->Alias()));
            }
            StartTaskPoll();
        }
       
        // ------------------------------------------------------------------------------------

        void NObjHwFinderTester::StartTaskPoll()
        {
            m_timer.Start(500, true);
        }

        void NObjHwFinderTester::OnPollTimer( iCore::MsgTimer* pTimer )
        {
            if (!AsyncActive()) return;       

            while (!m_task.IsEmpty())
            {
                if (!m_task.Front()->Process()) return;
                if (!m_task.Front()->Ok())
                {
                    m_timer.Stop();
                    QString info = m_task.Front()->Info();
                    m_task.Clear(); 
                    AsyncComplete(false, info);
                    return;
                }
                // complete, ok                        
                Log(QString("Task with alias %1 complete.").arg(m_task.Front()->Alias()));
                m_task.Delete(0);
            }        
            
            AsyncComplete(true);
        }   

        void NObjHwFinderTester::Log(QString msg)
        {
            AsyncOutput(msg);
            AsyncFlush();
        }

        NObjHwFinderTester::~NObjHwFinderTester()
        {
            m_task.Clear();
        }    

    } // namespace HwFinder
} // namespace HiLevelTests
