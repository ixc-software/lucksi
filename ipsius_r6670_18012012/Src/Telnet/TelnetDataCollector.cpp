#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"

#include "TelnetDataCollector.h"
#include "TelnetCommands.h"

namespace
{
    enum
    {
        CmdNo = 1,
        OptNo = 2,

        XoptCmdNo = 0,
        XoptOptNo = 1,

        CmdGaBytes = 2,     // IAC GA
        CmdBytes = 3,       // IAC CMD OPT

        SbXoptBeginBytes = 3, // IAC SB EXOPL
        SbBeginBytes = 4,   // IAC SB OPT IS
        SbEndBytes = 2,     // IAC SE
    };

    bool IsInput = true;
}

// ------------------------------------------

namespace Telnet
{
    void TelnetDataCollector::ProcessSocketData(const QByteArray &data)
    {
        m_textBuff.clear();
        
        for (int i = 0; i < data.size(); ++i)
        {
            States prevState = m_currentState;
            ProcessChar(At(data, i));
            
            if ((prevState == InsideText) 
                && (m_currentState != prevState)) 
                SendText();
        }

        SendText();
    }

    // ------------------------------------------

    void TelnetDataCollector::SendText()
    {
        if (m_textBuff.isEmpty()) return;

        // m_logger.AddStr(m_textBuff, IsInput);
        m_pOwner->GetLogger().AddStr(m_textBuff, IsInput);
        m_pOwner->ParseText(m_textBuff);
        m_textBuff.clear();
    }

    // ------------------------------------------
    
    void TelnetDataCollector::ProcessChar(byte val)
    {
        switch (m_currentState)
        {
        case (InsideText):
            ProcessPair(val);
            break;
            
        case (InsideCmd):
            ProcessCmd(val);
            break;

        case (InsideSb):
            ProcessSb(val);
            break;

        default:
            ESS_ASSERT(0 && "Unknown state");
        }
    }

    // ------------------------------------------

    void TelnetDataCollector::ProcessIac(byte val)
    {
        if (val == CMD_SB)
        {
            m_currentState = InsideSb;
            ProcessSb(val);
        }
        else
        {
            m_currentState = InsideCmd;
            ProcessCmd(val);
        }
    }

    // ------------------------------------------

    void TelnetDataCollector::ProcessCmd(byte val)
    {
        Append(m_cmdBuff,val);

        if ((m_cmdBuff.size() == CmdGaBytes)
            && (At(m_cmdBuff,CmdNo) == CMD_GA))
        {
            // m_logger.Add(m_cmdBuff, IsInput, TelnetDataLogger::CMD_2b);
            m_pOwner->GetLogger().Add(m_cmdBuff, IsInput, TelnetDataLogger::CMD_2b);
            /* dont send above */
            m_cmdBuff.clear();
            m_currentState = InsideText;
        }

        if (m_cmdBuff.size() == CmdBytes) 
        {
            // m_logger.Add(m_cmdBuff, IsInput, TelnetDataLogger::CMD_3b);
            m_pOwner->GetLogger().Add(m_cmdBuff, IsInput, TelnetDataLogger::CMD_3b);
            m_pOwner->ParseCmd(At(m_cmdBuff, CmdNo), At(m_cmdBuff, OptNo));
            m_cmdBuff.clear();
            m_currentState = InsideText;
        }
    }

    // ------------------------------------------

    void TelnetDataCollector::ProcessSb(byte val)
    {
        Append(m_cmdBuff, val);

        if (val != CMD_SE) return;

        // ensure that we have IAC before SE or
        // wait for non-xopt subnegotiation SE
        if (At(m_cmdBuff, (m_cmdBuff.size() - 2)) != CMD_IAC) return;

        byte opt = At(m_cmdBuff, OptNo);
        QByteArray params = ExtractSbData();

        // m_logger.Add(m_cmdBuff, IsInput, TelnetDataLogger::SB);
        m_pOwner->GetLogger().Add(m_cmdBuff, IsInput, TelnetDataLogger::SB);

        if (opt == OPT_EXTOPL) ParseXoptSb(params);
        else m_pOwner->ParseSb(opt, params);

        m_cmdBuff.clear();
        m_currentState = InsideText;
    }

    // ------------------------------------------

    QByteArray TelnetDataCollector::ExtractSbData()
    {
        // use m_cmdBuff
        byte opt = At(m_cmdBuff, OptNo);
        byte firstBytes = ((opt == OPT_EXTOPL) || (opt == OPT_NAWS))
                           ? SbXoptBeginBytes : SbBeginBytes;
        int len = m_cmdBuff.size() - firstBytes - SbEndBytes;

         return m_cmdBuff.mid(firstBytes, len);
    }

    // ------------------------------------------------

    void TelnetDataCollector::ParseXoptSb(const QByteArray &data)
    {
        // IAC SB EXOPL WILL UNICODE IAC SE
        // or
        // IAC SB EXOPL SB CODEC "WINDOWS-1251" SE IAC SE
        
        byte cmd = At(data, XoptCmdNo); 
        byte opt = At(data, XoptOptNo);

        if (cmd == CMD_SB)
        {
            int begin = XoptOptNo + 1;
            int n = (data.size() - 1) - begin;
            m_pOwner->ParseSb(opt, data.mid(begin, n));
        }
        else 
        {
            m_pOwner->ParseCmd(cmd, opt);
        }
    }

    // ------------------------------------------------

    void TelnetDataCollector::ProcessPair(byte val)
    {
        // if text
        if ((m_pairBuff.isEmpty())
            && (val != CMD_IAC))
        {
            Append(m_textBuff, val);
            return;
        }

        // first byte == IAC
        Append(m_pairBuff, val);

        // we should know 2 bytes to recognized it's text or cmd 
        if (m_pairBuff.size() == 1) return;
        
        if (At(m_pairBuff, 1) == CMD_IAC) 
        {
            // if IAC doubled, it's text also
            Append(m_textBuff, At(m_pairBuff,1));
        }
        else
        {
            // if cmd
            Append(m_cmdBuff, CMD_IAC);
            //m_currentState = InsideIac;
            ProcessIac(At(m_pairBuff,1));
        }

        m_pairBuff.clear();
    }

} // namespace Telnet
