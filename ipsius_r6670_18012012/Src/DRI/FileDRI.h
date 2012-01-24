#ifndef __FILEDRI__
#define __FILEDRI__

#include "Utils/SafeRef.h"
#include "Utils/IBasicInterface.h"
#include "Utils/StringList.h"
#include "Utils/WeakRef.h"
#include "Domain/IDomain.h"
#include "DriSec/SecUtils.h"
#include "iLog/LogWrapper.h"
#include "iCore/MsgObject.h"

#include "SessionDRI.h"
#include "DRIParserConsts.h"
#include "DRICommandStreamParser.h"
#include "drifileload.h"
#include "IDriFileExecuteDone.h"


namespace DRI
{
    using boost::scoped_ptr;
    using boost::shared_ptr;
    using Utils::SafeRef;
    using iLogCommon::LogString;

    enum FileExecutionResultCode
    {
        ferOK,
        ferAbortByError,
        ferAbortRequested,
    };

    class FileExecuteResult : boost::noncopyable
    {
        typedef std::vector< shared_ptr<CommandDRI> > CmdList;

        CmdList m_list;
        bool m_incompletedTransaction;
        FileExecutionResultCode m_result;
        QString m_resultDesc;

        void DebugResult(DRI::CommandDriResult &result)
        {
            Utils::StringList sl = result.Output();

            if (sl.size() == 0) return;

            QString s = sl.at(0);
            if (s.indexOf("ROOT : RootNamedObject") >= 0)
            {
                char c = s.at(s.size() - 1).toAscii();
                int i = 0;

                s = result.OutputAsString();
                i++;
            }
        }

    public:

        FileExecuteResult()
        {
            m_incompletedTransaction = false;
            m_result = ferOK;
        }

        void Add(shared_ptr<CommandDRI> cmd)
        {
            m_list.push_back(cmd);
        }

        void SetIncompletedTransaction()
        {
            m_incompletedTransaction = true;
        }

        void SetCompleteCode(FileExecutionResultCode code, const QString &desc)
        {
            m_result = code;
            m_resultDesc = desc;
        }
        
        FileExecutionResultCode CompleteCode() const 
        {
            return m_result;
        }

        const QString& CompleteDesc() const
        {
            return m_resultDesc;
        }

        QString ToString(bool justShortStatistic = false)
        {
            QString res;
            
            int totalCmd = 0;
            int scriptCmd = 0;
            int serviceCmd = 0;
            int completedWithErrs = 0;

            for(size_t i = 0; i < m_list.size(); ++i)
            {
                shared_ptr<CommandDRI> cmd = m_list.at(i);
                bool cmdOK = cmd->Result()->OK();

                if (cmd->IsService()) 
                {
                    serviceCmd++;
                    continue;
                }

                // add to result
                QString s;
                if (cmd->IsPrint() && cmdOK)
                {
                    QString res(cmd->Result()->OutputAsString());
                    if (!res.startsWith(CStringChar) && !res.endsWith(CStringChar))
                    {
                        res = QString("%1%2%1").arg(CStringChar).arg(res); 
                    }
                    s +=  QString("%1%2 %3\n\n").arg(DRI::CommandDRI::ServiceLinePrefix())
                                                 .arg("PRINT").arg(res);
                }
                else
                {
                    s += cmd->CommandToString() + "\n";
                    bool withOutput = true;
                    s += cmd->ResultToString(withOutput) + "\n\n";

                    // debug
                    DebugResult( *(cmd->Result()) );
                }

                if (!justShortStatistic) res += s;

                // stats
                totalCmd++;
                if (!cmdOK) completedWithErrs++;
                if (cmd->IsScripted()) scriptCmd++;
            }
            
            if (!justShortStatistic) res += "\n";
            
            // stats
            res += QString("Commands in total %1, with errors %2, service %3, scripted %4")
                            .arg(totalCmd).arg(completedWithErrs)
                            .arg(serviceCmd).arg(scriptCmd);

            if (justShortStatistic) return res;
            
            if (m_incompletedTransaction) res += "\nIncomplete transaction";
            if (m_result == ferAbortByError) res += "\nAborted by first error";
            if (m_result == ferAbortRequested) res += "\nAborted by request";

            return res;
        }

    };

