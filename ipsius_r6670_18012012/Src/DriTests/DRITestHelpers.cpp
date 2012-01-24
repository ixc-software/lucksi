
#include "stdafx.h"
#include "DRITestHelpers.h"

namespace
{
    bool LineContainsCommand(const QString &line)
    {
        return (line.startsWith(">"));
    }

    // -----------------------------------------------------------------

    QString GetFirstLines(const QString &s, int linesCount)
    {
        int pos = -1;
        int count = 0;

        for(int i = 0; i < s.size(); ++i)
        {
            if (s.at(i) == QChar(0x0a))
            {
                ++count;
                if (count >= linesCount) 
                {
                    pos = i;
                    break;
                }
            }
        }

        return (pos >= 0) ? s.left(pos) : s;
    }


    // -----------------------------------------------------------------

    class BlockComparator
    {
        bool m_leftBlockEnd;
        bool m_rightBlockEnd;
        QStringList m_leftBuffer;
        QStringList m_rightBuffer;

        static bool IsSpecialCommandEnd(const QString &line)
        {
            return (line.startsWith(":OK") || line.startsWith(":ERROR"));
        }

        static bool IsBlockEnd(const QString &line)
        {
            return (IsSpecialCommandEnd(line) || LineContainsCommand(line));
        }

    private:
        virtual int CompareInner() = 0;

    protected:
        const QStringList& LeftBuffer() { return m_leftBuffer; }
        const QStringList& RightBuffer() { return m_rightBuffer; }

    public:
        BlockComparator() :
            m_leftBlockEnd(false),
            m_rightBlockEnd(false)
        {
        }

        virtual ~BlockComparator()
        {
        }

        int Compare()
        {
            ESS_ASSERT(m_leftBlockEnd && m_rightBlockEnd);

            int res = CompareInner();

            m_leftBuffer.clear();
            m_rightBuffer.clear();

            m_leftBlockEnd = false;
            m_rightBlockEnd = false;

            return res;
        }

        // returns true if need next
        bool AddLeft(const QString &left)
        {
            ESS_ASSERT(!m_leftBlockEnd);

            m_leftBlockEnd = IsBlockEnd(left);
            if (!m_leftBlockEnd) m_leftBuffer.push_back(left);

            return !m_leftBlockEnd;
        }

        // returns true if need next
        bool AddRight(const QString &right)
        {
            ESS_ASSERT(!m_rightBlockEnd);

            m_rightBlockEnd = IsBlockEnd(right);
            if (!m_rightBlockEnd) m_rightBuffer.push_back(right);

            return !m_rightBlockEnd;
        }
    };

    // -----------------------------------------------------------------

    class MetaCommandsComparator : public BlockComparator
    {
        int CompareInner() // override
        {
            int res = 0;

            if (LeftBuffer().isEmpty() && (!RightBuffer().isEmpty())) res = 1;
            if ((!LeftBuffer().isEmpty()) && RightBuffer().isEmpty()) res = -1;

            return res;
        }
    };

    // -----------------------------------------------------------------

    class MetaTypesComparator : public BlockComparator
    {
        int CompareWithCondition(const QString &left, const QString &right,
                                 const QString &typeName)
        {
            if (left.contains(typeName) && (!right.contains(typeName))) return -1;
            if (!left.contains(typeName) && (right.contains(typeName))) return 1;

            return 0;
        }

        int CompareInner() // override
        {
            // one is empty
            if (LeftBuffer().isEmpty() && (!RightBuffer().isEmpty())) return 1;
            if ((!LeftBuffer().isEmpty()) && RightBuffer().isEmpty()) return -1;

            QString left = LeftBuffer().join(";");
            QString right = RightBuffer().join(";");

            // don't have needed output
            int res = CompareWithCondition(left, right, "NObjRoot");
            if (res != 0) return res;

            res = CompareWithCondition(left, right, "NObjDomainAutorun");
            if (res != 0) return res;

            res = CompareWithCondition(left, right, "NObjTelnetServerDRI");
            if (res != 0) return res;

            return 0;
        }
    };

} // namespace

// -----------------------------------------------------------------

namespace DriTestHelpers
{
    // -----------------------------------------------------------------
    // ScriptRun::Comparator impl

    class ScriptRun::Comparator : public DRI::IDiffLinesCompare
    {
        boost::scoped_ptr<BlockComparator> m_blockComparator;

        static QString FindSpecialCommand(const QString &line)
        {
            if (!LineContainsCommand(line)) return QString();

            if (line.contains("MetaTypes")) return "MetaTypes";
            if (line.contains("MetaTypeInfo")
                || line.contains("MetaMethods")
                || line.contains("MetaFields")
                || line.contains("MetaEnums")) return "MetaOther";

            return QString();
        }

        BlockComparator* NewBlockComparator(const QString &specCommand)
        {
            if (specCommand == "MetaTypes") return new MetaTypesComparator;
            if (specCommand == "MetaOther") return new MetaCommandsComparator;

            return 0;
        }

        // work with 'old' and 'new' exception info
        static bool IsExceptionLine(QString s)
        {
            return s.startsWith("Exception")                       &&
                  (s.contains(".h:") || s.contains(".cpp:")        &&
                   s.contains(" @ "));
        }

    // DRI::IDiffLinesCompare impl
    public:

