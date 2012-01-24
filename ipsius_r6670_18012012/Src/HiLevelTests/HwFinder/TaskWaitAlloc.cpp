#include "stdafx.h"
#include "TaskWaitAlloc.h"

namespace HiLevelTests
{
    namespace HwFinder
    {    


        TaskWaitAlloc::TaskWaitAlloc( QString alias, int hwNum, int fwuVer, int timeOut /*= -1*/ )
        {
            m_alias = alias;
            m_hwNum = hwNum;
            m_fwuVer = fwuVer;
            m_timeOut = timeOut;

            m_complete = false;
            m_ok = false;        
        }

        // ------------------------------------------------------------------------------------

        TaskWaitAlloc::~TaskWaitAlloc()
        {
            if (!m_finder.IsEmpty()) m_finder->FreeBoard(m_alias);
        }

        // ------------------------------------------------------------------------------------

        void TaskWaitAlloc::Complete( bool ok, QString info /*= ""*/ )
        {
            m_complete = true;
            m_ok = ok;
            m_info = info;
            
            ESS_ASSERT ( m_finder->FreeBoard(m_alias) );
            m_finder.Clear();
        }

        // ------------------------------------------------------------------------------------

        bool TaskWaitAlloc::Timeout()
        {
            return m_timeOut < 0 ? false : m_tickTimer.TimeOut();
        }

        // ------------------------------------------------------------------------------------

        void TaskWaitAlloc::BoardAllocated( QString alias, const BfBootCore::BroadcastMsg &msg )
        {
            if (Process()) return;

            if (msg.IsBooter()) 
            {
                Complete(false, QString("Test failed in %1. Allocated board in booter stage.").arg(m_alias) );
                return;
            }

            if (alias == m_alias && m_hwNum == msg.HwNumber && m_fwuVer == msg.SoftRelNumber) 
                Complete(true);
        }

        // ------------------------------------------------------------------------------------

        bool TaskWaitAlloc::Process()
        {
            if (!m_complete && Timeout()) Complete(false, QString("Failed by timeout in %1").arg(m_alias));             
            return m_complete;
        }

        // ------------------------------------------------------------------------------------

        QString TaskWaitAlloc::Info() const
        {
            ESS_ASSERT(m_complete);
            return m_info;
        }

        // ------------------------------------------------------------------------------------

        bool TaskWaitAlloc::Ok() const
        {
            ESS_ASSERT(m_complete);
            return m_ok;
        }

        // ------------------------------------------------------------------------------------

        QString TaskWaitAlloc::Alias() const
        {
            return m_alias;
        }

        // ------------------------------------------------------------------------------------    

        bool TaskWaitAlloc::Start(Utils::SafeRef<IpsiusService::NObjHwFinder> finder)
        {
            finder->Alias(m_hwNum, m_alias);
            if (m_timeOut >= 0) m_tickTimer.Set(m_timeOut, false);
            bool alloc = finder->AllocBoard(m_alias, this);            
            if (alloc) m_finder = finder;
            return alloc;
        }

        void TaskWaitAlloc::BoardUnallocated( QString alias )
        {
            ESS_ASSERT(m_complete);
            //std::cout << "Unalloc " << alias.toStdString() << std::endl;
        }

    } // namespace HwFinder
} // namespace HiLevelTests
