#pragma once

#include "Utils/StringList.h"

#include "ParsedDRICmd.h"
#include "DRICommandStreamParser.h"
#include "DriIncludeDirList.h"
#include "Utils/SafeRef.h"

namespace DRI
{
    using boost::shared_ptr;

    // Load script from file or QStringList
    class DRIFileLoad : boost::noncopyable
    {
    public:
        class RawCommand;

    private:

        typedef shared_ptr<ParsedDRICmd> CmdPtr;

        std::queue<CmdPtr> m_cmds;
        
        class Preprocessor;

        void Load(const Utils::StringList &script,
                  const DriIncludeDirList &pathes,
                  const QStringList &specParamsList,
                  const QString &macroReplaceList, 
                  const QString &location);
        
        void ParseCommands(const QString &fileName, const std::vector<RawCommand> &sl);
        
    public:
        ESS_TYPEDEF(DriIncludeError);
        ESS_TYPEDEF(IncompleteCmd);

        class RawCommand
        {
            QString m_cmd;
            int m_cmdNum;
            QString m_fileName;
            int m_lineNum;

        public:
            RawCommand(const QString &cmd, int cmdNum, const QString &file, int line);

            const QString& get() const  { return m_cmd; } 
            QString Location() const;
        };

    public:
        
        DRIFileLoad(QString fileName, 
                    const DriIncludeDirList &pathes,
                    const QStringList &specParamsList,
                    QString macroReplaceList = "");  // can throw
        
        DRIFileLoad(const Utils::StringList &script,
                    const DriIncludeDirList &pathes,
                    const QStringList &specParamsList,
                    QString macroReplaceList = "");  // can throw
        
        CmdPtr Peek()
        {
            if (m_cmds.empty()) return CmdPtr();

            CmdPtr cmd = m_cmds.front();
            m_cmds.pop();
            return cmd;
        }

        bool Empty() const { return m_cmds.empty(); }

        static void Parse(const Utils::StringList &script,
                          const DriIncludeDirList &pathes,
                          const QStringList &specParamsList,
                          const QString &macroReplaceList,
                          std::vector<RawCommand> &result);
    };
    
    
}  // namespace DRI

