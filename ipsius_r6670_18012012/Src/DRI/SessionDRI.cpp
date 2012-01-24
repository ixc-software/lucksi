#include "stdafx.h"

#include "DriSec/SecUtils.h"
#include "DriSec/NObjSecRoot.h"

#include "SessionDRI.h"

// --------------------------------------------------

namespace DRI
{            
    using DriSec::ISecurity;

    bool SessionDRI::TransactionFromFirstCommand()
    {
        // single command
        while (m_cmds.size() != 0)
        {
            // CS_Begin -- multicommand transaction
            if (m_cmds.at(0)->IsTransactionBegin()) break;

            // CS_End without CS_Begin
            if (m_cmds.at(0)->IsTransactionEnd())
            {
                CmdCompleteAtTransaction<CommandDriTransactionError>
                    (m_cmds.at(0), "CS_End without CS_Begin");
                m_cmds.erase( m_cmds.begin() );
                continue;
            }

            // CS_Discard outside transaction
            if (m_cmds.at(0)->IsTransactionDiscard())
            {
                CmdCompleteAtTransaction<CommandDriTransactionError>
                    (m_cmds.at(0), "CS_Discard without CS_Begin");
                m_cmds.erase( m_cmds.begin() );
                continue;
            }

            return true; // single command
        }

        return false;
    }

    // -------------------------------------------------------------

    int SessionDRI::FindTransaction()
    {
        if (TransactionFromFirstCommand()) return 1;

        if (m_cmds.size() < 2) return -1;  // not enough commands

        // m_cmd[0] is CS_Begin
        // try to find transaction end
        CmdList::iterator i = m_cmds.begin() + 1;

        while(i != m_cmds.end())
        {
            // inner CS_Begin
            if ( (*i)->IsTransactionBegin() ) 
            {
                CmdCompleteAtTransaction<CommandDriTransactionError>
                    ((*i), "Inner CS_Begin");
                i = m_cmds.erase(i);
                continue;
            }

            // CS_Discard
            if ( (*i)->IsTransactionDiscard() )
            {
                int paramsSize = (*i)->GetParsedCmd()->getParamsSize();
                if (paramsSize != 0)
                {
                    CmdCompleteAtTransaction<CommandDriTransactionError>
                        ((*i), "Call CS_Discard with params!");
                    i = m_cmds.erase(i);
                    continue;
                }

                DiscardCommands(i);
                return 0;  // transaction canceled, call again
            }

            // CS_End
            if ( (*i)->IsTransactionEnd() )
            {
                return (i - m_cmds.begin()) + 1;
            }

            ++i;
        }

        return -1;  // CS_End not found
    }

    // -------------------------------------------------------------

    void SessionDRI::CreateTransaction( int transSize )
    {
        // create transaction
        shared_ptr<TransactionDRI> trans( new TransactionDRI(m_domain, *this) );

        // add CS_Begin for single command
        if (transSize == 1) trans->Add( CreateCmdFromString(CCS_Begin) );

        // move transSize commands to new transaction
        {
            int count = 0;
            CmdList::iterator i = m_cmds.begin();

            while(i != m_cmds.end())
            {
                trans->Add( *i );
                i = m_cmds.erase(i);

                if (++count == transSize) break;
            }

            ESS_ASSERT(count == transSize);
        }

        // add CS_End for single command
        if (transSize == 1) trans->Add( CreateCmdFromString(CCS_End) );

        // add new transaction to list
        m_transactions.push(trans);
    }

    // -------------------------------------------------------------

    void SessionDRI::ExecuteProperty( shared_ptr<CommandDRI> cmdDri, 
                                      Domain::NamedObject *pObj, 
                                      const QMetaProperty &metaProperty )
    {
        QString actionName = cmdDri->GetParsedCmd()->getMethodOrPropertyName();

        ParsedDRICmd::ActionType aType = cmdDri->GetParsedCmd()->getActionType();
        const QStringList &params = cmdDri->GetParsedCmd()->getParams();

        if ((params.size() > 1) || (aType == ParsedDRICmd::MethodCall))
        {
            PropertyError(cmdDri, actionName);
            return;
        }
        
        // operation
        bool propRead = (params.size() == 0);

        if ( (propRead) && (aType != ParsedDRICmd::Unknown) )
        {
            PropertyError(cmdDri, actionName);
            return;
        }

        // debug log
        if (m_debug) 
        {
            QString op = propRead ? "READ" : "WRITE";
            Log(QString("%1 %2").arg(op).arg(actionName));
        }
        
        // security
        {
            ISecurity::ActionKind kind = propRead ? ISecurity::akPropRead : ISecurity::akPropWrite;
            QString info;
            if ( !m_security->VerifyAction(pObj, actionName, kind, info) )
            {
                DriPrepareError(cmdDri, info);
                return;
            }
        }

        if (propRead)  
        {            
            cmdDri->PropertyRead(pObj, actionName);
        }
        else 
        {
            cmdDri->PropertyWrite(pObj, actionName, params.at(0));
        }
    }

