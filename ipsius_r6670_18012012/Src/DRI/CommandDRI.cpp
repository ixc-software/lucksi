#include "stdafx.h"

#include "Platform/Platform.h"

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "CommandDRI.h"
#include "driutils.h"

// ---------------------------------------

namespace DRI
{
    CommandDRI::CommandDRI(ICommandDriOwner &owner, shared_ptr<ParsedDRICmd> cmd, 
                           CommandDRI::CommandType type) : 
        m_owner(owner),
        m_cmd(cmd),
        m_type(type)
    {
        m_debug = false;
        
        // ...
    }

    // -----------------------------------------------------------------
    
    void CommandDRI::Completed(CommandDriResult *pError, bool withNotify)
    {
        ESS_ASSERT(m_result == 0);
        ESS_ASSERT(pError != 0);

        // if (!m_started) Started();

        FlushOutput(true);

        ESS_ASSERT(m_outputBuffer.empty());

        m_result.reset(pError);

        if (withNotify)
        {
            m_owner.OnCommandCompleted(*this);
        }
    }

    // -----------------------------------------------------------------

    QString CommandDRI::ResultToString(bool withOutput, bool extErrorInfo) const
    {
        ESS_ASSERT(m_result != 0);
        
        QString s;

        if (withOutput) s += OutputToString();
        
        if (!s.isEmpty()) s += C_LF;

        if (m_result->OK())
        {
            s += ServiceLinePrefix() + "OK";
        }
        else
        {
            QString location = m_cmd->LocationInfo();
            if ((extErrorInfo) && !location.isEmpty())
            {
                s += ServiceLinePrefix() + "LOCATION ";
                s += location;
                s += C_LF;
            }
            s += ServiceLinePrefix() + "ERROR ";
            s += m_result->ErrorTypeinfo();
        }

        return s;
    }

    // -----------------------------------------------------------------

    QString CommandDRI::OutputToString() const
    {
        ESS_ASSERT(m_result != 0);
        return m_result->OutputAsString();
    }

    // -----------------------------------------------------------------
    
    void CommandDRI::PropertyRead( Domain::NamedObject *pObj, QString propertyName )
    {
        Started();
        
        QString val = pObj->Property(propertyName, true); 

        Add(val, true);
        
        Completed(new CommandDriResultOk(val), true);
    }

    // -----------------------------------------------------------------

    void CommandDRI::PropertyWrite( Domain::NamedObject *pObj, QString propertyName, QString val )
    {
        Started();
        
        bool result = false;

        try
        {
            result = pObj->Property(propertyName, val);

            if (!result) ESS_THROW_MSG(DRI::ExecuteException, "Can't set property");
        }
        catch(const std::exception &e)
        {
            QString msg = QString(" @ %1.%2 = %3")
                .arg(pObj->Name().Name(true))
                .arg(propertyName)
                .arg(val);

            CompleteWithException<CommandDriWriteProperty>(e, msg);
            return;
        }

        Completed(new CommandDriResultOk(), true);
    }

    // -----------------------------------------------------------------

    void CommandDRI::ProcessSpecialFirstParam( bool &cmdIsAsync )
    {
        cmdIsAsync = false;

        QList<QByteArray> argTypes = m_invoke->Method().parameterTypes();
        if (argTypes.size() < 1) return;

        QString firstType = argTypes.at(0);

        ISessionCmdContext *pCmdContext = m_owner.GetSessionCmdContext();
        if (firstType == DriUtils::QtTypename(pCmdContext))
        {
            m_invoke->AddSpecialParam(pCmdContext);
        }

        ICmdOutput *pCmdOutput = this;
        if (firstType == DriUtils::QtTypename(pCmdOutput))
        {
            m_invoke->AddSpecialParam(pCmdOutput);
        }

        IAsyncCmd *pAsyncCmd = this;
        if (firstType == DriUtils::QtTypename(pAsyncCmd))
        {
            cmdIsAsync = true;
            m_invoke->AddSpecialParam(pAsyncCmd);
        }
    }

    // -----------------------------------------------------------------

    void CommandDRI::AsyncStarted(Utils::SafeRef<Domain::NamedObject> asyncObject)
    {
        ESS_ASSERT(m_asyncObject.IsEmpty());
        m_asyncObject = asyncObject;
    }

    // -----------------------------------------------------------------
    
