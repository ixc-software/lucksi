#ifndef __SESSIONDRI__
#define __SESSIONDRI__

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"
#include "iCore/MsgObject.h"
#include "iLog/LogWrapper.h"
#include "iLog/iLogSessionCreator.h"
#include "iLog/LogManager.h"
#include "Domain/IDomain.h"
#include "Domain/DomainClass.h"
#include "Domain/NObjRoot.h"
// #include "Domain/NamedObject.h"

#include "MetaInvoke.h"
#include "ExecutiveDRI.h"
#include "ISessionCmdContext.h"
#include "DriArgFactory.h"
#include "drifileload.h"

namespace Domain
{
    class NamedObject;

} 

namespace DriSec
{
    class ISecurity;
}

namespace DRI
{
    using boost::shared_ptr;
    using boost::scoped_ptr;
    using iLogW::EndRecord;
    using iLogCommon::LogString;

    class ISessionDriOwner : public Utils::IBasicInterface
    {
    public:
        virtual void CommandQueueEmpty() = 0;
        virtual void TransactionBeginExec() = 0;
        virtual void TransactionEndExec() = 0;
        virtual void SessionExitRequest() = 0;
        virtual void CommandStarted(shared_ptr<CommandDRI> cmd) = 0;
        virtual void CommandOutput(CommandDRI::CommandType type, QString data, bool lineFeed) = 0;
        virtual void CommandCompleted(shared_ptr<CommandDRI> cmd) = 0;

        virtual QString GetFullSessionInfo() const = 0;             
    };

    // ----------------------------------------------------------
    
