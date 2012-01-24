#include "stdafx.h"

#include "ScriptCore.h"
#include "Parser.h"

#include "iCore/MsgScheduler.h"
#include "iCore/MsgErrHookFn.h"

#include "CallScp/NObjCallScpTest.h"
#include "iLog/LogWrapper.h"


namespace
{
    const char *CScpStartCommand = "start";

    using namespace CallScp;
    // compile time
    ESS_TYPEDEF(BadCommandName);

    // runtime
    ESS_TYPEDEF_T(WaitException, int);  // for pass wait param to ExecutionStream

    // ------------------------------------------------------------

    class Cmd : public BasicCommand
    {
        ScriptMethod m_fn;

    public:

        Cmd(const ScriptMethod &fn,
            const QString &cmd,
            const QStringList &args,
            const QString &locationInfo) :
            BasicCommand(cmd, args, locationInfo),
            m_fn(fn)
        {
            ESS_ASSERT(!m_fn.empty());
        }

        void Exec(IExecutionStream &stream) const
        {
            ESS_ASSERT (!m_fn.empty());
            m_fn(stream);
        }
    };

    // ------------------------------------------------------------

    class IExeStreamOwner : public Utils::IBasicInterface
    {
    public:
        virtual void ExecutionTrace(const QString &msg) = 0;
        virtual void StreamCompleted(const IExecutionStream &stream) = 0;
        virtual void ScheduleMsg(const boost::function<void ()> &fn, int delayMs) = 0;
    };

    // ------------------------------------------------------------

    class ExecutionStream : boost::noncopyable,
        public IExecutionStream
    {
        IExeStreamOwner &m_owner;
        const QString m_className;
        const QString m_objectName;
        const QString m_eventName;
        const CmdStream<Cmd> m_cmds;
        const IExecutionStream *m_pWaitCompleted;

        int m_cmdToRun;

    private:

        QString CmdInfo(const Cmd &c)
        {
            QString msg = QString("%1> %2: %3")
                .arg( m_className )
                .arg( m_cmds.StreamHeader() )
                .arg( c.Info() );

            return msg;
        }

        // ------------------------------------------------------------

        void TraceCmd(const Cmd &c)
        {
            m_owner.ExecutionTrace( CmdInfo(c) );
        }

        // ------------------------------------------------------------

        void ProcessCmd()  // note: completly synced code
        {
            // completed
            if (Completed())
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
            catch(ScriptCore::RuntimeError &e)  // rethrow with command info
            {
                QString msg = QString("Error '%1' for command [%2]")
                    .arg( e.getTextMessage().c_str() )
                    .arg( CmdInfo(c) );

                ESS_THROW_MSG( ScriptCore::RuntimeError, msg.toStdString() );
            }

            // next
            if (m_pWaitCompleted != 0) return;   // execution suspended

            RequestNextCmd(delayMs);
        }

        // ------------------------------------------------------------

        void RequestNextCmd(int delayMs = 0)
        {
            m_owner.ScheduleMsg(
                boost::bind(&ExecutionStream::ProcessCmd, this),
                delayMs
                );
        }

    public:

        ExecutionStream(IExeStreamOwner &owner,
            const QString &className,
            const QString &objectName,
            const QString &eventName,
            const CmdStream<Cmd> &cmds) :
            m_owner(owner),
            m_className(className),
            m_objectName(objectName),
            m_eventName(eventName),
            m_cmds(cmds),
            m_pWaitCompleted(0)
        {
            m_cmdToRun = 0;
            RequestNextCmd();

            m_owner.ExecutionTrace( Info() );
        }

            // ------------------------------------------------------------

        void StreamFinishNotify(const IExecutionStream *pFinished)
        {
            if (pFinished == this) return;

            if (pFinished == m_pWaitCompleted)   // resume
            {
                m_pWaitCompleted = 0;
                RequestNextCmd();
            }
        }


    // IExecutionStream
    public:

        const QString& EventName() const { return m_eventName; }
        const QString& ObjectName() const { return m_objectName; }
        const QString& ClassName() const { return m_className; }

        QString Info() const
        {
            return QString("%1::%2 start '%3'").arg(m_className).arg(m_objectName).arg(m_eventName);
        }

        bool Completed() const
        {
            return m_cmdToRun >= m_cmds.Size();
        }

        void WaitCompleted(IExecutionStream *pOther)
        {
            ESS_ASSERT(pOther != 0);
            ESS_ASSERT(m_pWaitCompleted == 0);

            m_pWaitCompleted = pOther;
        }

    };

}

