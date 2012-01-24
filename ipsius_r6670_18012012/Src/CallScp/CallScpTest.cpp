#include "stdafx.h"

#include "iCore/MsgScheduler.h"
#include "iCore/MsgErrHookFn.h"

#include "NObjCallScpTest.h"

#include "Parser.h"

// --------------------------------------------------------

namespace
{
    using namespace CallScp;

    class Call;
    class ExecutionStream;

    typedef boost::function<void (QString info)>        CompletedFn;
    typedef boost::function<void (Call&)>               CallChangedFn;
    typedef boost::function<void (ExecutionStream&)>    CmdFn;

    const QString CStartup = "startup";

    // compile time
    ESS_TYPEDEF(BadCommandName);
    ESS_TYPEDEF(BadCmdArgs);

    // runtime 
    ESS_TYPEDEF_T(WaitException, int);  // for pass wait param to ExecutionStream
    ESS_TYPEDEF(RuntimeError);

    // ------------------------------------------------------------

    class Cmd : public BasicCommand
    {
        CmdFn m_fn;

    public:        

        Cmd(const CmdFn &fn, QString cmd, QStringList args, QString locationInfo) : 
          BasicCommand(cmd, args, locationInfo),
          m_fn(fn)
        {
            ESS_ASSERT(fn);
        }

        void Exec(ExecutionStream &stream) const
        {
            m_fn(stream);
        }
    };

    // ------------------------------------------------------------

    class IExeStreamOwner : public Utils::IBasicInterface
    {
    public:
        virtual void ExecutionTrace(QString msg) = 0;
        virtual void StreamCompleted(ExecutionStream &stream) = 0;
        virtual void ScheduleMsg(const boost::function<void ()> &fn, int delayMs) = 0;
    };


    // ------------------------------------------------------------

    class ExecutionStream : boost::noncopyable
    {
        IExeStreamOwner &m_owner; 
        const QString m_roleName; 
        const CmdStream<Cmd> m_cmds;     
        ExecutionStream *m_pWaitCompleted;

        int m_cmdToRun;

        QString CmdInfo(const Cmd &c)
        {
            QString msg = QString("%1> %2: %3")
                .arg( m_roleName )
                .arg( m_cmds.StreamHeader() )
                .arg( c.Info() );

            return msg;
        }

        void TraceCmd(const Cmd &c)
        {
            m_owner.ExecutionTrace( CmdInfo(c) );
        }

        void ProcessCmd()  // note: completly synced code 
        {
            // completed 
            if (m_cmdToRun >= m_cmds.Size())    
            {
                m_owner.StreamCompleted(*this);
                return;
            }

            // peek next cmd 
            const Cmd &c = m_cmds[m_cmdToRun++];

            TraceCmd(c);

            int delayMs = 0;

            try
            {
                c.Exec(*this);
            }
            catch(const WaitException &e)
            {
                delayMs = e.getProperty();
            }
            catch(RuntimeError &e)  // rethrow with command info 
            {
                QString msg = QString("Error '%1' for command [%2]")
                    .arg( e.getTextMessage().c_str() )
                    .arg( CmdInfo(c) );

                ESS_THROW_MSG( RuntimeError, msg.toStdString() );
            }

            // next 
            if (m_pWaitCompleted != 0) return;   // execution suspended

            RequestNextCmd(delayMs);
        }

        void RequestNextCmd(int delayMs = 0)
        {
            m_owner.ScheduleMsg(
                boost::bind(&ExecutionStream::ProcessCmd, this),
                delayMs
                );                
        }

    public:

        ExecutionStream(IExeStreamOwner &owner, QString roleName, const CmdStream<Cmd> &cmds) :
          m_owner(owner), m_roleName(roleName), m_cmds(cmds), m_pWaitCompleted(0)
        {
            m_cmdToRun = 0;
            RequestNextCmd();

            m_owner.ExecutionTrace( Info() );
        }

        /*
        void Insert(CmdStream<Cmd> &cmds)
        {
            m_cmds = CmdStream<Cmd>(m_cmds, m_cmdToRun, cmds);
        } */

        const QString& RoleName() const { return m_roleName; }
        const QString& ClassName() const { return m_cmds.ClassName(); }

        const QString Info() const 
        { 
            return QString("%1 start '%2'").arg(m_roleName).arg( m_cmds.StreamHeader() );
        }

        bool Completed() const
        {
            return (m_cmdToRun >= m_cmds.Size());
        }

        void WaitCompleted(ExecutionStream *pOther)
        {            
            ESS_ASSERT(pOther != 0);
            ESS_ASSERT(m_pWaitCompleted == 0);

            m_pWaitCompleted = pOther;
        }

        void StreamFinishNotify(ExecutionStream *pFinished)
        {
            if (pFinished == m_pWaitCompleted)   // resume 
            {
                m_pWaitCompleted = 0;
                RequestNextCmd();
            }
        }

    };

