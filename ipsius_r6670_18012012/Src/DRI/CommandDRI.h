#ifndef __COMMANDDRI__
#define __COMMANDDRI__

#include "Utils/IBasicInterface.h"
#include "Utils/StringList.h"
#include "Utils/SafeRef.h"

#include "ISessionCmdContext.h"
#include "DRIParserConsts.h"
#include "ParsedDRICmd.h"
#include "MetaInvoke.h"
#include "commanddriresult.h"

namespace Domain
{
    class NamedObject;

}  // namespace Domain



namespace DRI
{
    using boost::shared_ptr;

    class TransactionDRI;
    class CommandDRI;
    class ISessionCmdContext;
    class CommandAsyncDone;

    // ---------------------------------------------------------------

    // cmd -> session
    class ICommandDriOwner : public Utils::IBasicInterface
    {
    public:
        virtual ISessionCmdContext* GetSessionCmdContext() = 0;
        virtual void CommandRunAsyncCallback(shared_ptr<CommandAsyncDone>) = 0;
        virtual void OnCommandStarted(CommandDRI &cmd) = 0;
        virtual void OnCommandOutput(const QString &data, bool lineFeed) = 0;
        virtual void OnCommandCompleted(CommandDRI &cmd) = 0;
    };

    // ---------------------------------------------------------------
    
    // комманда DRI
    class CommandDRI : 
        public virtual Utils::SafeRefServer,
        public IAsyncCmd, 
        boost::noncopyable
    {
        
    public:

        enum CommandType
        {
            ctUser,     // command created by user (session owner)
            ctService,  // command created by session (CS_Begin/CS_End)
            ctScript,   // command loaded from script (CS_Exec)
        };

        CommandDRI(ICommandDriOwner &owner, shared_ptr<ParsedDRICmd> cmd, 
                   CommandDRI::CommandType type);

        CommandDRI::CommandType Type() const { return m_type; }
        bool IsService() const { return m_type == ctService; }
        bool IsScripted() const { return m_type == ctScript; }

        // CS_xxx command
        bool IsSessionControl() const
        {            
            QString name = m_cmd->getMethodOrPropertyName();
            if (name.size() < CCSPrefix.size()) return false;

            return (name.left( CCSPrefix.size() ) == CCSPrefix);
        }

        bool IsTransactionBegin() const
        {
            return (m_cmd->getMethodOrPropertyName() == CCS_Begin);
        }

        bool IsTransactionEnd() const
        {
            return (m_cmd->getMethodOrPropertyName() == CCS_End);
        }

        bool IsTransactionDiscard() const
        {
            return (m_cmd->getMethodOrPropertyName() == CCS_Discard);
        }

        bool IsPrint() const
        {
            return (m_cmd->getMethodOrPropertyName() == CCS_Print);
        }

        shared_ptr<ParsedDRICmd> GetParsedCmd() { return m_cmd; }

        shared_ptr<CommandDriResult> Result() { return m_result; }

        void Completed(CommandDriResult *pError, bool withNotify);

        bool Completed() const { return (m_result != 0); }
       
        void Invoke(shared_ptr<MetaInvoke> invoke);

        void PropertyRead(Domain::NamedObject *pObj, QString propertyName);

        void PropertyWrite(Domain::NamedObject *pObj, QString propertyName, QString val);

        bool TryComplete(bool haltOnFail);
        
        QString CommandToString(QString prefix = ">") const
        {
            QString s;

            s += prefix;
            if (!s.isEmpty()) s += " ";
            // s += m_cmd->OriginalCmd();
            s += m_cmd->ProcessedCmd();

            return s;
        }

        static const QString& ServiceLinePrefix() 
        { 
            static QString prefix(":");
            return prefix; 
        }

        QString ResultToString(bool withOutput, bool extErrorInfo = true) const;
        QString OutputToString() const;

        int DurationMin() const
        {
            if (m_startDt.isNull()) return -1;
            return m_startDt.secsTo(QDateTime::currentDateTime()) / 60;
        }
        
    // IAsyncCmd impl
    private:
        void Add(const QString &data, bool lineFeed);
        DriSec::ISecurity* GetSecurity();
        void Flush();
        void AsyncStarted(Utils::SafeRef<Domain::NamedObject> asyncObject);
        void AsyncCompleted(bool isOK, const QString &errorMsg = "");
        
    private:

        struct OutputData
        {
            QString Str;
            bool LineFeed;

            OutputData(const QString &s, bool lineFeed) :
                Str(s), LineFeed(lineFeed)
            {
            }
        };
        
        ICommandDriOwner &m_owner;
        shared_ptr<ParsedDRICmd> m_cmd;
        CommandDRI::CommandType m_type;
        
        // this fields describes object state: init -> invoke -> completed
        shared_ptr<MetaInvoke> m_invoke;  // must be "global" for store args for async commands
        shared_ptr<CommandDriResult> m_result;
        std::vector<OutputData> m_outputBuffer;
        bool m_debug;

        Utils::SafeRef<Domain::NamedObject> m_asyncObject;
        Utils::AtomicBool m_asyncCompleted;

        QDateTime m_startDt;
        
        friend class CommandAsyncDone; 
        Utils::StringList m_output;
        
        void ProcessSpecialFirstParam(bool &cmdIsAsync);

        template<class TResult>
        void CompleteWithException(const std::exception &e,  const QString &addInfo = "")
        {
            Completed(new TResult(e, addInfo), true);
        }

        void FlushOutput(bool last = false);
        void AsyncException(const std::exception &e);
        void Started();
    };    

    // -------------------------------------------------------------

    /*
        Класс для развезки асинхронного завершения команды через сообщения.
        CommandDRI не является MsgObject, поэтому развязка идет через сессию 
    */
    class CommandAsyncDone : boost::noncopyable
    {
        CommandDRI &m_cmd; 
        bool m_isOK; 
        QString m_errMsg;

    public:

        CommandAsyncDone(CommandDRI &cmd, bool isOK, const QString &errMsg) :
          m_cmd(cmd), m_isOK(isOK), m_errMsg(errMsg)
        {
        }

        void Exec()
        {
            if (m_isOK)
            {
                m_cmd.Completed(new CommandDriResultOk(m_cmd.m_output), true);
            }
            else
            {
                m_cmd.Completed(new CommandDriRuntimeAsyncError(m_errMsg), true);
            }
        }

    };
    
}  // namespace DRI

#endif
