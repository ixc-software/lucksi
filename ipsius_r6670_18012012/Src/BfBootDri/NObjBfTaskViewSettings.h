#ifndef NOBJTASKVIEWSETTINGS_H
#define NOBJTASKVIEWSETTINGS_H

#include "NObjCommonBfTaskProfile.h"
#include "BfBootCli/TaskComposer.h"
#include "ProfileHolder.h"


namespace BfBootDRI
{
    class NObjBfTaskViewSettings : public Domain::NamedObject, public ITask
    {                
        Q_OBJECT

    // DRI intf:
    public:
        
        NObjBfTaskViewSettings(Domain::IDomain *pDomain, const Domain::ObjectName &name)            
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

    /*
        public:
                NObjBfTaskViewSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name)
                    : Domain::NamedObject(&pParent->getDomain(), name, pParent),            
                    m_prof(*this),
                    m_taskResult(new std::vector<BfBootCore::ConfigLine>)
                {}*/
        

    // ITask
    private:
        void Run(bool& isLastTask)
        {          
            isLastTask =true;
            m_task = BfBootCli::TaskComposer::ViewBoardParams(
                m_prof->getMngProf(), 
                m_prof->getPwd(),                
                m_prof->getTransport(), 
                m_taskResult,
                m_prof->getTraceServer()
                );
        }
        void RunNext()
        {
            ESS_UNIMPLEMENTED;
        }

    private:
        ProfileHolder m_prof;        
        boost::shared_ptr<BfBootCli::ITaskManager> m_task;
        boost::shared_ptr<std::vector<BfBootCore::ConfigLine> > m_taskResult;
    };
} // namespace BfBootDRI

#endif
