#pragma once 

namespace CallScp
{
    class ExecutionStream;

    class IExeStreamOwner : public Utils::IBasicInterface
    {
    public:
        virtual void ExecutionTrace(const QString &msg) = 0;
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
            return m_cmdToRun >= m_cmds.Size();
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

}


namespace CallScp
{
    ScriptCore::ScriptCore()
    {

    }

    // --------------------------------------------------------

    void ScriptCore::Run(const QString &scpName, 
        bool trace, 
        boost::function<void (const QString &info)> completeFn)
    {

    }

    // --------------------------------------------------------

    void ScriptCore::AddCalss(const QString &class, 
        const QStringList &events)
    {

    }

    // --------------------------------------------------------

    void ScriptCore::AddCommand(const QString &commandName, BinderFn binderFn)
    {

    }

}  // namespace CallScp