    // -------------------------------------------------------------

    void SessionDRI::ExecuteMethod( shared_ptr<CommandDRI> cmdDri, 
        Domain::NamedObject *pObj, const QMetaMethod &method )
    {
        QString actionName = cmdDri->GetParsedCmd()->getMethodOrPropertyName();

        if (QString(method.typeName()) != "") 
        {
            DriPrepareError(cmdDri, QString("Meta-function '%1' must return void!").arg(actionName));
            return;
        }

        QString info;
        if ( !m_security->VerifyAction(pObj, actionName, ISecurity::akExecute, info) )
        {
            DriPrepareError(cmdDri, info);
            return;
        }

        // debug log
        if ((m_debug) && (!cmdDri->IsService())) Log(actionName); 
        
        // create invoke and execute
        shared_ptr<MetaInvoke> invoke( new MetaInvoke(ExeDRI().ArgFactory(), pObj, method) );

        cmdDri->Invoke(invoke);
    }


    // -------------------------------------------------------------

    void SessionDRI::LogExecuteCommandState( shared_ptr<CommandDRI> cmdDri )
    {
        if (!m_debug) return;
        if (!m_log->LogActive(m_tagInfo)) return;

        QString info;

        info += cmdDri->CommandToString();

        // history
        const int CMaxHistoryCommandsInfo = 4;

        info += " // cmds queue // ";

        for(int i = 0; i < CMaxHistoryCommandsInfo; ++i)
        {
            if (i >= m_cmds.size()) break;

            info += m_cmds.at(i)->CommandToString();
            info += " / ";
        }

        Log(info);
    }

    // -------------------------------------------------------------

    void SessionDRI::DoExecuteCommand( shared_ptr<CommandDRI> cmdDri )
    {
        ESS_ASSERT(cmdDri != 0);
        shared_ptr<ParsedDRICmd> cmd = cmdDri->GetParsedCmd();

        // LogExecuteCommandState(cmdDri);  // log

        // find NamedObject for command
        QString debugPathInfo = QString("Full name %1, relative %2, curr %3")
            .arg(cmd->FullObjName()).arg(cmd->IsRelativePath()).arg(m_currObjectName);
        QString objName = ObjectForCmdName(cmd);
        Domain::NamedObject *pObj = ObjectForCmd(objName); 
        if (pObj == 0) 
        {
            DriPrepareError(cmdDri, QString("Object '%1' not found!").arg(objName));
            return;
        }

        // is it method
        QMetaMethod metaMethod; 
        QString actionName = cmd->getMethodOrPropertyName();
        if ( FindMethod(pObj, actionName, metaMethod) )
        {
            ExecuteMethod(cmdDri, pObj, metaMethod);
            return;
        }

        // is it property
        QMetaProperty metaProperty;
        if (FindProperty(pObj, actionName, metaProperty))
        {
            ExecuteProperty(cmdDri, pObj, metaProperty);
            return;
        }

        // error
        DriPrepareError(cmdDri, QString("Method/property '%1' not found").arg(actionName));
    }

    // -------------------------------------------------------------

    void SessionDRI::ExecuteFile( QString fileName, QString macroReplaceList )
    {
        if (fileName.size() == 0) 
        {
            ESS_THROW_MSG(DRI::ExecuteException, "File name is empty!");
        }

        DRIFileLoad file(fileName, 
                         m_domain.getDomain().Params().IncludeDirList(),
                         m_domain.getDomain().Params().SpecialParams(),
                         macroReplaceList);

        CmdList commands;

        while(true)
        {
            shared_ptr<ParsedDRICmd> cmd = file.Peek();
            if (cmd == 0) break;

            // insert new command
            shared_ptr<CommandDRI> cmdDri( 
                new CommandDRI(*this, cmd, CommandDRI::ctScript) );
            commands.push_back(cmdDri);

            // AddCommand(cmd);
        }

        // insert commands before m_cmds begin
        m_cmds.insert(m_cmds.begin(), commands.begin(), commands.end());
    }

    // -------------------------------------------------------------

    void SessionDRI::NotifyOwnerCommandCompleted( shared_ptr<CommandDRI> cmd )
    {
        // debug
        QString command = cmd->CommandToString();
        if (command.indexOf("CS_Set 2") >= 0)
        {
            int i = 0;
        }

        // log
        if (!cmd->IsService())
        {
            // name
            Log(cmd->CommandToString());
            
            // result + output
            const bool resWithOutput = true;
            QString res(cmd->ResultToString(resWithOutput));
            int trimCount = m_domain.getDomain().Params().LogCmdResultTrim();
            if (trimCount > 0) res = DriUtils::CommandResultToLine(res, trimCount, true);
            Log(res);
        }

        // trace
        if ( m_log->LogActive(m_tagTrace) )
        {
            QString output = cmd->Result()->OutputAsString();
            QString info;
            info += cmd->GetParsedCmd()->getMethodOrPropertyName();
            info += (cmd->IsService()) ? " SERVICE" : "";

            if (cmd->Result()->OK())
            {
                info += " OK";
            }
            else
            {
                info += " ERR " + cmd->Result()->ErrorTypeinfo();
            }

            *m_log << m_tagTrace << info << EndRecord;
        }

        // notify
        m_owner.CommandCompleted(cmd);
    }