        int CompareLine(QString left, QString right, bool &needCompareNextAsBlock)
        {
            ESS_ASSERT(m_blockComparator == 0);
            needCompareNextAsBlock = false;

             // ingnore -- different place of script files
            QString exp(":LOCATION");
            if (left.startsWith(exp)) return 0;

            // ignore 'couse different source file lines
            if (IsExceptionLine(left) && IsExceptionLine(right)) return 0;

            // ignore MsgProcessor.Statistic 
            exp = "MsgProcessed =";
            if (left.contains(exp) && right.contains(exp)) return 0;

            // compare
            int res = left.compare(right);
            if (res != 0) return res;

            // check if next will be parsed as block
            QString specCommand = FindSpecialCommand(left);
            if (!specCommand.isEmpty())
            {
                needCompareNextAsBlock = true;
                m_blockComparator.reset(NewBlockComparator(specCommand));
            }

            return res;
        }

        int CompareBlock()
        {
            ESS_ASSERT(m_blockComparator != 0);

            int res = m_blockComparator->Compare();

            m_blockComparator.reset();

            return res;
        }

        // returns true if need next
        bool ComparingBlockAddLeft(QString left)
        {
            ESS_ASSERT(m_blockComparator != 0);

            return m_blockComparator->AddLeft(left);
        }

        // returns true if need next
        bool ComparingBlockAddRight(QString right)
        {
            ESS_ASSERT(m_blockComparator != 0);

            return m_blockComparator->AddRight(right);
        }
    };

    // -----------------------------------------------------------------
    // ScriptRun impl

    void ScriptRun::LoadScript()
    {
        m_file.reset( new FileDRI(m_domain) );
        m_file->LoadFile(m_params.ScpFile);
        m_file->Execute(this, false);
    }

    // -----------------------------------------------------------------
    
    QString ScriptRun::LoadFile(QString fileName)
    {
        QFile file( fileName );
        TUT_ASSERT( file.open(QIODevice::ReadOnly) );

        QByteArray data = file.read( file.size() );

        return QString(data);
    }

    // -----------------------------------------------------------------
    // IFileExecuteDone impl
    
    void ScriptRun::OnFileExecuteDone(shared_ptr<FileExecuteResult> result)
    {
        using namespace std;
        QString output = result->ToString();

        if (!m_params.SilenceMode)
        {
            cout << output.toStdString() << endl;
        }

        // debug -- save output
        {
            QByteArray data = output.toAscii();
            QString fileName = m_params.ScpRefOutput.isEmpty() ? 
                m_params.ScpFile : m_params.ScpRefOutput;
            QFile f(fileName + ".curr");
            ESS_ASSERT(f.open(QIODevice::WriteOnly));
            f.write(data);
        }

        if (!m_params.ScpRefOutput.isEmpty())
        {
            DRI::DiffList diff;
            Comparator cmp;
            bool res = DRI::Diff::Do(LoadFile(m_params.ScpRefOutput), output, diff, &cmp);
            // m_params.OK = res;

            if (!res)
            {
                m_params.Diffs = QString("* DIFF for script '%1'*\n\n%2\n")
                                    .arg(m_params.ScpFile).arg(diff.ToString());
            
                if (!m_params.SilenceMode) cout << endl << m_params.Diffs.toStdString();
            }
        }

        // done
        m_domain.Stop(Domain::DomainExitOk);
    }

    // -----------------------------------------------------------------

    ScriptRun::ScriptRun(DomainClass &domain, ScriptRunParams &params) : 
        iCore::MsgObject(domain.getMsgThread()),
        m_domain(domain),
        m_params(params)
    {
        // trace to UDP
        SetupLog(domain);
        
        // start
        PutMsg(this, &ScriptRun::LoadScript);
    }

    // -----------------------------------------------------------------

    ScriptRun::~ScriptRun()
    {
    }
    
    // -----------------------------------------------------------------
    // runners
        
    void RunScript(const DriTests::DriTestsParams &params,
                      QString scriptName, 
                      QString scriptRefOutput,
                      const QStringList &specParams, 
                      /* out */ QString *pError)
    {
        DRI::DriIncludeDirList dl(params.ScriptDirs);
        
        Domain::DomainTestRunner runner;
        runner.Params().IncludeDirList(dl);
        runner.Params().SpecialParams(specParams);
        
        ScriptRunParams runParams;
        runParams.SilenceMode = params.SilentMode;
        runParams.ScpFile = dl.Find(scriptName); // can throw
        if (!scriptRefOutput.isEmpty()) runParams.ScpRefOutput = dl.Find(scriptRefOutput); // can throw

        bool res = runner.Run<ScriptRun>(runParams);
        if (!res) 
        {
            const char *pMsg = "Test timeout";
            if (pError == 0) ESS_THROW_MSG(ESS::TutException, pMsg);
            *pError = pMsg;
            return;
        }

        if (runParams.Diffs.isEmpty()) return;

        const int CMaxDiffLines = 8;
        QString msg = GetFirstLines(runParams.Diffs, params.MaxDiffLines);

        if (pError == 0) ESS_THROW_MSG(ESS::TutException, msg.toStdString());
        *pError = msg;
    }
    
    // -----------------------------------------------------------------
    
    void SetupLog(DomainClass &domain)
    {
        domain.Log().Output().TraceToUdp(false);
		domain.Log().Output().DstUdpTrace(domain.Params().RemoteLogAddr());
		domain.Log().Output().TraceToUdp(true);
    }


} // namespace DriTestHelperts