    // DRI сессия. Независимый источник DRI транзакций. Система запуска DRI комманд. 
    class SessionDRI : 
        public iCore::MsgObject,
        public ITransactionSource,
        public ICommandDriOwner,
        public ISessionCmdContext,
        public ITransToSession,
        boost::noncopyable
    {

        typedef std::vector< shared_ptr<CommandDRI> > CmdList;

        Domain::IDomain &m_domain;
        ISessionDriOwner &m_owner;

        scoped_ptr<iLogW::LogSession> m_log;
        iLogW::LogRecordTag m_tagInfo, m_tagTrace;

        CmdList m_cmds;
        std::queue< shared_ptr<TransactionDRI> > m_transactions;

        shared_ptr<TransactionDRI> m_currTransaction;

        // empty for root; not Domain::NamedObject* 'couse .Delete destroy this field
        QString m_currObjectName;  
        std::stack<QString> m_currObjHistory;

        bool m_exitRequested;
        
        Utils::SafeRef<DriSec::ISecurity> m_security;

        bool m_debug;

        template<class TResult>
        void CmdCompleteAtTransaction(shared_ptr<CommandDRI> cmd, QString msg)
        {
            CommandDriResult *pError = new TResult(msg);

            // manual notify 'couse this command is not in transaction
            cmd->Completed(pError, false);

            NotifyOwnerCommandCompleted(cmd);
        }

        bool TransactionFromFirstCommand();

        // send "discarded" + "OK"
        void DiscardCommands(CmdList::iterator iDiscard)
        {
            // [0 .. iDiscard) -> discarded
            CmdList::iterator i = m_cmds.begin();

            while(i != iDiscard)
            {
                CmdCompleteAtTransaction<CommandDriDiscarded>(*i, "Discarded!");
                ++i;
            }

            // discard -- ok
            CmdCompleteAtTransaction<CommandDriResultOk>(*i, "");

            // erase
            m_cmds.erase(m_cmds.begin(), iDiscard + 1);
        }

        // find full transaction in m_cmds
        // single command (without CS_Begin/CS_End) treated as transaction
        // return transaction size, 0 -- for call again, -1 -- can't create transaction
        int FindTransaction();

        shared_ptr<CommandDRI> CreateCmdFromString(const QString &cmd)
        {
            shared_ptr<ParsedDRICmd> parsedCmd( new ParsedDRICmd(cmd) );

            return shared_ptr<CommandDRI>( 
                    new CommandDRI(*this, parsedCmd, CommandDRI::ctService) 
                );
        }

        void CreateTransaction(int transSize);

        // find full transaction in m_cmds and move it to m_transactions
        bool CommandsToTransations();

        DRI::ExecutiveDRI& ExeDRI()
        {
            return m_domain.getDomain().ExeDRI();
        }

        QString ObjectForCmdName(shared_ptr<ParsedDRICmd> cmd)
        {
            QString path = cmd->FullObjName();

            QString name = path;  // for absolute path

            if (cmd->IsRelativePath())
            {
                name = m_currObjectName;
                if (!path.isEmpty()) 
                {
                    if (!name.isEmpty()) name += CPathSep;
                    name += path;
                }
            }

            return name;
        }

        Domain::NamedObject* ObjectForCmd(QString name);

        bool FindMethod(QObject *pObj, QString searchName, QMetaMethod &method);

        bool FindProperty(QObject *pObj, QString propertyName, QMetaProperty &metaProperty);

        void DoTransactionCompleted();

        void DriPrepareError(shared_ptr<CommandDRI> cmd, QString msg)
        {
            // notify owner that command was processed
            m_owner.CommandStarted(cmd);
            
            CommandDriResult *pError = new CommandDriPrepareError(msg);
            cmd->Completed(pError, true);
        }

        void AbortedCommand(shared_ptr<CommandDRI> cmd)
        {
            CommandDriResult *pError = new CommandDriAborted();

            // manual notify 'couse transaction already delete this command
            cmd->Completed(pError, false);  

            NotifyOwnerCommandCompleted(cmd);
        }

        void ExecuteMethod(shared_ptr<CommandDRI> cmdDri, 
            Domain::NamedObject *pObj, const QMetaMethod &method);
        
        void PropertyError(shared_ptr<CommandDRI> cmdDri, QString actionName)
        {
            DriPrepareError(cmdDri, QString("Bad property access '%1'").arg(actionName));
        }

        void ExecuteProperty(shared_ptr<CommandDRI> cmdDri, 
            Domain::NamedObject *pObj, const QMetaProperty &metaProperty);


        void LogExecuteCommandState(shared_ptr<CommandDRI> cmdDri);

        void DoExecuteCommand(shared_ptr<CommandDRI> cmdDri);

        void NotifyOwnerCommandCompleted(shared_ptr<CommandDRI> cmd);

        void DoCommandRunAsyncCallback(shared_ptr<CommandAsyncDone> async)
        {
            async->Exec();
        }

        void Log(const QString &data);

        
    // ICommandDriOwner impl
    private:

        ISessionCmdContext* GetSessionCmdContext()
        {
            return this;
        }

        void OnCommandStarted(CommandDRI &cmd)
        {
            ESS_ASSERT(m_currTransaction != 0);
            shared_ptr<CommandDRI> cmdPtr = m_currTransaction->StartedCommand();
            ESS_ASSERT(cmdPtr.get() == &cmd);

            // save OnCompleted
            // if (type != CommandDRIDetail::ctService) Log(cmd->ProcessedCmd());
            m_owner.CommandStarted(cmdPtr);// type, cmd);
        }
        
        void OnCommandOutput(const QString &data, bool lineFeed)
        {
            ESS_ASSERT(m_currTransaction != 0);
            CommandDRI::CommandType type = m_currTransaction->StartedCommand()->Type();
            // save OnCompleted
            // if (type != CommandDRIDetail::ctService) Log(data, lineFeed);
            m_owner.CommandOutput(type, data, lineFeed);
        }

        void OnCommandCompleted(CommandDRI &completedCmd)
        {
            ESS_ASSERT(m_currTransaction != 0);
            shared_ptr<CommandDRI> cmd = m_currTransaction->CommandCompleted(m_exitRequested);
            ESS_ASSERT(cmd.get() == &completedCmd);

            // notify
            NotifyOwnerCommandCompleted(cmd);
        }

        void CommandRunAsyncCallback(shared_ptr<CommandAsyncDone> async)
        {
            PutMsg(this, &SessionDRI::DoCommandRunAsyncCallback, async);
        }

    // ITransToSession impl
    private:

        void TransactionCompleted()
        {
            PutMsg(this, &SessionDRI::DoTransactionCompleted);
        }

        void ExecuteCommand(shared_ptr<CommandDRI> cmdDri)
        {
            PutMsg(this, &SessionDRI::DoExecuteCommand, cmdDri);
        }

        void TransactionAbortedCommand(shared_ptr<CommandDRI> cmd)
        {
            PutMsg(this, &SessionDRI::AbortedCommand, cmd);
        }

    // ISessionCmdContext impl
    private:

        void SetCurrentObject(QString absoluteName)
        {
            if (m_domain.getDomain().FindFromRoot(absoluteName) == 0)
            {
                std::ostringstream ss;
                ss << "Can't change current object to " 
                   << absoluteName.toStdString();

                ESS_THROW_MSG(DRI::ExecuteException, ss.str());
            }

            
            m_currObjHistory.push(m_currObjectName);
            m_currObjectName = absoluteName;
        }

        void SetCurrentObjectBack()
        {
            if (m_currObjHistory.empty())
            {
                ESS_THROW_MSG(DRI::ExecuteException, "Current object names stack is empty!");
            }

            m_currObjectName = m_currObjHistory.top();
            m_currObjHistory.pop();
        }


        void TransactionBegin(bool continueOnError)
        {
            ESS_ASSERT(m_currTransaction != 0);
            m_currTransaction->ContinueOnError(continueOnError);
        }

        void TransactionEnd() 
        {
            // nothing
        }

        void TransactionDiscard() 
        {
            ESS_THROW_MSG(DRI::ExecuteException, "Never executed!");
        }

        void SetCommandTimeout(int intervalMs)
        {
            ESS_THROW_MSG(DRI::ExecuteException, "Unimplemented");
        }

        void ExecuteFile(QString fileName, QString macroReplaceList);

        void OutputPrint(QString msg)
        {
            ESS_ASSERT(m_currTransaction != 0);
            m_currTransaction->OutputToCurrentCommand(msg);
        }

        void SessionExitRequest()
        {
            m_exitRequested = true;            
        }

        MetaObjectsInfo& MetaObjInfo()
        {
            return m_domain.getDomain().ExeDRI().MetaObjInfo();
        }

        void PrintSessionInfo()
        {
            QString info; // ("Current session info: ");
            info += m_owner.GetFullSessionInfo();
            OutputPrint(info);
        }

        void SwitchSecProfile(QString profileName); 

        DriSec::ISecurity* GetSecurity()
        {
            return m_security.operator->();
        }


    // ITransactionSource impl
    private:

        TransactionDRI* PeekTransaction()
        {
            ESS_ASSERT(m_currTransaction == 0);

            if (m_transactions.size() == 0) return 0;

            m_owner.TransactionBeginExec();

            m_currTransaction = m_transactions.front();
            m_transactions.pop();

            return m_currTransaction.get();
        }

        void NotifyTreeChanged()
        {
            QString name = m_currObjectName;

            while(!name.isEmpty())
            {
                if (m_domain.getDomain().FindFromRoot(name) != 0) break;

                name = Domain::ObjectName(name).Parent();
            }

            m_currObjectName = name;
        }

        
    public:

        SessionDRI(Domain::IDomain &domain, 
            ISessionDriOwner &owner, 
			iLogW::ILogSessionCreator &logCreator,
            int id,
            Utils::SafeRef<DriSec::ISecurity> security) : 
            iCore::MsgObject(domain.getDomain().getMsgThread()),
            m_domain(domain),
            m_owner(owner),
            m_exitRequested(false),
            m_security(security)
        {
            ESS_ASSERT( !m_security.IsEmpty() );

            m_debug = false;
            
            // log
            {
                std::string name = "SessionDRI_";
                name += Utils::IntToString(id);
                m_log.reset( logCreator.CreateSession(name, true) );

                m_tagInfo = m_log->RegisterRecordKindStr("Info");
                m_tagTrace = m_log->RegisterRecordKindStr("Trace");

                Log("Started!");
            }

            // register in ExeDRI as transactions source
            ExeDRI().RegisterSource(this);
        }

        ~SessionDRI();

        // return true, if transaction created
        bool AddCommand(shared_ptr<ParsedDRICmd> cmd) 
        {
            shared_ptr<CommandDRI> cmdDri( 
                new CommandDRI(*this, cmd, CommandDRI::ctUser) );
            m_cmds.push_back(cmdDri);

            return CommandsToTransations();
        }

        void DropAllCommands();

        bool CmdQueueEmpty() const
        {
            return m_cmds.empty();
        }

        bool HasTransactions()
        {
            return (m_currTransaction != 0) || (m_transactions.size() > 0);
        }

        bool Active() const { return (m_currTransaction != 0); }

        const QString& CurrentObject() const { return m_currObjectName; }

        QString CurrTransactionInfo() const
        {
            return (m_currTransaction != 0) ? m_currTransaction->Info() : "";
        }

        void TryFinalize(bool haltOnFail)
        {
            if (m_currTransaction != 0)
            {
                m_currTransaction->TryFinalize(haltOnFail);
            }
        }

        void ExitRequest()
        {
            m_exitRequested = true;
        }

    };

    
}  // namespace DRI


#endif
