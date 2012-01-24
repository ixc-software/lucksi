#include "stdafx.h"

#include "drifileload.h"
#include "DriDefineAndMacroList.h"


namespace
{
    // -------------------------------------------------
    
    const QString CInclude = "#include";
    const QChar CInclLeftBreak = '<';
    const QChar CInclRightBreak = '>';
    const int CMaxIncludeDepth = 10;

} // namespace

// -------------------------------------------------

namespace DRI
{
    // -------------------------------------------------
    // DRIFileLoad::RawCommand impl
    
    DRIFileLoad::RawCommand::RawCommand(const QString &cmd, 
                                        int cmdNum, 
                                        const QString &file, 
                                        int line): 
        m_cmd(cmd), m_cmdNum(cmdNum), m_fileName(file), m_lineNum(line)
    {
        ESS_ASSERT(!m_cmd.isEmpty());
        ESS_ASSERT(m_cmdNum > 0);
        ESS_ASSERT(!m_fileName.isEmpty());
        ESS_ASSERT(m_lineNum > 0);
    }

    // -------------------------------------------------

    QString DRIFileLoad::RawCommand::Location() const
    {
        QString res = QString("%1, line %2 (command # %3)")
                              .arg(m_fileName).arg(m_lineNum).arg(m_cmdNum);
        return res;
    }

    // -------------------------------------------------
    // DRIFileLoad::Preprocessor impl
    
    class DRIFileLoad::Preprocessor
    {
        struct Location
        {
            QString File;
            int Line;

            Location(const QString &file) : 
                File(file.trimmed()), Line(0)
            {
            }
        };
        
        DriDefineAndMacroList m_defs;
        const DriIncludeDirList &m_pathes;
        // Utils::StringList m_result;
        std::vector<DRIFileLoad::RawCommand> m_result;

        bool m_debug;

        static void Out(const QString &data) { std::cout << data << std::endl; } 
        
        static void IncludeErr(const QString &reason, const QString &src)
        {
            QString msg = QString("%1 in '%2'").arg(reason).arg(src);
            ESS_THROW_MSG(DriIncludeError, msg.toStdString());
        }

        static bool StartWith(QString s, QString toCompare, int indx)
        {
            if ((s.size() - indx) < toCompare.size()) return false;

            return (s.mid(indx, toCompare.size()) == toCompare);
        }

        /*
        static QString SimplifyLine(QString s, bool &inComment)
        {
            s = s.trimmed();

            int inString = 0;

            for(int i = 0; i < s.size(); ++i)
            {
                if (s.at(i) == CStringChar) inString++;

                if ((inString & 1) == 0)  // outside string
                {
                    if (StartWith(s, CComment, i)) return s.left(i).trimmed();
                }
            }

            return s;
        } */

        // inComment -- global state of multiline comment 
        static QString SimplifyLine(QString s, bool &inComment)
        {
            s = s.trimmed();

            QString res;
            bool inLocalComment = false;
            int inString = 0;

            int pos = 0;

            while(pos < s.size())
            {
                if (s.at(pos) == CStringChar) inString++;   // in string detection

                if ((inString & 1) == 0)  // outside string -- process global and local comments
                {
                    if (inComment)
                    {
                        if ( StartWith(s, CGlobalCommentEnd, pos) ) 
                        {
                            inComment = false;
                            pos += CGlobalCommentEnd.length();
                            continue;
                        }
                    }
                    else
                    {
                        if ( StartWith(s, CGlobalCommentBegin, pos) )
                        {
                            inComment = true;
                            pos += CGlobalCommentBegin.length();
                            continue;
                        }

                        if ( !inLocalComment && StartWith(s, CComment, pos) )
                        {
                            inLocalComment = true;
                            pos += CComment.length();
                            continue;
                        }

                    }
                }

                if (!(inComment || inLocalComment)) // add to result current char if not in comment
                {
                    res.append( s.at(pos) );
                }

                ++pos;  // next
            }

            return res;
        }

        static bool CutBeginEndChars(QChar begin, QChar end, QString &str)
        {
            if ((!str.startsWith(begin)) && (!str.endsWith(end))) return false;

            str.remove(0, 1).chop(1);

            return true;
        }

        static QString ExtractFileName(QString line)
        {
            QString fileName = line.mid(CInclude.size()).trimmed();
            if (!CutBeginEndChars(CStringChar, CStringChar, fileName)) 
            {
                CutBeginEndChars(CInclLeftBreak, CInclRightBreak, fileName);
            }
    
            return fileName;
        }