namespace CallScp
{
    class ScriptCore::Impl : boost::noncopyable,
        IExeStreamOwner,
        public ICmdMaker<Cmd>
    {
        typedef Impl T;

    public:

        Impl(iCore::MsgThread &thread, 
            TraceFn trace,
            IScriptCoreUser &user) :
            m_trace(trace),
            m_errHook( thread, boost::bind(&Impl::ExceptionHook, this, _1, _2) ),
            m_scheduler(thread),
            m_user(user)
        {
        }

        // ------------------------------------------------------------

        void AddCommand(const QString &commandName, ScriptMethodBinder binderFn)
        {
            ESS_ASSERT( m_commandsMap.find(commandName) == m_commandsMap.end() );

            m_commandsMap[commandName] = binderFn;
        }

        // --------------------------------------------------------

        void AddCommand(CommonCmd command)
        {
            switch(command)
            {
            case cPass:
                AddCommand("pass", scriptMethodBind(this, &T::CmdPass));
            break;
            case cPrint:
                AddCommand("print", scriptMethodBind(this, &T::CmdPrint));
            break;
            case cWait:
                AddCommand("wait", scriptMethodBind(this, &T::CmdWait));
            break;
            case cAssert:
                AddCommand("assert", scriptMethodBind(this, &T::CmdAssert));
            break;
            case cRun:
                AddCommand("run", scriptMethodBind(this, &T::CmdRun));
            break;
            case cEnd:
                AddCommand("end", scriptMethodBind(this, &T::CmdEnd));
            break;
            default:
                ESS_HALT ("Unknown cmd");
            }
        }

        // --------------------------------------------------------

        void SetHookOnUnknownCommand(ScriptMethodBinder binder)
        {
            m_hookUnknownCom = binder;
        }

        // --------------------------------------------------------

        void Run(const QString &scpName,
            const QStringList &startupClasses)
        {
            m_cmd.reset( Load(scpName) );
            m_trace( m_cmd->Info() + "\n");

            m_startupClasses = startupClasses;

            ESS_ASSERT(m_startupClasses.size() > 0);

            for (QStringList::const_iterator classIt = m_startupClasses.begin();
                classIt != m_startupClasses.end(); ++classIt)
            {
                QStringList events = m_cmd->EnumerateEvents(*classIt);

                if (!events.contains(CScpStartCommand))
                    ESS_THROW_MSG( ScriptCore::RuntimeError, "Start event not found for class " + classIt->toStdString());

                StartStream(*classIt, *classIt, CScpStartCommand);

            }
        }

        // ------------------------------------------------------------

        void ScheduleUserMsg(boost::function<void ()> fn, int delayMs)
        {
            m_scheduler.PutMsg(fn, delayMs);
        }

        // ------------------------------------------------------------

        ExecutionStream* StartStream(const QString &className,
            const QString &objectName,
            const QString &eventName,
            bool allowEmpty = false)
        {
            CmdStream<Cmd> s =
                m_cmd->FindCmdStream(className.section('.', -1), eventName);

            if ( s.Empty() )
            {
                if (!allowEmpty)
                {
                    QString desc = QString("Class: %1, obj: %2, event: %3").
                        arg((className.isEmpty()) ? "unknown" : className).
                        arg(objectName).arg(eventName);

                    ESS_THROW_MSG( ScriptCore::RuntimeError, 
                        "ScriptCore. Null in StartStream. " + desc.toStdString());
                }

                return 0;
            }

            ExecutionStream *p = new ExecutionStream( *this, className, objectName, eventName, s );
            m_streams.Add(p);

            return p;
        }

    // ICmdMaker
    private:

        bool VerifyEventName(const QString &className, const QString &eventName, /* out */ QString &error)
        {
            return m_user.VerifyEventName(className, eventName, error);
        }

        // ------------------------------------------------------------

        Cmd* CreateCmd(const QString &cmd, const QStringList &args, const QString &locationInfo)
        {
            CommandsMap::const_iterator i = m_commandsMap.find(cmd);

            ScriptMethod fn;
            if ( i != m_commandsMap.end() ) fn = (i->second)(args);

            if (fn.empty() && !m_hookUnknownCom.empty())  fn = m_hookUnknownCom(args);

            if (fn.empty()) ESS_THROW(BadCommandName);

            return new Cmd( fn, cmd, args, locationInfo );
        }

    private:

        IScpTree<Cmd>* Load(const QString &scpName, const QString &className = "")
        {
            Utils::StringList sl;
            sl.LoadFromFile(scpName);

            return Parser<Cmd>::CreateTree( sl, *this);
        }

        // ------------------------------------------------------------

        void DeleteStream(const IExecutionStream *pS)
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

        // ------------------------------------------------------------

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

            m_user.ScriptComplete(msg);
        }

