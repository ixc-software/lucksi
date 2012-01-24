
#include "stdafx.h"
#include "NObjDomainAutorun.h"
// #include "DRI/FileDRI.h"
#include "DRI/driutils.h"
#include "Utils/QtHelpers.h"



namespace 
{
    const QString CTrimmedLineEnding = " ...";

} // namespace

// -----------------------------------------------------------

namespace 
{
    using namespace Domain;
    
    class ResetFileDRI : public iCore::MsgThread::AsyncTask
    {
        Utils::SafeRef<IDomainAutorunToOwner> m_task;
        QString m_err;
        
    public:
        ResetFileDRI(Utils::SafeRef<IDomainAutorunToOwner> task, const QString &err) :
            m_task(task), m_err(err)
        {
        }
            
        void Execute() // override
        {
            m_task->AutorunFinished(m_err);
        }
    };
    
} // namesapce 

// -----------------------------------------------------------

namespace Domain
{
    NObjDomainAutorun::NObjDomainAutorun(NamedObject *pParent, 
                                         const ObjectName &name,
                                         Utils::SafeRef<IDomainAutorunToOwner> owner,
                                         bool stopOnFirstErr) :
        NamedObject(&pParent->getDomain(), name, pParent),
        m_domain(pParent->getDomain()), 
        m_owner(owner),
        m_trimLineCharCount(m_domain.getDomain().Params().LogCmdResultTrim()),
        m_canPrintCharCount(m_trimLineCharCount)
    {
        m_enableTrace = (m_domain.getDomain().Params().EnableScriptTrace()
                         && (!m_domain.getDomain().Params().SuppressVerbose()));
        
        QString scriptFile = m_domain.getDomain().Params().ScriptFileName();
        ESS_ASSERT(!scriptFile.isEmpty()); 

        m_file.reset( new DRI::FileDRI(m_domain, this) );

        try
        {
            m_file->LoadFile(scriptFile);
        }
        catch(DRI::ParsedDRICmd::InvalidCommandSyntax &e)
        {
            AsyncExit(QString("Script file loading error: %1.").arg(e.getTextMessage().c_str()));
            return;
        }
        catch(DRI::DRIFileLoad::IncompleteCmd &e)
        {
            AsyncExit(QString("Script file loading error: %1 \"%2\".")
                      .arg("Incomplete command")
                      .arg(e.getTextMessage().c_str()));
            return;
        }

        m_owner->LogAutorunOutput(
                QString("Script '%1' execution's started ...").arg(scriptFile));
        
        m_file->Execute(this, stopOnFirstErr /* true */);
    }

    // -----------------------------------------------------------
    
    void NObjDomainAutorun::OnFileExecuteDone(boost::shared_ptr<DRI::FileExecuteResult> result)
    {
        bool justStats = true;

        m_owner->LogAutorunOutput(
            QString("\nScript execution's complited: %1.").arg(result->ToString(justStats)));

        QString err;
        if (result->CompleteCode() == DRI::ferAbortByError)
        {
            err = QString("Script execution error: %1.").arg(result->CompleteDesc());
        }

        AsyncExit(err);
    }

    // -----------------------------------------------------------

    void NObjDomainAutorun::AsyncExit(const QString &err)
    {
        boost::shared_ptr<ResetFileDRI> task(new ResetFileDRI(m_owner, err));
        m_domain.getDomain().getMsgThread().RunAsyncTask(task);
    }

    // -----------------------------------------------------------

    /*void NObjDomainAutorun::AddToOutput(const QString &data, bool trimIfNeed)
    {
        if (!m_enableTrace) return;
        
        QString formatted(m_coutFormatter.Format(data));
            
            if (trimIfNeed)
            {
                int trimCount = m_domain.getDomain().Params().LogCmdResultTrim();
                if (trimCount > 0) 
                {
                    formatted = DRI::DriUtils::CommandResultToLine(formatted, trimCount);
                }
            }
            
            std::cout << formatted.toStdString() << std::endl;
    }*/

    // -----------------------------------------------------------

    QString NObjDomainAutorun::TrimData(const QString &data, bool isFullLine)
    {
        if (m_trimLineCharCount <= 0) return data;

        ESS_ASSERT(m_canPrintCharCount >= 0);

        if (m_canPrintCharCount == 0) return QString();

        QString res(data);
        int canPrintWithoutEnding = m_canPrintCharCount - CTrimmedLineEnding.size();
        if ((canPrintWithoutEnding == 0) || (res.size() >= canPrintWithoutEnding))
        {
            res = DRI::DriUtils::CommandResultToLine(res, m_canPrintCharCount, 
                                                     isFullLine, CTrimmedLineEnding);
        }
        
        m_canPrintCharCount -= res.size();

        return res;
    }

    // -----------------------------------------------------------

    void NObjDomainAutorun::AddToOutput(const QString &data, bool lineFeed)
    {
        if (!m_enableTrace) return;
        
        QString printData(data);

        if (m_trimLineCharCount == m_canPrintCharCount) // new line
        {
            printData = m_coutFormatter.FormatIncompleteLineBegin(data);
        }

        if (!lineFeed)
        {
            printData = TrimData(printData, lineFeed);
            std::cout << printData << std::flush;
            return;
        }
    
        printData = m_coutFormatter.FormatIncompleteLineEnd(data);
        printData = TrimData(printData, lineFeed);
        m_canPrintCharCount = m_trimLineCharCount;
        
        std::cout << printData << std::endl;
    }
    
} // namespace Domain
