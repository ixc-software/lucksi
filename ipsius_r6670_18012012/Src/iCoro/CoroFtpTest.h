#pragma once

#include "Utils/IVirtualDestroy.h"

#include "Domain/NamedObject.h"

namespace iCoro
{

    class NObjCoroFtpTest : public Domain::NamedObject 
    {
        Q_OBJECT;

        typedef NObjCoroFtpTest T; 

        boost::scoped_ptr<Utils::IVirtualDestroy> m_body;

        void OnCompleted(QString info)
        {
            PutMsg(this, &T::DoComplete, info);
        }

        void DoComplete(QString info)
        {
            m_body.reset();

            if ( info.isEmpty() ) AsyncOutput("OK");

            AsyncComplete(info.isEmpty() ? true : false, info);
        }

        void DoAbort()
        {
            m_body.reset();
            AsyncComplete(false, "Aborted");
        }

    public: 

        NObjCoroFtpTest(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
          Domain::NamedObject(pDomain, name)
        {
        }

        Q_INVOKABLE void Run(DRI::IAsyncCmd *pAsyncCmd, QString srcDir, QString dstDir);

    };
        
}  // namespace iCoro