        // ------------------------------------------------------------

        bool ExceptionHook(const std::exception &e, iCore::MsgBase*)
        {
            const ScriptCore::RuntimeError *pE = dynamic_cast<const ScriptCore::RuntimeError*>(&e);

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

        void ExecutionTrace(const QString &msg)
        {
            m_trace(msg);
        }

        // ------------------------------------------------------------

        void StreamCompleted(const IExecutionStream &stream)
        {
            m_scheduler.PutMsg(
                boost::bind(&Impl::DeleteStream, this, &stream)
                );
        }

        // ------------------------------------------------------------

        void ScheduleMsg(const boost::function<void ()> &fn, int delayMs)
        {
            ScheduleUserMsg(fn, delayMs);
        }

    // commands impl
    private:

        void CmdPass(IExecutionStream&)
        {
            // nothing
        }

        // ------------------------------------------------------------

        void CmdPrint(IExecutionStream&, QString str)
        {
            m_trace(str);
        }

        // ------------------------------------------------------------

        void CmdWait(IExecutionStream&, int delayMs)
        {
            ESS_THROW_T(WaitException, delayMs);
        }


        // ------------------------------------------------------------

        void CmdAssert(IExecutionStream&, QString obj, QString op, QString val)
        {
            // ESS_HALT("Script assert");
        }

        // ------------------------------------------------------------

        void CmdEnd(IExecutionStream &s)
        {
            QString className = s.ClassName().split(".").first();

            if ( m_completedClasses.contains(className) )
            {
                ESS_THROW_MSG( ScriptCore::RuntimeError, "Double 'end' for " + className.toStdString() );
            }

            m_completedClasses.push_back(className);

            if ( m_startupClasses.size() == m_completedClasses.size() )
            {
                Finish("");  // OK
            }

        }

        // ------------------------------------------------------------

        void CmdRun(IExecutionStream &s, QString className, QString eventName)
        {
            QString objectName = (className == "self") ? s.ObjectName() : className;

            if (className == "self") className = s.ClassName();

            ExecutionStream *pS = StartStream(className, objectName, eventName);

            ESS_ASSERT(pS != 0);

            s.WaitCompleted(pS);
        }

    private:

        TraceFn m_trace;

        // init
        iCore::MsgErrHookFn m_errHook;
        IScriptCoreUser &m_user;

        boost::scoped_ptr< IScpTree<Cmd> > m_cmd;
        iCore::MsgScheduler m_scheduler;

        typedef std::map<QString, ScriptMethodBinder> CommandsMap;  // command -> function
        CommandsMap m_commandsMap;
       
        ScriptMethodBinder m_hookUnknownCom;

        // runtime
        Utils::ManagedList<ExecutionStream> m_streams;

        QStringList m_startupClasses;
        QStringList m_completedClasses;

    };


}

// ------------------------------------------------------------

namespace CallScp
{
    ScriptCore::ScriptCore(iCore::MsgThread &thread, 
        TraceFn trace,
        IScriptCoreUser &user) :
        m_impl(new Impl(thread, trace, user))
    {

    }

    // --------------------------------------------------------

    ScriptCore::~ScriptCore()
    {}

    // --------------------------------------------------------

    void ScriptCore::Run(const QString &scpName,
        const QStringList &startupClasses)
    {
        m_impl->Run(scpName, startupClasses);
    }

    // --------------------------------------------------------

    void ScriptCore::AddCommand(const QString &commandName, ScriptMethodBinder binder)
    {
        m_impl->AddCommand(commandName, binder);
    }

    // --------------------------------------------------------

    void ScriptCore::AddCommand(CommonCmd command)
    {
        m_impl->AddCommand(command);
    }

    // --------------------------------------------------------

    void ScriptCore::SetHookOnUnknownCommand(ScriptMethodBinder binder)
    {
        m_impl->SetHookOnUnknownCommand(binder);
    }

    // --------------------------------------------------------

    void ScriptCore::ScheduleMsg(boost::function<void ()> fn, int delayMs)
    {
        m_impl->ScheduleUserMsg(fn, delayMs);
    }

    // --------------------------------------------------------

    void ScriptCore::StartStream(const QString &className,
        const QString &objectName,
        const QString &eventName,
        bool allowEmpty)
    {
        m_impl->StartStream(className, objectName, eventName, allowEmpty);
    }

}  // namespace CallScp


