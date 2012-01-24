#include "stdafx.h"

#include "CallScp/NObjCallScpTest.h"
#include "CallScp/ScriptCore.h"
#include "Utils/FullEnumResolver.h"


// --------------------------------------------------------

namespace
{
    using namespace CallScp;

    class Call;
    typedef boost::function<void (Call&)> CallChangedFn;

    // --------------------------------------------------------

    QStringList ConvertSToQStringList(const QList<std::string> &list)
    {
        typedef QList<std::string> List;
        QStringList res;
        for (List::const_iterator i = list.begin(); i != list.end(); ++i)
        {
            res << (*i).c_str();
        }
        return res;
    }

    // --------------------------------------------------------

    QStringList AddPrefixToQStringList(const QString &prefix, const QStringList &list)
    {
        QStringList res;
        for (QStringList::const_iterator i = list.begin(); i != list.end(); ++i)
        {
            res << prefix + *i;
        }
        return res;
    }

    // ------------------------------------------------------------

    class Call : boost::noncopyable
    {
    public:

        enum Direction
        {
            Incoming,
            Outgoing
        };

        Call(const QString &name,
            Direction dir,
            const QString &localAddr,
            const QString &remoteAddr,
            const CallChangedFn &fn) :
            m_name(name),
            m_dir(dir),
            m_localAddr(localAddr),
            m_remoteAddr(remoteAddr),
            m_notify(fn)
        {
            SetState(
                (dir == Incoming) ? CallState::alerting : CallState::trying);
        }

        // --------------------------------------------------------

        const QString& Name() const { return m_name; }
        bool IsIncoming() const {   return m_dir == Incoming; }
        const QString& LocalAddr()  const { return m_localAddr; }
        const QString& RemoteAddr() const { return m_remoteAddr; }

        // --------------------------------------------------------

        CallState::State State() const { return m_state; }

        // --------------------------------------------------------

        void Accept()
        {
            SetState(CallState::talking);
        }

        // --------------------------------------------------------

        void Drop()
        {
            SetState(CallState::disconnected);
        }

    private:

        void SetState(CallState::State state)
        {
            m_state = state;
            m_notify(*this);
        }

    private:
        QString m_name;
        Direction m_dir;
        QString m_localAddr;
        QString m_remoteAddr;

        CallChangedFn m_notify;
        CallState::State m_state;
    };

    // ------------------------------------------------------------

