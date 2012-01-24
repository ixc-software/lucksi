#ifndef TASKWAITALLOC_H
#define TASKWAITALLOC_H

#include "IpsiusService/NObjHwFinder.h"

namespace HiLevelTests
{
    namespace HwFinder
    {    


        class TaskWaitAlloc 
            : public Utils::SafeRefServer,
            public IpsiusService::IAllocBoard
        {
            QString m_alias;
            int m_hwNum;
            int m_fwuVer;  
            int m_timeOut;
            Utils::TimerTicks m_tickTimer;

            bool m_complete;
            bool m_ok;
            QString m_info;

            Utils::SafeRef<IpsiusService::NObjHwFinder> m_finder;

            void Complete(bool ok, QString info = "");
            bool Timeout();        

            // IAllocBoard
        private:
            void BoardAllocated(QString alias, const BfBootCore::BroadcastMsg &msg);
            void BoardUnallocated(QString alias);

        public:
            TaskWaitAlloc(QString alias, int hwNum, int fwuVer, int timeOut = -1);
            ~TaskWaitAlloc();

            bool Process();
            bool Start(Utils::SafeRef<IpsiusService::NObjHwFinder> finder);

            QString Info() const;
            bool Ok() const;
            QString Alias() const;                
        };


    } // namespace HwFinder
} // namespace HiLevelTests



#endif