        void ProcessLine(QString line, int level, const Location &location, bool &inComment)
        {
            if (m_debug) Out(QString("Line (level %1): %2").arg(level).arg(line));

            // remove comments, etc.
            line = SimplifyLine(line, inComment); 
            if (line.isEmpty()) return;

            // process includes
            if (line.startsWith(CInclude)) 
            {
                ProcessInclude(line, level + 1, location);
                return;
            }

            // process defines
            line = m_defs.ProcessLine(line);
            
            if (line.isEmpty()) return;

            // save
            int cmdNum = m_result.size() + 1;
            m_result.push_back(DRIFileLoad::RawCommand(line, cmdNum, 
                                                       location.File, location.Line));
        }

        void ProcessInclude(const QString &line, int level, const Location &location) // can throw
        {
            QString fileName(ExtractFileName(line));
            if (fileName.isEmpty()) IncludeErr("#include without parameters", line);
            
            Utils::StringList sl;
            LoadFromFile(fileName, m_pathes, sl);
            
            QString includedFrom = QString("%1 (included from %2, line %3)")
                                   .arg(fileName.trimmed()).arg(location.File, location.Line);
            Process(sl, level, includedFrom);
        }
         
    public:
        Preprocessor(const DriIncludeDirList &pathes, 
                     const QStringList &specParamsList,
                     const QString &macroReplaceList = "") 
            : m_defs(specParamsList, macroReplaceList), m_pathes(pathes)
        {
            m_debug = false;
        }

        void Process(const Utils::StringList &data, int level, const QString &location)
        {
            if (m_debug) Out(QString("List (level %1)").arg(level));

            if (level > CMaxIncludeDepth) 
            {
                IncludeErr(QString("Include depth more than %1.").arg(CMaxIncludeDepth), 
                           ""/*fileName*/);
            }

            Location fileLine(location);
            bool inComment = false;  // state var for multiline comments 
            for (int i = 0; i < data.size(); ++i)
            {
                fileLine.Line = i + 1; // starts from 1
                ProcessLine(data.at(i), level, fileLine, inComment);
            }
        }

        // const Utils::StringList& Result() const { return m_result; }
        const std::vector<DRIFileLoad::RawCommand>& Result() const { return m_result; }

        static void LoadFromFile(QString fileName, 
                                 const DriIncludeDirList &pathes, 
                                 Utils::StringList &res) // can throw
        {
            res.clear();
            fileName = pathes.Find(fileName);
            res.LoadFromFile(fileName);
        }
    };

    // --------------------------------------------------------
    
    DRIFileLoad::DRIFileLoad(QString fileName, 
                             const DriIncludeDirList &pathes, 
                             const QStringList &specParamsList,
                             QString macroReplaceList /*= ""*/) /* can throw */
    {
        Utils::StringList sl;
        Preprocessor::LoadFromFile(fileName, pathes, sl);
        Load(sl, pathes, specParamsList, macroReplaceList, fileName);
    }

    // --------------------------------------------------------

    DRIFileLoad::DRIFileLoad(const Utils::StringList &script,
                             const DriIncludeDirList &pathes,
                             const QStringList &specParamsList,
                             QString macroReplaceList /*= ""*/)  /* can throw */
    {
        Load(script, pathes, specParamsList, macroReplaceList, "<list>");
    }

    // --------------------------------------------------------

    void DRIFileLoad::Load(const Utils::StringList &script,
                           const DriIncludeDirList &pathes,
                           const QStringList &specParamsList,
                           const QString &macroReplaceList,
                           const QString &location)
    {
        Preprocessor p(pathes, specParamsList, macroReplaceList);
        p.Process(script, 1, location); // can throw

        ParseCommands(location, p.Result());
    }

    void DRIFileLoad::ParseCommands( const QString &fileName, 
                                     const std::vector<DRIFileLoad::RawCommand> &sl )
    {
        DRICommandStreamParser parser;

        for(int i = 0; i < sl.size(); ++i)
        {
            QString s = sl.at(i).get();
            
            // parse data
            parser.AddData(s + "\n");

            while(true)
            {
                CmdPtr cmd = parser.GetParsedCmd();
                if (cmd == 0) break;

                cmd->LocationInfo(sl.at(i).Location());

                m_cmds.push(cmd);
            }

            QString incompleteCmd(parser.ResetIncompleteCmd().trimmed());
            if (incompleteCmd.isEmpty()) continue;
            ESS_THROW_MSG(IncompleteCmd, incompleteCmd.toStdString());
        }
    }

    // --------------------------------------------------------

    void DRIFileLoad::Parse( const Utils::StringList &script, 
                             const DriIncludeDirList &pathes, 
                             const QStringList &specParamsList, 
                             const QString &macroReplaceList, 
                             std::vector<RawCommand> &result )
    {
        Preprocessor p(pathes, specParamsList, macroReplaceList);
        p.Process(script, 1, "<list>"); 
        result = p.Result();
    }

    
    
}  // namespace DRI