    class Body : boost::noncopyable,
        public Utils::IVirtualDestroy,
        IScriptCoreUser
    {
        typedef Body T;

    public:

        Body(iCore::MsgThread &thread,
            const QString &scpName,
            ScriptCore::TraceFn traceFn,
            boost::function<void (QString)> fn) :
            m_scriptCore(thread, traceFn, *this),
            m_completedFn(fn)
        {

            m_eventsMap["trainer"] = QStringList("start");
            m_eventsMap["pult"] = QStringList("start");

            {
                using Utils::FullEnumResolver;

                const FullEnumResolver::EnumResolver *p =
                    FullEnumResolver::Instance().GetResolver<CallState::State>();

                ESS_ASSERT(p != 0);

                QStringList res = ConvertSToQStringList(p->AllValues());

                m_eventsMap["c1"] = AddPrefixToQStringList("i.", res) +
                    AddPrefixToQStringList("o.", res);
            }

            m_scriptCore.AddCommonCommands();
            m_scriptCore.AddCommand("createOutgoingCall", scriptMethodBind(this, &T::CmdCreateOutCall));
            m_scriptCore.AddCommand("incomingCallHandler",  scriptMethodBind(this, &T::IncomingCallHandler));
            m_scriptCore.AddCommand("lp_ring",   scriptMethodBind(this, &T::CmdLpRing));
            m_scriptCore.AddCommand("lp_accept", scriptMethodBind(this, &T::CmdLpAccept));
            m_scriptCore.AddCommand("call_disc", scriptMethodBind(this, &T::CmdCallDisc));

            m_scriptCore.Run(scpName, QString("trainer,pult").split(","));
        }

    // IScriptCoreUser
    private:

        bool VerifyEventName(const QString &className, const QString &eventName, /* out */ QString &error)
        {
            EventsMap::const_iterator i = m_eventsMap.find(className);

            return i != m_eventsMap.end() && i->second.contains(eventName);
        }

        void ScriptComplete(const QString &info)
        {
            m_completedFn(info);
        }

    private:

        Call* FindCall(const QString &name)
        {
            for(int i = 0; i < m_calls.Size(); ++i)
            {
                if (m_calls[i]->Name() == name) return m_calls[i];
            }

            return 0;
        }

        // --------------------------------------------------------

        void TryRunCallEvent(Call &call, const QString &event)
        {
            m_scriptCore.StartStream(call.Name(),
                (call.IsIncoming() ? "i." : "o.") + event,
                false);
        }

        // --------------------------------------------------------

        void DoCallChanged(Call &call)
        {
            Call *opositCall = FindCall(
                (call.IsIncoming()) ? m_outCallName : m_inCallName);

            bool err;
            QString event = Utils::EnumResolve(call.State(), &err).c_str();

            TryRunCallEvent(call, event);

            if (opositCall != 0) TryRunCallEvent(*opositCall, event);
        }

        // --------------------------------------------------------

        void OnCallChanged(Call &call)
        {
            m_scriptCore.ScheduleMsg(
                boost::bind(&Body::DoCallChanged, this, boost::ref(call))
                );
        }

        // --------------------------------------------------------

        Call& SmartFindCall(IExecutionStream &s, QString name)
        {
            if ( name.startsWith("self.") || (name == "self") )
            {
                name = s.ClassName();
            }

            Call *p = FindCall(name);

            if (p == 0)
            {
                ESS_THROW_MSG( ScriptCore::RuntimeError, QString("Can't find call %1").arg(name).toStdString() );
            }

            return *p;
        }

        // --------------------------------------------------------

        void CreateCall(const QString &callName,
            Call::Direction dir,
            const QString &localAddr,
            const QString &remoteAddr)
        {
            if ( FindCall(callName) )
            {
                ESS_THROW_MSG( ScriptCore::RuntimeError, "Duplicate call " + callName.toStdString() );
            }

            CallChangedFn fn = boost::bind(&Body::OnCallChanged, this, _1);

            // create and register
            Call *p = new Call(callName,
                dir,
                localAddr,
                remoteAddr,
                fn);

            m_calls.Add(p);
        }

        // --------------------------------------------------------

        void IncomingCall (const QString &src)
        {
            CreateCall(m_inCallName,
                Call::Incoming,
                m_inCallName,
                src);
        }

    // commands impl
    private:

        void IncomingCallHandler(IExecutionStream &s,
            QString callName)
        {
            m_inCallName =
                QString("%1.%2").arg(s.ClassName()).arg(callName);
        }

        // --------------------------------------------------------

        void CmdCreateOutCall(IExecutionStream &s,
            QString callName,
            QString dst)
        {
            m_outCallName =
                QString("%1.%2").arg(s.ClassName()).arg(callName);

            CreateCall(m_outCallName,
                Call::Outgoing,
                m_outCallName,
                dst);

            m_scriptCore.ScheduleMsg(
                boost::bind(&Body::IncomingCall, this, m_outCallName));
        }

        // --------------------------------------------------------

        void CmdLpRing(IExecutionStream&, QString convName)
        {
            // nothing
        }

        // --------------------------------------------------------

        void CmdLpAccept(IExecutionStream &s, QString convName)
        {
            Call &call = SmartFindCall(s, convName);
            call.Accept();
        }

        // --------------------------------------------------------

        void CmdCallDisc(IExecutionStream &s, QString callName)
        {
            Call &call = SmartFindCall(s, callName);
            call.Drop();
        }

    private:

        ScriptCore m_scriptCore;

        boost::function<void (QString)> m_completedFn;
        QString m_outCallName;
        QString m_inCallName;

        typedef std::map<QString, QStringList> EventsMap;  // class name -> events
        EventsMap m_eventsMap;

        // runtime
        Utils::ManagedList<Call> m_calls;

    };


}  // namespace


// --------------------------------------------------------

namespace CallScp
{

    Q_INVOKABLE void NObjScpTest2::Run(DRI::IAsyncCmd *pAsyncCmd, const QString &scpName, bool trace)
    {
        m_body.reset(
            new Body( getMsgThread(), scpName, boost::bind(&T::Trace, this, _1), boost::bind(&T::OnCompleted, this, _1) )
            );

        AsyncBegin(pAsyncCmd);
    }





}  // namespace CallScp
