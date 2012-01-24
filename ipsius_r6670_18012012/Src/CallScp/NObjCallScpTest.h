#pragma once 

#include "Utils/IVirtualDestroy.h"
#include "Utils/FuncRAII.h"

#include "iCore/MsgThreadFn.h"
#include "Domain/NamedObject.h"

namespace CallScp
{

    class CallState : public QObject
    {
        Q_OBJECT;

    public:

        enum State
        {
            trying,
            alerting,
            talking,
            disconnected,
        };

        Q_ENUMS(State);

    };

    // ----------------------------------------------------------

    class NObjScpTest : public Domain::NamedObject 
    {
        Q_OBJECT;

        typedef NObjScpTest T; 

        boost::scoped_ptr<Utils::IVirtualDestroy> m_body;

        void OnCompleted(QString info)
        {
            // iCore::MsgThreadFn( getMsgThread(), boost::bind(&T::DoComplete, this, ok, info) );
            PutMsg(this, &T::DoComplete, info);
        }

        void DoComplete(QString info)
        {
            m_body.reset();

            if ( info.isEmpty() ) AsyncOutput("OK");

            AsyncComplete(info.isEmpty() ? true : false, info);
        }

    public: 

        NObjScpTest(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
          Domain::NamedObject(pDomain, name)
        {
        }

        Q_INVOKABLE void Run(DRI::IAsyncCmd *pAsyncCmd, QString scpName, bool trace);

    };

    // ----------------------------------------------------------

    class NObjScpTest2 : public Domain::NamedObject 
    {
        Q_OBJECT;

        typedef NObjScpTest2 T; 

        boost::scoped_ptr<Utils::IVirtualDestroy> m_body;

        void OnCompleted(QString info)
        {
            // iCore::MsgThreadFn( getMsgThread(), boost::bind(&T::DoComplete, this, ok, info) );
            PutMsg(this, &T::DoComplete, info);
        }

        void Trace(const QString &msg)
        {
            if (IsLogActive(InfoTag)) Log(InfoTag) << msg << iLogW::EndRecord;
        }

        void DoComplete(QString info)
        {
            m_body.reset();

            if ( info.isEmpty() ) AsyncOutput("OK");

            AsyncComplete(info.isEmpty() ? true : false, info);
        }

    public: 

        NObjScpTest2(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
            Domain::NamedObject(pDomain, name)
            {
            }

        Q_INVOKABLE void Run(DRI::IAsyncCmd *pAsyncCmd, 
            const QString &scpName, bool trace);

    };
    
}  // namespace CallScp

