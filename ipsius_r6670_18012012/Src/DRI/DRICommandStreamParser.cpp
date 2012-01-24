
#include "stdafx.h"

#include "DRICommandStreamParser.h"
#include "ParsedDRICmd.h"
#include "DRIParserHelpers.h"
#include "DRIParserConsts.h"

namespace DRI
{
    void DRICommandStreamParser::AddData(const QString &data)
    {
        m_buff += data;
        if ( !m_buff.contains('\n') ) return;

        ParseBuff();
    }

    // -----------------------------------------------------------

    void DRICommandStreamParser::ExtractFullCmds(QString &inData, QStringList &outCmdList)
    {
        QStringList splitedToLines;
        // split inData into commands
        int endOfLastCmd = SplitWithQuotesConsideration(inData, '\n', splitedToLines);
        // remove incomlete cmd
        if ( endOfLastCmd != (inData.size() - 1) ) splitedToLines.removeLast(); 
        if ( splitedToLines.isEmpty() ) return;

        // advanced split with CCmdSep
        for (int i = 0; i < splitedToLines.size(); ++i)
        {
            QStringList splitedByCCmdSep;
            SplitWithQuotesConsideration(splitedToLines.at(i), CCmdSep, splitedByCCmdSep);
            outCmdList += splitedByCCmdSep;
        }

        // remove completed data and last separator from inData
        inData = inData.remove( 0, (endOfLastCmd + 1) );
    }

    // -----------------------------------------------------------

    void DRICommandStreamParser::ParseBuff()
    {
        QStringList cmdList;
        ExtractFullCmds(m_buff, cmdList);
        if ( cmdList.isEmpty() ) return;

        // parse commands and safe in list
        for (int i = 0; i < cmdList.size(); ++i)
        {
            m_cmds.push_back( new ParsedDRICmd(cmdList.at(i)) );
        }
    }

    // -----------------------------------------------------------

    boost::shared_ptr<ParsedDRICmd> DRICommandStreamParser::GetParsedCmd()
    {
        // ESS_ASSERT(m_cmds.size() != 0);
        if (m_cmds.size() == 0) return boost::shared_ptr<ParsedDRICmd>();

        boost::shared_ptr<ParsedDRICmd> firstCmd( m_cmds.pop_front().release() );
        
        return firstCmd;
    }

    // -----------------------------------------------------------

    QString DRICommandStreamParser::ResetIncompleteCmd()
    {
        QString res(m_buff);
        
        m_buff.clear();

        return res;
    }

    // -----------------------------------------------------------
    
} // namespace DRI

