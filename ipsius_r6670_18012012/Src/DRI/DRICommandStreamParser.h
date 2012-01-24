
#ifndef __DRICOMMANDSTREAMPARSER__
#define __DRICOMMANDSTREAMPARSER__

#include "Utils/IBasicInterface.h"
#include "IIdentificatorValidator.h"
#include "Utils/SafeRef.h"
                                 
namespace DRI
{
    class ParsedDRICmd;

    // Collect DRI command parts and parse full commands.
    // When atleast one cmd parsed, HasParsedCmd() returns 'true'
    class DRICommandStreamParser
        : boost::noncopyable
    {
        boost::ptr_list<ParsedDRICmd> m_cmds;
        QString m_buff;

        // Split inData to list of no-parsed commands and returns then as outCmdList;
        // cut full commands from inData
        static void ExtractFullCmds(QString &inData, QStringList &outCmdList);
        void ParseBuff();
        
    public:
        DRICommandStreamParser()
        {}

        void AddData(const QString &data); // can throw ParsedDRICmd::InvalidCommandSyntax
        boost::shared_ptr<ParsedDRICmd> GetParsedCmd();
        bool HasParsedCmd() const { return (m_cmds.size() > 0); }

        // Returns incomplete cmd if any
        QString ResetIncompleteCmd();
    };
    
} // namespace DRI

#endif