    // ------------------------------------------------------------

    class Call : boost::noncopyable
    {
        const QString m_name, m_from, m_to; 
        CallChangedFn m_notify;

        CallState::State m_state;

        void SetState(CallState::State state)
        {
            m_state = state;
            m_notify(*this);
        }

    public:

        Call(QString name, QString from, QString to, const CallChangedFn &fn) :
          m_name(name), m_from(from), m_to(to), m_notify(fn)
        {
            SetState(CallState::alerting);
        }

        const QString& Name() const { return m_name; }
        const QString& From() const { return m_from; }
        const QString& To() const   { return m_to; }

        CallState::State State() const { return m_state; }

        void Accept()
        {
            SetState(CallState::talking);
        }

        void Drop()
        {
            SetState(CallState::disconnected);
        }

    };

    // ------------------------------------------------------------

    class Core : 
        public Utils::IVirtualDestroy, 
        public ICmdMaker<Cmd>,
        IExeStreamOwner,
        boost::noncopyable
    {

        class CmdFactory : boost::noncopyable
        {
            typedef CmdFactory T;
            typedef boost::function<CmdFn (QStringList args)> CmdBindFn;

            typedef std::map<QString, CmdBindFn> Map;  // command -> function


            Core &m_owner;
            Map m_map;

            void ArgCountCheck(const QStringList &args, int val)
            {
                if (args.size() != val) 
                {
                    QString msg = QString("Bad arg count, wait %1 is %2").arg(val).arg(args.size());
                    ESS_THROW_MSG(BadCmdArgs, msg.toStdString());
                }
            }

            void DuplicateCmdCheck(const QString &cmdName)
            {
                ESS_ASSERT( m_map.find(cmdName) == m_map.end() );
            }

            template<class T>
            T ArgTo(const QString &val);

            template<>
            QString ArgTo<QString>(const QString &val) { return val; }

            template<>
            int ArgTo<int>(const QString &s) 
            { 
                bool ok;
                int val = s.toInt(&ok);

                if (!ok) 
                {
                    QString msg = QString("Can't convert to int '%1'").arg(s);
                    ESS_THROW_MSG(BadCmdArgs, msg.toStdString());
                }

                return val;
            }

        // binders
        private:

            CmdFn BinderArg0(void (Core::*fn)(ExecutionStream&), QStringList args)
            {
                ArgCountCheck(args, 0);
                return boost::bind( fn, &m_owner, _1 );
            }

            template<class TArg0>
            CmdFn BinderArg1(void (Core::*fn)(ExecutionStream&, TArg0), QStringList args)
            {
                ArgCountCheck(args, 1);
                return boost::bind( fn, &m_owner, _1, ArgTo<TArg0>(args.at(0)) );
            }

            template<class TArg0, class TArg1>
            CmdFn BinderArg2(void (Core::*fn)(ExecutionStream&, TArg0, TArg1), QStringList args)
            {
                ArgCountCheck(args, 2);
                return boost::bind( fn, &m_owner, _1, ArgTo<TArg0>(args.at(0)), ArgTo<TArg1>(args.at(1)) );
            }

            template<class TArg0, class TArg1, class TArg2>
            CmdFn BinderArg3(void (Core::*fn)(ExecutionStream&, TArg0, TArg1, TArg2), QStringList args)
            {
                ArgCountCheck(args, 3);
                return boost::bind( fn, &m_owner, _1, ArgTo<TArg0>(args.at(0)), ArgTo<TArg1>(args.at(1)), ArgTo<TArg2>(args.at(2)) );
            } 

        // adders
        private:

            void Add( QString cmdName, void (Core::*fn)(ExecutionStream&) )
            {
                DuplicateCmdCheck(cmdName);
                m_map[cmdName] = boost::bind(&T::BinderArg0, this, fn, _1);
            }

            template<class TArg0>
            void Add( QString cmdName, void (Core::*fn)(ExecutionStream&, TArg0) )
            {
                DuplicateCmdCheck(cmdName);
                m_map[cmdName] = boost::bind(&T::BinderArg1<TArg0>, this, fn, _1);
            }

            template<class TArg0, class TArg1>
            void Add( QString cmdName, void (Core::*fn)(ExecutionStream&, TArg0, TArg1) )
            {
                DuplicateCmdCheck(cmdName);
                m_map[cmdName] = boost::bind(&T::BinderArg2<TArg0, TArg1>, this, fn, _1);
            }

            template<class TArg0, class TArg1, class TArg2>
            void Add( QString cmdName, void (Core::*fn)(ExecutionStream&, TArg0, TArg1, TArg2) )
            {
                DuplicateCmdCheck(cmdName);
                m_map[cmdName] = boost::bind(&T::BinderArg3<TArg0, TArg1, TArg2>, this, fn, _1);
            }

        public:

            CmdFactory(Core &owner) : m_owner(owner)
            {
                Add("pass",         &Core::CmdPass);
                Add("print",        &Core::CmdPrint);
                Add("wait",         &Core::CmdWait);
                Add("create",       &Core::CmdCreate);
                Add("lp_ring",      &Core::CmdLpRing);
                Add("lp_accept",    &Core::CmdLpAccept);
                Add("call_disc",    &Core::CmdCallDisc);
                Add("assert",       &Core::CmdAssert);
                Add("end",          &Core::CmdEnd);
                Add("run",          &Core::CmdRun);
            }

            CmdFn Bind(QString cmdName, QStringList args) const
            {
                Map::const_iterator i = m_map.find(cmdName);

                if ( i == m_map.end() ) 
                {
                    return CmdFn();
                }

                return (i->second)(args);
            }

        };

    private:

        // init
        iCore::MsgErrHookFn m_errHook;
        bool m_trace; 
        CompletedFn m_completeFn;
        boost::scoped_ptr< IScpTree<Cmd> > m_cmd;
        iCore::MsgScheduler m_scheduler;
        QStringList m_roles;

        // runtime
        Utils::ManagedList<Call> m_calls;
        Utils::ManagedList<ExecutionStream> m_streams;
        QStringList m_completedRoles;

        // static 
        CmdFactory m_factory;
        QList<std::string> m_correctCallStates;  

        IScpTree<Cmd>* Load(QString scpName)
        {
            Utils::StringList sl;
            sl.LoadFromFile(scpName);

            return Parser<Cmd>::CreateTree( sl, *this );
        }

        Call* FindCall(QString name)
        {
            for(int i = 0; i < m_calls.Size(); ++i)
            {
                if (m_calls[i]->Name() == name) return m_calls[i];
            }

            return 0;
        }

        void Trace(QString s)
        {
            if (!m_trace) return;
            std::cout << s.toStdString() << std::endl; 
        }

        ExecutionStream* StartStream(const QString &role, const QString &className, const QString &eventName, 
            bool allowEmpty = false)
        {
            CmdStream<Cmd> s = m_cmd->FindCmdStream(className, eventName);

            if ( s.Empty() ) 
            {
                if (!allowEmpty)
                {
                    ESS_THROW_MSG( RuntimeError, "Null in StartStream");
                }

                return 0;
            }

            ExecutionStream *p = new ExecutionStream( *this, role, s );
            m_streams.Add(p);

            return p;
        }

        void DeleteStream(ExecutionStream *pS)
        {
            ESS_ASSERT(pS != 0);

            // notify other
            for(int i = 0; i < m_streams.Size(); ++i)
            {
                m_streams[i]->StreamFinishNotify(pS);
            }

            // delete
            int i = m_streams.Find(pS);
            ESS_ASSERT(i >= 0);
            m_streams.Delete(i);
        }

        void VerifyRole(QString role)
        {
            if ( !m_roles.contains(role) )
            {                
                ESS_THROW_MSG( RuntimeError, QString("Bad role %1").arg(role).toStdString() );
            }

        }

        void TryRunCallEvent(Call &call, bool incoming)
        {
            QString eventName = incoming ? "i" : "o";
            eventName += ".";

            bool err;
            eventName += Utils::EnumResolve(call.State(), &err).c_str();
            ESS_ASSERT(!err);

            QString role = incoming ? call.To() : call.From();
            VerifyRole(role);

            StartStream(role, call.Name(), eventName, true);
        }

        void DoCallChanged(Call &call)
        {
            TryRunCallEvent(call, true);
            TryRunCallEvent(call, false);
        }


        void OnCallChanged(Call &call)
        {
            m_scheduler.PutMsg( 
                boost::bind(&Core::DoCallChanged, this, boost::ref(call)) 
                );
        }

        Call& SmartFindCall(ExecutionStream &s, QString name)
        {
            if ( name.startsWith("self.") || (name == "self") )
            {
                name = s.ClassName();
            }

            Call *p = FindCall(name);

            if (p == 0) 
            {
                ESS_THROW_MSG( RuntimeError, QString("Can't find call %1").arg(name).toStdString() );                
            }

            return *p;
        }

        void Finish(QString msg)
        {
            if ( msg.isEmpty() )
            {
                QStringList sl;

                for(int i = 0; i < m_streams.Size(); ++i)
                {
                    if ( !m_streams[i]->Completed() ) 
                    {
                        sl.append( m_streams[i]->Info() );
                    }
                }

                if ( sl.size() )
                {
                    msg = "Warning, uncompleted:\n" + sl.join("\n");
                }
            }

            m_completeFn(msg);
        }

        bool ExceptionHook(const std::exception &e, iCore::MsgBase*)
        {
            const RuntimeError *pE = dynamic_cast<const RuntimeError*>(&e);

            if (pE)
            {
                QString err = pE->getTextMessage().c_str();
                Finish(err);
                return true;
            }

            return false;
        }

    // IExeStreamOwner impl
    private:

        void ExecutionTrace(QString msg)
        {
            Trace(msg);
        }

        void StreamCompleted(ExecutionStream &stream)
        {
            m_scheduler.PutMsg(
                boost::bind(&Core::DeleteStream, this, &stream)
                );
        }

        void ScheduleMsg(const boost::function<void ()> &fn, int delayMs)
        {
            m_scheduler.PutMsg(fn, delayMs);
        }

    // commands impl 
    private:

        void CmdPass(ExecutionStream&)
        {
            // nothing
        }

        void CmdPrint(ExecutionStream&, QString msg)
        {
            Trace(msg);
        }

        void CmdWait(ExecutionStream&, int delayMs)
        {
            ESS_THROW_T(WaitException, delayMs);
        }

        void CmdCreate(ExecutionStream &s, QString className, QString dst)
        {
            CallChangedFn fn = boost::bind(&Core::OnCallChanged, this, _1);
            VerifyRole(dst);

            if ( FindCall( className ) ) 
            {
                ESS_THROW_MSG( RuntimeError, "Duplicate call " + className.toStdString() );
            }

            // create and register 
            Call *p = new Call(className, s.RoleName(), dst, fn);
            m_calls.Add(p);
        }

        void CmdLpRing(ExecutionStream&, QString convName)
        {
            // nothing
        }

        void CmdLpAccept(ExecutionStream &s, QString convName)
        {
            Call &call = SmartFindCall(s, convName);
            call.Accept();
        }

        void CmdCallDisc(ExecutionStream &s, QString callName)
        {
            Call &call = SmartFindCall(s, callName);
            call.Drop();
        }

        void CmdAssert(ExecutionStream&, QString obj, QString op, QString val)
        {
            // not implemented yet 
        }

        void CmdEnd(ExecutionStream &s)
        {
            QString role = s.RoleName();

            VerifyRole( role );

            if ( m_completedRoles.contains(role) )
            {
                ESS_THROW_MSG( RuntimeError, "Double 'end' for " + role.toStdString() );
            }

            m_completedRoles.push_back(role);

            if ( m_roles.size() == m_completedRoles.size() )
            {
                Finish("");  // OK
            }
        }

        void CmdRun(ExecutionStream &s, QString className, QString eventName)
        {
            ExecutionStream *pS = StartStream(s.RoleName(), className, eventName);
            ESS_ASSERT(pS != 0);

            s.WaitCompleted(pS);
        }

    private:

        bool VerifyEventName(const QString &className, const QString &eventName, /* out */ QString &error)
        {
            if (className == CStartup) return true;

            int n = eventName.indexOf(".");
            if (n < 0) return false;

            QString left = eventName.left(n);
            if (left != "i" && left != "o") return false;

            QString e = eventName.mid(n + 1);

            return m_correctCallStates.contains( e.toStdString() );
        }

        Cmd* CreateCmd(const QString &cmd, const QStringList &args, const QString &locationInfo)
        {
            CmdFn fn = m_factory.Bind(cmd, args);
            if (!fn) ESS_THROW(BadCommandName);

            return new Cmd( fn, cmd, args, locationInfo ); 
        }

    public:
        
        Core(iCore::MsgThread &thread, QString scpName, bool trace, CompletedFn fn) :
          m_errHook( thread, boost::bind(&Core::ExceptionHook, this, _1, _2) ),
          m_trace(trace),
          m_completeFn(fn),
          m_scheduler(thread),
          m_factory(*this)
        {
            {
                using Utils::FullEnumResolver;

                const FullEnumResolver::EnumResolver *p = 
                    FullEnumResolver::Instance().GetResolver<CallState::State>();
                ESS_ASSERT(p != 0);
                m_correctCallStates = p->AllValues();
            }

            m_cmd.reset( Load(scpName) );
            Trace( m_cmd->Info() + "\n");            

            m_roles = m_cmd->EnumerateEvents(CStartup);
            ESS_ASSERT(m_roles.size() >= 2);

            for(int i = 0; i < m_roles.size(); ++i)
            {
              QString role = m_roles.at(i);
              StartStream(role, CStartup, role);
            }
        }

    };


}  // namespace


// --------------------------------------------------------

namespace CallScp
{

    Q_INVOKABLE void NObjScpTest::Run(DRI::IAsyncCmd *pAsyncCmd, QString scpName, bool trace)
    {
        m_body.reset(
            new Core( getMsgThread(), scpName, trace, boost::bind(&T::OnCompleted, this, _1) )
            );

        AsyncBegin(pAsyncCmd);
    }

}  // namespace CallScp
