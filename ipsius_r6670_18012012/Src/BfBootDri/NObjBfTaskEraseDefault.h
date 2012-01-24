#ifndef NOBJTASKERASEDEFAULT_H
#define NOBJTASKERASEDEFAULT_H

#include "NObjCommonBfTaskProfile.h"
#include "ProfileHolder.h"

#include "BfBootCli/ITaskManager.h"

#include "BfBootCli/TaskComposer.h"
#include "NObjBroadcastReceiver.h"

namespace BfBootDRI
{
    using boost::shared_ptr;

    class NObjBfTaskEraseDefault : public Domain::NamedObject, public ITask
    {
        Q_OBJECT;
    
    // DRI intf:
    public:
        NObjBfTaskEraseDefault(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : Domain::NamedObject(pDomain, name),
            m_prof(*this)
        {}

        Q_INVOKABLE void Init(QString profile)
        {
            std::string err;
            if (!m_prof.Init(profile, &err)) ThrowRuntimeException(err);            
        }        

        Q_INVOKABLE void Run(DRI::IAsyncCmd *pAsyncCmd)
        {
            m_prof->RunTask(pAsyncCmd, *this, Name());                       
        }  

    // ITask
    private:
        void Run(bool& isLastTask)
        {
            isLastTask = true;
            m_task = BfBootCli::TaskComposer::EraseDefault(
                m_prof->getMngProf(),
                m_prof->getPwd(),
                m_prof->getTransport(),               
                m_prof->getTraceServer()
                );
        }

        void RunNext()
        {
            ESS_UNIMPLEMENTED;
        }

    // fields:
    private:
        ProfileHolder m_prof;       
        shared_ptr<BfBootCli::ITaskManager> m_task;
    };

} // namespace BfBootDRI

#endif
