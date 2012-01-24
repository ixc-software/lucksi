#ifndef NOBJHWFINDERTESTER_H
#define NOBJHWFINDERTESTER_H

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "Domain/NObjLogSettings.h"


namespace HiLevelTests
{
    namespace HwFinder
    {    


        class TaskWaitAlloc;
        class X;

        class NObjHwFinderTester
            : public Domain::NamedObject
           
        {
            Q_OBJECT;           
            
        public:
            NObjHwFinderTester(Domain::IDomain *pDomain, const Domain::ObjectName &name); 
            ~NObjHwFinderTester();

            Q_INVOKABLE void AddTaskWaitAlloc(QString taskName, int hwNum, int fwuVer, int timeout = -1);
            Q_INVOKABLE void StartAllTask(DRI::IAsyncCmd* pAsyncCmd);        

        private:       
            void StartTaskPoll();
            void OnPollTimer(iCore::MsgTimer* pTimer);                
            void Log(QString);

        private:                                

            Utils::ManagedList<TaskWaitAlloc> m_task;
            iCore::MsgTimer m_timer;                
        };


    } // namespace HwFinder
} // namespace HiLevelTests

#endif
