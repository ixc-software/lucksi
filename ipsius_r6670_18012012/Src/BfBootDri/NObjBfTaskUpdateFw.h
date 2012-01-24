#ifndef NOBJTASKUPDATEFW_H
#define NOBJTASKUPDATEFW_H

#include "NObjCommonBfTaskProfile.h"
#include "ProfileHolder.h"
#include "IpsiusService/HwFirmware.h"
#include "BfBootCli/ITaskManager.h"

#include "BfBootCli/TaskComposer.h"
#include "NObjBroadcastReceiver.h"

namespace BfBootDRI
{
    using boost::scoped_ptr;
    using boost::shared_ptr;

    class NObjBfTaskUpdateFw : public Domain::NamedObject, public ITask
    {
        Q_OBJECT;
    // DRI intf:
    public:
        NObjBfTaskUpdateFw(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : Domain::NamedObject(pDomain, name),
            m_prof(*this),
            m_checkAfter(false)
        {}

        Q_INVOKABLE void Init(QString profile)
        {
            std::string err;
            if (!m_prof.Init(profile, &err)) ThrowRuntimeException(err);            
        }

        Q_INVOKABLE void SetFirmware(QString fileName)
        {            
            //use try&catch ?
            m_fwu.reset(new IpsiusService::HwFirmware(fileName));            
            ESS_ASSERT(m_fwu);
        }

        Q_INVOKABLE void Run(DRI::IAsyncCmd *pAsyncCmd)
        {
            NObjBroadcastReceiver* receiver = 0;            
            if (m_checkAfter) receiver = CreateReceiver();

            m_prof->RunTask(pAsyncCmd, *this, Name(), receiver);            
        }

        Q_PROPERTY(bool CheckAfter WRITE m_checkAfter READ m_checkAfter);        

    private:        

        NObjBroadcastReceiver* CreateReceiver()
        {            
            NObjBroadcastReceiver* receiver = new NObjBroadcastReceiver(this, "CompleteValidator");            

            if (!m_fwu) ThrowRuntimeException("NoTaskAssigned");            
            receiver->m_softNum = m_fwu->Release();            
            receiver->m_cmpPort = 0;

            receiver->m_srcAddress = m_prof->getBoardHostAddress();
            return receiver;
        }

    // ITask
    private:
        void Run(bool& isLastTask)
        {
            isLastTask = true;
            if (!m_fwu) ThrowRuntimeException("NoFirmware");

            m_task = BfBootCli::TaskComposer::UpdateFirmware(
                m_prof->getMngProf(), 
                m_prof->getTransport(),
                m_prof->getPwd(),
                *m_fwu,
                m_prof->getTraceServer()
                );
        }
        void RunNext()
        {
            ESS_UNIMPLEMENTED;
        }

        ProfileHolder m_prof;
        scoped_ptr<IpsiusService::HwFirmware> m_fwu;

        shared_ptr<BfBootCli::ITaskManager> m_task;

        bool m_checkAfter;
    };
} // namespace BfBootDRI

#endif
