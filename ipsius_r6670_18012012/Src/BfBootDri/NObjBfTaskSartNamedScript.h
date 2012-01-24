#ifndef NOBJTASKSARTNAMEDSCRIPT_H
#define NOBJTASKSARTNAMEDSCRIPT_H

#include "NObjCommonBfTaskProfile.h"
#include "ProfileHolder.h"
#include "IpsiusService/HwFirmware.h"
#include "BfBootCli/ITaskManager.h"

#include "BfBootCli/TaskComposer.h"
#include "NObjBroadcastReceiver.h"

#include "BfBootCore/NamedScript.h"

namespace BfBootDRI
{
    class NObjBfTaskStartNamedScript : public Domain::NamedObject, public ITask
    {                
        Q_OBJECT

        // DRI intf:
    public:
        NObjBfTaskStartNamedScript(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : Domain::NamedObject(pDomain, name),
            m_prof(*this)
        {}

        Q_INVOKABLE void Init(QString profile)
        {
            std::string err;
            if (!m_prof.Init(profile, &err)) ThrowRuntimeException(err);            
        }

        Q_INVOKABLE void ScriptName(QString name)
        {
            if (BfBootCore::NamedScript::NameOfBootScript() == name.toStdString())
                ThrowRuntimeException(name.append(" is reserved name."));
            m_scriptName = name;
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
            m_task = BfBootCli::TaskComposer::StartNamedScript(
                m_prof->getMngProf(), 
                m_prof->getPwd(),
                m_scriptName.toStdString(),
                m_prof->getTransport(),                
                m_prof->getTraceServer()
                );
        }
        void RunNext()
        {
            ESS_UNIMPLEMENTED;
        }
    
    private:
        ProfileHolder m_prof;
        QString m_scriptName;

        boost::shared_ptr<BfBootCli::ITaskManager> m_task;
    };

} // namespace BfBootDRI

#endif
