#ifndef NOBJTASKUPDATEBOOTER_H
#define NOBJTASKUPDATEBOOTER_H

#include "BfBootCli/ITaskManager.h"
#include "BfBootCore/IHwFirmware.h"
#include "NObjCommonBfTaskProfile.h"
#include "ProfileHolder.h"

namespace BfBootDRI
{
    class NObjBfTaskUpdateBooter : public Domain::NamedObject, public ITask
    {
        Q_OBJECT
    public:
        NObjBfTaskUpdateBooter(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : Domain::NamedObject(pDomain, name),
            m_prof(*this)
        {}

        Q_INVOKABLE void Init(QString profile);
        Q_INVOKABLE void SetBootImage(QString file);
        Q_INVOKABLE void Run(DRI::IAsyncCmd *pAsyncCmd);

    // ITask
    private:
        void Run(bool& isLastTask);
        void RunNext()
        {
            ESS_UNIMPLEMENTED;
        }

    private:
        ProfileHolder m_prof;        
        boost::shared_ptr<BfBootCli::ITaskManager> m_task;
        boost::scoped_ptr<BfBootCore::IHwFirmwareImage> m_bootImage;
    };
} // namespace BfBootDRI

#endif