    void CommandDRI::AsyncCompleted(bool isOK, const QString &errorMsg)
    {
        ESS_ASSERT(m_result == 0);

        ESS_ASSERT(!m_asyncObject.IsEmpty());
        m_asyncObject.Clear();
        m_asyncCompleted.Set(true);
        
        if ((isOK) && (!errorMsg.isEmpty())) Add(errorMsg, true);

        // if !isOK -- it's abort so we can't output anything
        if (isOK) FlushOutput(true); // copy buffer to output
        
        shared_ptr<CommandAsyncDone> async( new CommandAsyncDone(*this, isOK, errorMsg));
        m_owner.CommandRunAsyncCallback(async);
    }

    // -----------------------------------------------------------------

    void CommandDRI::Invoke( shared_ptr<MetaInvoke> invoke )
    {
        ESS_ASSERT(m_result == 0);
        ESS_ASSERT(m_invoke == 0);
        m_invoke = invoke;

        Started();
        
        try
        {
            // detect special type first param
            bool cmdIsAsync = false;  // dummi
            ProcessSpecialFirstParam(cmdIsAsync);  

            // add params from m_cmd to m_invoke
            m_invoke->AddParams( m_cmd->getParams() );  

            // invoke
            m_invoke->Invoke();  
        }
        catch(const std::exception &e)
        {
            if (!m_asyncObject.IsEmpty()) AsyncException(e);
            else CompleteWithException<CommandDriRuntimeError>(e, m_invoke->InvokeInfo());
            return;
        }

        // complete sync cmd
        // (m_asyncObject can be set and reset againg in invocable method)
        if (m_asyncObject.IsEmpty() && !m_asyncCompleted)
        {
            FlushOutput(true); // copy buffer to output

            Completed(new CommandDriResultOk(m_output), true);
        }
    }

    // -----------------------------------------------------------------

    void CommandDRI::Flush()
    {
        FlushOutput();
    }

    // -----------------------------------------------------------------

    bool CommandDRI::TryComplete(bool haltOnFail)
    {
        // возможно, ASSERT это сильно жестко
        // может ли "зависать" в деструкторе транзакции не асинхронная команда - ?!
        ESS_ASSERT( !Completed() );
        ESS_ASSERT( !m_asyncObject.IsEmpty() );
        
        bool aborted = m_asyncObject->AbortAsyncCommand();
        if (aborted)
        {
            return true;
        }

        if (!haltOnFail) return false;             
        
        QString err = QString("Session closed while the asynchronous command \"%1.%2\" "
                              "has being executed!")
                              .arg(m_asyncObject->Name().Name())
                              .arg(GetParsedCmd()->getMethodOrPropertyName());
        // ESS_HALT(err.toStdString());
        Platform::ExitWithMessage(err.toStdString().c_str());

        return false;
    }

    // -----------------------------------------------------------------

    void CommandDRI::FlushOutput(bool last)
    {
        // if it's last flush, force append LF to the last part of data

        int size = m_outputBuffer.size();
        for (int i = 0; i < size; ++i)
        {
            QString str = m_outputBuffer.at(i).Str;
            bool lf = m_outputBuffer.at(i).LineFeed;
            if (last && (i == (size - 1))) lf = true; 

            // send to owner
            m_owner.OnCommandOutput(str, lf);

            // copy to the common output
            DriUtils::AddToStringList(m_output, str, lf);

            if (m_debug) std::cout << "CommandDRI::FlushOutput(" << str.toStdString() << ")" << std::endl;
        }

        // clear buffer
        m_outputBuffer.clear();
    }

    // -----------------------------------------------------------------

    void CommandDRI::Add(const QString &data, bool lineFeed)
    {
        if (m_debug) std::cout << "CommandDRI::Add(" << data.toStdString() << ")" << std::endl;
        
        m_outputBuffer.push_back(OutputData(data, lineFeed));
    }

    // -----------------------------------------------------------------

    void CommandDRI::AsyncException(const std::exception &e)
    {
        QString err(DriUtils::ExceptionToString(e));
        QString msg = QString("Exception inside the asynchronous "
                              "command \"%1.%2\": %3")
                              .arg(m_asyncObject->Name().Name()) 
                              .arg(GetParsedCmd()->getMethodOrPropertyName()) 
                              .arg(err);
        
        ESS_HALT(msg.toStdString());
    }

    // -----------------------------------------------------------------

    void CommandDRI::Started()
    {
        m_startDt = QDateTime::currentDateTime();
        m_owner.OnCommandStarted(*this);
    }

    // -----------------------------------------------------------------

    DriSec::ISecurity* CommandDRI::GetSecurity()
    {
        return m_owner.GetSessionCmdContext()->GetSecurity();
    }

}  // namespace DRI