    // --------------------------------------------------------------

    // Using by autorun script runner
    class IFileDRICoutTracer : public Utils::IBasicInterface
    {
    public:
        virtual void AddToOutput(const QString &data, bool lineFeed) = 0;
    
    };

    // --------------------------------------------------------------
    
    class FileDRI : 
        public iCore::MsgObject,
        public ISessionDriOwner, 
        boost::noncopyable
    {
        Domain::IDomain &m_domain;
		// debug log
		scoped_ptr<iLogW::LogSession> m_log;
		iLogW::LogRecordTag m_tagInfo;

        SafeRef<IFileExecuteDone> m_resultCallback;
        bool m_stopOnFirstError;
        int  m_commandInTransaction; // commands in incompleted transaction
        bool m_aborted;

        scoped_ptr<DRIFileLoad> m_file;
        shared_ptr<FileExecuteResult> m_result;

        SessionDRI *m_pSessionDri;  // ExecutiveDRI have ownership for this object 
        Utils::WeakRefHost m_host;

        QString m_fileName;
        int m_sessionID;
        
        // debug cout
        bool m_debug;

        IFileDRICoutTracer *m_pCoutTracer;
        
        void InitLog()
        {
            bool enableFileDRITrace = false;
                        
            LogString name = QString("FileDRI%1").arg(m_sessionID).toStdWString();
            m_log.reset(m_domain.getDomain().LogCreator()-> 
                CreateSessionExt(name, enableFileDRITrace) );
            m_tagInfo = m_log->RegisterRecordKindStr("Info");
        }

        void ExecutionDone(FileExecutionResultCode resultCode, const QString &resultDesc)
        {
            m_result->SetCompleteCode(resultCode, resultDesc);

            if (m_commandInTransaction > 0)
            {
                using namespace std;
                if (m_debug) cout << "Incompeted transaction(s) in file" << endl;
                m_result->SetIncompletedTransaction();
            }

            m_pSessionDri = 0;
            m_host.Clear();

            m_log.reset();
            m_file.reset();

            // fix sending callback twice
            if ((resultCode == ferAbortByError) && m_aborted) return;
            
            DebugMsg("OnFileExecuteDone");
            m_resultCallback->OnFileExecuteDone(m_result);
        }

        void OnExecutionAbort()
        {
            ExecutionDone(ferAbortRequested, "Abort requested");
        }

        void Log(const QString &data, const QString &prefix)
        {
            if (!m_log->LogActive(m_tagInfo)) return;

            *m_log << m_tagInfo << prefix << data << EndRecord;
        }

        void LogToCout(QString data, bool lineFeed)
        {
            if (m_pCoutTracer == 0) return;

            m_pCoutTracer->AddToOutput(data, lineFeed);
        }
        
        // cmd + output
        void LogCompleteCmd(shared_ptr<CommandDRI> cmd)
        {
            if (cmd->IsService()) return; 

            // name
            Log(cmd->CommandToString(), "");

            // result + output
            const bool resWithOutput = true;
            QString result = cmd->ResultToString(resWithOutput);
            Log(result, "< ");
            // autorun cout (don't show ":OK" results)
            if (!cmd->Result()->OK()) LogToCout(result, true);
        }
        
        void NextCommand()
        {
            if (m_aborted) return;

            if (m_file == 0) return;  // execution done
            if (m_pSessionDri == 0) return;

            // don't need to add commadns
            if (m_pSessionDri->HasTransactions())
            {
                return;
            }

            // peek command
            shared_ptr<ParsedDRICmd> cmd = m_file->Peek();

            // EOF
            if (cmd == 0)
            {
                // if file has no commands
                if (m_result == 0) m_result.reset(new FileExecuteResult());
                ExecutionDone(ferOK, "OK");

                return;
            }

            DebugMsg("Add cmd " + cmd->OriginalCmd());

            // put command in session
            if (m_pSessionDri->AddCommand(cmd))
            {
                m_commandInTransaction = 0;
                // wait for TransactionEndExec() event
            }
            else
            {
                m_commandInTransaction++;
                RequestNextCommand();
            }

        }

        void RequestNextCommand()
        {
            PutMsg(this, &FileDRI::NextCommand);
        }

        bool StopOnCommand(shared_ptr<CommandDRI> cmd)
        {
            return !cmd->Result()->OK() && m_stopOnFirstError;
        }

        void ProcessCompletedCommand(shared_ptr<CommandDRI> cmd)
        {            
            // log
            LogCompleteCmd(cmd);

            // process cmd
            if (m_result == 0) m_result.reset( new FileExecuteResult() );
            m_result->Add(cmd);

            if ( StopOnCommand(cmd) )
            {
                ExecutionDone( ferAbortByError, cmd->ResultToString(true) );
            }
        }

        void DebugMsg(QString msg)
        {
            if (!m_debug) return;

            std::cout << "[FileDRI] " << msg.toStdString() << std::endl;
        }

    // ISessionDriOwner impl
    private:

        void CommandCompleted(shared_ptr<CommandDRI> cmd)
        {
            PutMsg(this, &FileDRI::ProcessCompletedCommand, cmd);
            
            if ( StopOnCommand(cmd) )
            {
                // debug info
                bool cmdOk = cmd->Result()->OK();
                QString cmdName = cmd->GetParsedCmd()->getMethodOrPropertyName();

                ESS_ASSERT(m_pSessionDri != 0);
                m_pSessionDri->ExitRequest();
                // m_pSessionDri->DropAllCommands();
            } 
        }

        void CommandQueueEmpty()
        {
        }

        void TransactionBeginExec()
        {
        }

        void TransactionEndExec()
        {
            RequestNextCommand();
        }

        void SessionExitRequest()
        {
            m_aborted = true;

            // ExecutionDone(true);
            PutMsg(this, &FileDRI::OnExecutionAbort);
        }

        void CommandStarted(shared_ptr<CommandDRI> cmd)
        {
            if (cmd->IsService()) return;

            // Log(cmd->CommandToString(), true); // save OnComplete
            LogToCout(cmd->CommandToString(), true);
        }

        void CommandOutput(CommandDRI::CommandType type, QString data, bool lineFeed)
        {
            if (type == CommandDRI::ctService) return;
            
            // Log(data, lineFeed); // save OnComplete
            LogToCout(data, lineFeed);
        }

        QString GetFullSessionInfo() const
        {
            return "File " + m_fileName;
        }
        
    public:

        // enableCoutTrace - true, if autorun script
        FileDRI(Domain::IDomain &domain, 
                IFileDRICoutTracer *pCoutTrace = 0) :
            iCore::MsgObject(domain.getDomain().getMsgThread()),
            m_domain(domain), 
            m_pSessionDri(0),
            m_pCoutTracer(pCoutTrace)
        {
            m_debug = false;  // debug
            m_stopOnFirstError = false;
            m_aborted = false;

            m_sessionID = m_domain.getDomain().ExeDRI().AllocSessionNumber();
        }

        ~FileDRI()
        {
            if (m_pSessionDri != 0) 
            {
                m_pSessionDri->TryFinalize(true);
            }
        }

        void LoadFile(QString fileName)  // can throw -- file i/o or parser error
        {
            m_fileName = fileName;
                
            m_file.reset( new DRIFileLoad(fileName, 
                                          m_domain.getDomain().Params().IncludeDirList(),
                                          m_domain.getDomain().Params().SpecialParams()) );
            InitLog();
        }

        void Execute(SafeRef<IFileExecuteDone> resultCallback, bool stopOnFirstError)  
        {
            ESS_ASSERT(m_file != 0);

            m_resultCallback = resultCallback;
            m_stopOnFirstError = stopOnFirstError;

            // execute
            ExecutiveDRI &exe = m_domain.getDomain().ExeDRI();

            m_pSessionDri = &exe.GetNewSession(DriSec::Const::RootLogin(),                
                m_host.Create<ISessionDriOwner&>(*this), 
                m_sessionID);
            
            // feed commands
            m_commandInTransaction = 0;
            RequestNextCommand();
            // m_file->AddCommandsToSession(*m_sessionDri);
        }
        
    };
    
}  // namespace DRI

#endif