    // -------------------------------------------------------------

    void SessionDRI::DoTransactionCompleted()
    {
        ESS_ASSERT(m_currTransaction != 0);

        m_owner.TransactionEndExec();

        ExeDRI().TransactionDone( m_currTransaction.get() );
        m_currTransaction.reset();

        if (m_exitRequested)
        {
            m_exitRequested = false;
            m_owner.SessionExitRequest();
            return;
        }

        if (m_transactions.empty()) CommandsToTransations();

        if (m_cmds.empty() && m_transactions.empty()) 
        {
            m_owner.CommandQueueEmpty();
        }
    }

    // -------------------------------------------------------------

    bool SessionDRI::FindProperty( QObject *pObj, QString propertyName, QMetaProperty &metaProperty )
    {
        ESS_ASSERT(pObj != 0);

        const QMetaObject *pMeta = pObj->metaObject();
        ESS_ASSERT(pMeta != 0);

        for(int i = 0; i < pMeta->propertyCount(); ++i)
        {
            metaProperty = pMeta->property(i);

            if (metaProperty.name() == propertyName.toStdString()) return true;
        }

        return false;
    }

    // -------------------------------------------------------------

    bool SessionDRI::FindMethod( QObject *pObj, QString searchName, QMetaMethod &method )
    {
        ESS_ASSERT(pObj != 0);

        const QMetaObject *pMeta = pObj->metaObject();
        ESS_ASSERT(pMeta != 0);

        for(int i = 0; i < pMeta->methodCount(); ++i)
        {
            method = pMeta->method(i);

            if (method.methodType() != QMetaMethod::Method) continue;

            if (DriUtils::MethodName(method) == searchName.toStdString())
            {
                return true;
            }
        }

        return false;
    }

    // -------------------------------------------------------------

    bool SessionDRI::CommandsToTransations()
    {
        bool transCreated = false;

        while(true)
        {
            int transSize = FindTransaction(); 
            if (transSize == 0) continue;  // try again
            if (transSize < 1) break;

            CreateTransaction(transSize);
            transCreated = true;

            // fix  -- don't convert all commands to transactions 
            // 'couse CS_Exec incorrectly add commands to end 
            break;
        }

        // activate
        if (transCreated) ExeDRI().Activate();

        return transCreated;
    }

    // -------------------------------------------------------------

    void SessionDRI::Log(const QString &data)
    {
        if (!m_log->LogActive(m_tagInfo)) return;
			
        *m_log << m_tagInfo << data << EndRecord;
    }

    // -------------------------------------------------------------

    Domain::NamedObject* SessionDRI::ObjectForCmd( QString name )
    {
        if (!Domain::Identificator::CheckObjectName(name))
        {
            ESS_HALT("");
            return 0;
        }

        return m_domain.getDomain().FindFromRoot(name);
    }

    // -------------------------------------------------------------

    SessionDRI::~SessionDRI()
    {
        // ESS_ASSERT(m_currTransaction == 0);

        // is it safe - ?
        /*if (m_currTransaction != 0)
        {
            // Domain::NamedObject *p = m_domain.getDomain().ObjectRoot()->FindObject(m_currObjectName);
            // p->AbortAsyncCommand();

            ExeDRI().TransactionDone( m_currTransaction.get() );
            m_currTransaction.reset();
        }*/
        DropAllCommands();

        ExeDRI().UnregisterSource(this);
    }

    // -------------------------------------------------------------

    void SessionDRI::DropAllCommands()
    {
        if (m_currTransaction != 0)
        {
            // Domain::NamedObject *p = m_domain.getDomain().ObjectRoot()->FindObject(m_currObjectName);
            // p->AbortAsyncCommand();

            ExeDRI().TransactionDone( m_currTransaction.get() );
            m_currTransaction.reset();
        }

        while (!m_transactions.empty()) 
        {
            m_transactions.pop();
        }
        
        m_cmds.clear();
        // m_currObjectName.clear();  
    }

    // -------------------------------------------------------------

    void SessionDRI::SwitchSecProfile( QString profileName )
    {
        using namespace DriSec;

        ISecContext &sec = m_domain.getDomain().Security();
        NObjSecProfile *p = sec.FindProfile(profileName);
        if (p == 0) 
        {
            ESS_THROW_MSG(DRI::ExecuteException, "Can't find profile!");
        }

        m_security = p->GetSecurity();
    }

}  // namespace DRI

