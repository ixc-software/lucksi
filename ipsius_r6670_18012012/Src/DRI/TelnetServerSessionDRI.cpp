#include "stdafx.h"

#include "iReg/ISysReg.h"

#include "TelnetServerSessionDRI.h"
#include "DriHintDb.h"
#include "TelnetKeys.h"
#include "CommandDRI.h"
#include "DriSessionList.h"

// ----------------------------------------

using namespace TelnetKey;

namespace DRI
{

    class TelnetCmdHistory : boost::noncopyable
    {
        enum 
        {
            CEnableHistortyLimit = true,

            CMaxPrevHistory = 64,
            CCutHistoryTo   = 32,
        };

        BOOST_STATIC_ASSERT(CMaxPrevHistory > CCutHistoryTo);

        std::vector<QString> m_buff;
        int m_currPos;

        // [from; to)
        bool RemoveDublicate(int from, int to, const QString &val)
        {
            ESS_ASSERT(from <= to);

            for(int i = from; i < to; ++i)
            {
                if (m_buff.at(i) == val)
                {
                    m_buff.erase(m_buff.begin() + i);
                    return true;
                }
            }

            return false;
        }

        void RemoveDublicates(const QString &val)
        {
            // beyond m_currPos
            if (m_currPos < m_buff.size())
            {
                if (RemoveDublicate(m_currPos, m_buff.size(), val)) return;
            }

            // before m_currPos
            if (m_currPos > 0)
            {
                if (RemoveDublicate(0, m_currPos, val))
                {
                    --m_currPos;  // fix, 'couse single item before m_currPos removed                    
                }
            }
        }

        void CutHistory()
        {
            // [0]...[above][pos][below]...[n]
            int maxLeftSize = m_buff.size() - CCutHistoryTo;


            int abovePos = m_currPos; // left from above from this pos
            int belowPos = m_currPos; // left from below up to this pos
            int count = 1;
            while (count < maxLeftSize)
            {
                if ((abovePos == 0) && (count == 0)) ++count;

                if (abovePos > 0)
                {
                    ++count;
                    --abovePos;
                }

                if (count == maxLeftSize) break;

                if (belowPos < m_buff.size())
                {
                    ++count;
                    ++belowPos;
                }
            }

            m_buff.erase(m_buff.begin(), m_buff.begin() + abovePos);

            belowPos -= abovePos;
            int toDelete = m_buff.size() - CCutHistoryTo;
            m_buff.erase(m_buff.begin() + belowPos, m_buff.begin() + belowPos + toDelete);

            ESS_ASSERT(m_buff.size() == CCutHistoryTo);

            m_currPos -= abovePos;
            ESS_ASSERT(m_currPos <= m_buff.size());
        }

    public:

        TelnetCmdHistory()
        {
            m_currPos = 0;
        }

        void Insert(const QString &cmd)
        {
            if (cmd.isEmpty()) return;

            ESS_ASSERT(m_currPos <= m_buff.size());

            /*
            // remove dublicate only in prev item
            if (m_currPos > 0)
            {
                // don't add dublicates
                if (m_buff.at(m_currPos - 1) == cmd) return;
            } */

            RemoveDublicates(cmd);

            m_buff.insert(m_buff.begin() + m_currPos, cmd);
            m_currPos++;

            /*if ((m_currPos > CMaxPrevHistory) && CEnableHistortyLimit)
            {
                int deleteCount = m_currPos - CCutHistoryTo;
                m_buff.erase(m_buff.begin() + CCutHistoryTo, m_buff.begin() + m_currPos);
                m_currPos = CCutHistoryTo;
            }*/

            if ((m_buff.size() > CMaxPrevHistory) && CEnableHistortyLimit)
            {
                CutHistory();
            }
        }

        bool Prev(QString &cmd)
        {
            if (m_currPos == 0) return false;

            m_currPos--;
            cmd = m_buff.at(m_currPos);

            return true;
        }

        bool Next(QString &cmd)
        {
            if (m_currPos >= m_buff.size()) return false;

            m_currPos++;

            if (m_currPos >= m_buff.size())
            {
                cmd = "";
            }
            else
            {
                cmd = m_buff.at(m_currPos);
            }

            return true;
        }

    };

}  // namespace DRI



// ----------------------------------------

namespace DRI
{
    using boost::scoped_ptr;

    // implement user input line for telnet
    class TelnetServerSessionDRI::InputLine
    {
        ITelnetBuffOwner &m_owner;
        iLogW::LogSession &m_log;
        Telnet::TelnetServerSession &m_session;
        TelnetKey::Map m_keyMap;

        // log tags
        iLogW::LogRecordTag m_tagInRaw;
        iLogW::LogRecordTag m_tagInKeys;
        iLogW::LogRecordTag m_tagOut;

        // current input line data
        QString m_currBuff;
        QString m_currLine;
        int m_cursorPos;  // inside m_currLine
        bool m_maskedMode;
        bool m_logged;

        // history
        TelnetCmdHistory m_history;

        // hint stuff
        scoped_ptr<DriHintResult> m_hint;
        QString m_currHintText;

        static bool PrintableChar(QChar c)
        {
            return ((c >= ' ') && (c <= '~'));
        }

        static QString TelnetData(const QString &data)
        {
            QString res;

            // hex part
            for(int i = 0; i < data.size(); ++i)
            {
                int c = data.at(i).unicode();
                std::string v = Utils::IntToHexString(c, false, 1);
                res += v.c_str();
                res += " ";
            }

            // ascii part
            res += " | ";

            for(int i = 0; i < data.size(); ++i)
            {
                QChar c = data.at(i);
                res += PrintableChar(c) ? c : '?';
            }

            return res;
        }

        void Send(const QString &s)
        {
            if (s.isEmpty()) return;

            if (m_log.LogActive(m_tagOut))
            {
                QString dts = s.left(64);                
                m_log << m_tagOut << TelnetData(dts) 
                    << ((dts.size() != s.size()) ? " (...)" : "") << EndRecord;
            }

            m_session.Send(s);
        }

        void SendEnter()
        {
            Send( m_keyMap.Resolve(TelnetKey::Map::Enter) );
        }

        /*
        static QString KeyResolve(Map::KeyID key)
        {
            return m_keyMap.Resolve(); //  Map::Inst().Resolve(key);
        }

        void SendKey(Map::KeyID key)
        {
            Send( KeyResolve(key) );
        } */

        bool VerifyPosInLine(int pos)
        {
            return (pos >= 0) && (pos <= m_currLine.size()); 
        }

        // if diff < 0 -- move left, else right
        void MoveCursor(int diff)
        {
            using TelnetKey::Map;

            if (diff == 0) return;

            m_cursorPos += diff;  // update own position
            ESS_ASSERT( VerifyPosInLine(m_cursorPos) );

            QString moveSeq = m_keyMap.Resolve( (diff < 0) ? Map::Left : Map::Right );
            QString moveData;                

            int count = std::abs(diff);
            for(int i = 0; i < count; ++i) moveData += moveSeq;

            Send(moveData);
        }

        // update line from fromPos, cursor position after = cursor + cursorDiff
        void UpdateLine(int fromPos, int cursorDiff)
        {
            ESS_ASSERT( VerifyPosInLine(fromPos) );

            int posAtEnd = m_cursorPos + cursorDiff;

            // move to fromPos
            {
                MoveCursor(fromPos - m_cursorPos);
            }

            // send chars
            {
                QString sendChars = m_currLine.mid(fromPos);
                if (m_maskedMode) sendChars = QString(sendChars.size(), QChar('*'));
                sendChars += " "; // extra space for remove deleted chars

                Send(sendChars); 
                m_cursorPos += sendChars.size();
            }

            // move remotePos to required m_cursorPos
            MoveCursor(posAtEnd - m_cursorPos);
        }

        void BackspaceAction()
        {
            if (m_cursorPos <= 0) return;

            m_currLine = m_currLine.remove(m_cursorPos - 1, 1);
            UpdateLine(m_cursorPos - 1, -1);
        }

        void PrevHistory()
        {
            if (!m_logged) return;

            QString cmd;
            bool res = m_history.Prev(cmd);
            if (!res) return;

            InputReplace(cmd);
        }

        void NextHistory()
        {
            if (!m_logged) return;

            QString cmd;
            bool res = m_history.Next(cmd);
            if (!res) return;

            InputReplace(cmd);
        }


        void ProcessSpecialKey(TelnetKey::Map::KeyID key)
        {
            // chars remove
            if (key == Map::BS)
            {
                if (m_hint != 0)
                {
                    LeaveHintMode(false);
                    return;
                }

                BackspaceAction();
            }

            if (key == Map::Del)
            {
                if (m_hint != 0)
                {
                    LeaveHintMode(false);
                    return;
                }

                if (m_cursorPos < m_currLine.size())
                {
                    m_currLine = m_currLine.remove(m_cursorPos, 1);
                    UpdateLine(m_cursorPos, 0);
                }
            }

            // cursor movement
            if (key == Map::Home)
            {
                LeaveHintMode(true);
                MoveCursor(-m_cursorPos);
            }

            if (key == Map::End)
            {
                LeaveHintMode(true);
                MoveCursor(m_currLine.size() - m_cursorPos);
            }

            if (key == Map::Left)
            {
                LeaveHintMode(true);

                if (m_cursorPos > 0)
                {
                    MoveCursor(-1);
                }
            }

            if (key == Map::Right)
            {
                LeaveHintMode(true);

                if (m_cursorPos < m_currLine.size())
                {
                    MoveCursor(+1);
                }
            }

            // history
            if (key == Map::Up)
            {
                PrevHistory();
            }

            if (key == Map::Down)
            {
                NextHistory();
            }

            // other
            if (key == Map::Enter)
            {
                LeaveHintMode(true);

                if ( (m_logged) && (!m_currLine.isEmpty()) )
                {
                    m_history.Insert(m_currLine);
                }

                m_owner.EnterLine(m_currLine);
            }

            if (key == Map::Escape)
            {
                m_owner.CloseTelnetSession();
            }

            if (key == Map::Tab)
            {
                ProcessTabKey();
            }

        }

        // cursor must be in end of line
        // stop chars in CStopChars
        QString GetTextForHint() const
        {
            static const QString CStopChars(" .\"");

            if (m_currLine.isEmpty()) return "";
            if (m_cursorPos != m_currLine.size()) return "";

            for(int i = m_currLine.size() - 1; i >= 0; --i)
            {
                if (CStopChars.indexOf( m_currLine.at(i) ) >= 0) 
                {
                    if (i == (m_currLine.size() - 1)) return "";
                    return m_currLine.mid(i + 1);
                }
            }

            return m_currLine;
        }

        void LeaveHintMode(bool withHintTextAccept)
        {
            if (m_hint == 0) return;

            m_hint.reset(0);

            if (withHintTextAccept)
            {
                // nothing - ?
            }
            else
            {
                RemoveHint();
            }

            m_currHintText = ""; 
        }

        // remove m_currHintText from screen and m_currLine
        void RemoveHint()
        {
            int removeCount = m_currHintText.size();

            if (removeCount == 0) return;

            // 'backspace' to delete from screen and m_currLine
            for(int i = 0; i < removeCount; ++i) BackspaceAction();
        }

        void SetHintText(const QString &s)
        {
            RemoveHint();

            m_currHintText = s;

            // add hint (to screen and m_currLine)
            for(int i = 0; i < s.size(); ++i) 
            {
                InsertChar( s.at(i) );
            }
        }

        void ProcessTabKey()
        {
            if (!m_logged) return;

            if (m_hint == 0)
            {
                m_currHintText = "";  // fix

                QString textForHint = GetTextForHint();
                if (textForHint.isEmpty()) return;

                DriHintResult *pHint = m_owner.HintDb().GetHint(textForHint);
                if (pHint == 0) return;  // no hint

                // enter hint mode
                m_hint.reset(pHint);

                // show first hint
                SetHintText( m_hint->Curr() );

                return;
            }

            // in hint mode -- show next hint
            {
                m_hint->Next();
                QString hint = m_hint->Curr();

                if (hint.isEmpty())  // hint completed
                {
                    LeaveHintMode(false);
                    return;
                }

                SetHintText(hint);
            }
        }

        void InsertChar(QChar c)
        {
            ESS_ASSERT(m_cursorPos <= m_currLine.size());

            // buff
            m_currLine.insert(m_cursorPos, c);

            UpdateLine(m_cursorPos, +1);
        }

        // m_currBuff -> m_currLine
        void ProcessCurrBuff()
        {
            while(!m_currBuff.isEmpty())
            {
                // special key - ?
                Map::KeyID keyID;
                m_currBuff = m_keyMap.Peek(m_currBuff, keyID);
                if (keyID != Map::None)
                {
                    if (m_log.LogActive(m_tagInKeys)) 
                    {
                        m_log << m_tagInKeys << "[" << m_keyMap.KeyAsString(keyID) << "]" << EndRecord;
                    }
                    ProcessSpecialKey(keyID);
                    continue;
                }

                // ASCII
                QChar c = m_currBuff.at(0);
                if (PrintableChar(c))
                {
                    if (m_log.LogActive(m_tagInKeys)) 
                    {
                        m_log << m_tagInKeys << QString(c) << EndRecord;
                    }

                    if (m_hint != 0) LeaveHintMode(true);
                    InsertChar(c);
                    m_currBuff = m_currBuff.mid(1);
                    continue;
                }

                // stucked or bad code - just try to skip unknown char
                if ((m_currBuff.size() > 8) || (c.unicode() > 0x7f))
                {
                    if (m_log.LogActive(m_tagInKeys)) 
                    {
                        m_log << m_tagInKeys << "{" << c.unicode() << "}" << EndRecord;
                    }

                    m_currBuff = m_currBuff.mid(1);
                    continue;
                }

                break;
            }

        }

        // can write thru BackspaceAction()
        void ClearAllLine()
        {
            // move to begin
            MoveCursor(-m_cursorPos);

            // erase all
            m_currLine = QString(m_currLine.size(), QChar(' '));
            UpdateLine(0, 0);

            // clear local buff
            ESS_ASSERT(m_cursorPos == 0);
            m_currLine = "";
        }

        // for history -- full line change
        void InputReplace(const QString &data)
        {
            LeaveHintMode(false);

            ClearAllLine();

            // just send data
            m_currLine = data;
            UpdateLine(0, m_currLine.size());
        }

    public:

        InputLine(ITelnetBuffOwner &owner, iLogW::LogSession &log, bool linuxMode) : 
              m_owner(owner),
              m_log(log),
              m_session(owner.TelnetSession()),
              m_keyMap(linuxMode)
          {
              m_cursorPos = 0;
              m_logged = false;

              // log
              m_tagInRaw      = m_log.RegisterRecordKindStr("InRaw");
              m_tagInKeys     = m_log.RegisterRecordKindStr("InKeys");
              m_tagOut        = m_log.RegisterRecordKindStr("Out");
          }

          void Logged()
          {
              ESS_ASSERT( !m_logged );

              m_logged = true;
          }

          void PutData(const QString &data)
          {                
              if (m_log.LogActive(m_tagInRaw))
              {
                  m_log << m_tagInRaw << TelnetData(data) << EndRecord;
              } 

              // process data
              m_currBuff += data;
              ProcessCurrBuff();

              // direct echo -- debug
              // m_session.Send(data);
          }

          void BeginInput(QString inputPrefix, bool maskInput = false)
          {
              // if (!result.isEmpty()) SendKey(Map::Enter);
              // Send(result);

              LeaveHintMode(false);

              SendEnter();
              Send(inputPrefix);

              m_cursorPos = 0;
              m_currLine = "";
              m_currBuff = "";

              m_maskedMode = maskInput;
          }

          void Print(const QString &data = "", bool withLf = true)
          {
              Send(data);
              if (withLf) SendEnter();
          }


    };

}  // namespace DRI


// ----------------------------------------

namespace DRI
{

    TelnetServerSessionDRI::TelnetServerSessionDRI( Domain::IDomain &domain,
		ITelnetSessionDRIToServer &owner,
		iLogW::ILogSessionCreator &logCreator,
        shared_ptr<iNet::ITcpSocket> socket) :
        iCore::MsgObject(domain.getDomain().getMsgThread()),
        m_owner(owner),
		m_domain(domain),
        m_suspendExitTimer(this, &TelnetServerSessionDRI::OnExit),
        m_pSessionDRI(0)
    {
        m_sessionID = m_domain.getDomain().ExeDRI().AllocSessionNumber();
        InitLog(logCreator);

        // startup info
        {
            m_startupInfo = QString("LocalPort %1; Remote %2; Started %3; ID %4")
                .arg( socket->LocalHostInf().Port() )
                .arg( socket->PeerHostInf().ToString().c_str() )
                .arg( QDateTime::currentDateTime().toString("hh:mm:ss dd.MM") )
                .arg( m_sessionID );
        }

        m_remotePeer = socket->PeerHostInf();
        ESS_ASSERT( !m_remotePeer.Empty() );
       
        // Telnet session
        m_session.reset(
            new Telnet::TelnetServerSession(m_domain.getDomain().getMsgThread(), socket, this));
                
        // parser
        m_driParser.reset(new DRICommandStreamParser);
        
        // Telnet buff
        // m_line.reset( new InputLine(*this) );

        m_state = stNone;
    }

    // -----------------------------------------------------
    
    void TelnetServerSessionDRI::InitLog(iLogW::ILogSessionCreator &logCreator)
    {
        // this log is for debug only
        bool logActive = true;
                
        LogString name = QString("Telnet%1").arg(m_sessionID).toStdWString();

        m_log.reset( logCreator.CreateSessionExt(name, logActive) );
        m_tagInfo = m_log->RegisterRecordKindStr("Info");
        
        // started
        Log("Started!");
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::TelnetEstablishInd()
    {
        bool linuxMode = m_session->ConnectedWithLinuxClient();

        Log(QString("ESTABLISH, Linux mode %1").arg(linuxMode));

        ESS_ASSERT(m_state == stNone);
        ESS_ASSERT(m_line == 0);

        m_line.reset( new InputLine(*this, *m_log, linuxMode) );

        EnterInputLoginState();
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::TelnetDataInd( const QString &data )
    {
        // hack: sometimes this strange thing happends
        // ignore it; TODO fix on Telnet/socket level - ?!
        if (m_line == 0)   
        {
            return;
        }

        m_line->PutData(data);
    }

    // -----------------------------------------------------

    TelnetServerSessionDRI::~TelnetServerSessionDRI()
    {
        if (m_pSessionDRI != 0) m_pSessionDRI->TryFinalize(false);
        m_pSessionDRI = 0;

        m_host.Clear();
        
        m_session.reset();
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::EnterInputLoginState(QString msg)
    {
        if (!msg.isEmpty())
        {
            m_line->Print();  // lf
            m_line->Print(msg, false);
        }

        m_state = stInputLogin;
        m_line->BeginInput("login: ");
    }

    void TelnetServerSessionDRI::EnterInputPasswordState()
    {
        m_state = stInputPassword;
        m_line->BeginInput(QString("password for %1: ").arg(m_userName), true);
    }

    void TelnetServerSessionDRI::EnterLoggedState()
    {
        m_waitForCompletion = false;
        m_incompletedCmd = 0;

        // create DRI session
        {
            ExecutiveDRI &exe = m_domain.getDomain().ExeDRI();
            m_pSessionDRI = &exe.GetNewSession(m_userName, m_host.Create<ISessionDriOwner&>(*this), m_sessionID);
        }

        // print welcome home
        {
            QString domain = m_domain.getDomain().getName().Name();
            QString welcome 
                = QString("Welcome to domain %1. Type \"MetaTypeInfo\" to begin work. Hit ESC for exit, TAB for hint.").arg(domain);
            m_line->Print(); // lf
            m_line->Print(welcome);
            QString started = QString("Session started %1")
                                .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy 'at' hh:mm:ss"));
            m_line->Print(started);

            QString active = m_domain.getDomain().ExeDRI().SessionList().GetActiveSessionsInfo();
            if ( active.length() ) 
            {
                m_line->Print();
                m_line->Print("DRI blocked! " + active);
            }
        }

        m_regLog.reset( m_domain.getDomain().SysReg().CreateDriSessionLog(m_userName) );

        m_state = stLogged;
        m_line->Logged();
        BeginInputCommand();
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::EnterLine( QString line )
    {
        if (m_state == stInputLogin)
        {
            if (line.isEmpty()) EnterInputLoginState("Bad login!");
            else
            {
                m_userName = line;
                EnterInputPasswordState();
            }

            return;
        }

        if (m_state == stInputPassword)
        {
            QString authError;
            bool res = m_domain.getDomain().Authorize(m_userName, line, m_remotePeer, authError);
            if (res) EnterLoggedState();
                else EnterInputLoginState("ERROR: " + authError);

            return;
        }

        if (m_state == stLogged)
        {
            if (m_waitForCompletion)
            {
                // nothing - ?
            }
            else
            {
                ExecuteCommand(line);
            }

            return;
        }

        ESS_HALT("");
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::BeginInputCommand()
    {
        ESS_ASSERT(m_pSessionDRI != 0);

        // make prefix
        QString domainName = m_domain.getDomain().getName().Name();
        QString prefix = QString("%1:%2> ").arg(domainName)
                                           .arg(m_pSessionDRI->CurrentObject());

        // if (!result.isEmpty()) m_line->Print(result);
        m_line->BeginInput(prefix);
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::ExecuteCommand( const QString &line )
    {
        m_line->Print(); // send LF

        // try parse
        try
        {
            m_driParser->AddData(line + "\n");
        }
        catch(const ParsedDRICmd::InvalidCommandSyntax &e)
        {
            m_line->Print( e.getTextMessage().c_str() );
            BeginInputCommand();
            return;
        }
        
        TryToExecute();

        if (!m_waitForCompletion) BeginInputCommand();
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::TryToExecute()
    {
        ESS_ASSERT(m_pSessionDRI != 0);
        ESS_ASSERT(!m_waitForCompletion);

        while(m_driParser->HasParsedCmd())
        {
            shared_ptr<ParsedDRICmd> cmd = m_driParser->GetParsedCmd();

            bool transactionStarted = m_pSessionDRI->AddCommand(cmd);

            if (transactionStarted)
            {
                m_waitForCompletion = true;
                break;
            }
        }

        QString incompleteCmd(m_driParser->ResetIncompleteCmd().trimmed());
        
        if (incompleteCmd.isEmpty()) return;

        QString err = QString("Incomplete command '%1'\n"
                              "%2ERROR DRI::CommandDriPrepareError\n")
                              .arg(incompleteCmd).arg(CommandDRI::ServiceLinePrefix());
        
        PrintCmdData(err, true);
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::CommandCompleted( shared_ptr<CommandDRI> cmd )
    {
        ESS_ASSERT(m_pSessionDRI != 0);

        if (cmd->IsService()) return; // do nothing
        
        bool withOutput = (cmd->Result()->OK())? false : true;
        PrintCmdData(cmd->ResultToString(withOutput), true);

        // add to registration
        if (m_regLog != 0)
        {
            QString cmdBody = cmd->GetParsedCmd()->OriginalCmd();
            QString cmdResult = cmd->Result()->OutputAsString();

            QString s = QString("> %1\n%2\n").arg(cmdBody).arg(cmdResult);

            m_regLog->Add(s);
        }

        // add to stats
        m_stats.AddCmd();
        
        if (!cmd->IsScripted()) return;

        // exit script on first error
        if (cmd->Result()->OK()) return;

        // reset
        m_pSessionDRI->DropAllCommands();
        m_driParser->ResetIncompleteCmd();
        m_incompletedCmd = 0;
        CommandQueueEmpty();
    }

    void TelnetServerSessionDRI::TransactionBeginExec()
    {
        // ...
    }

    void TelnetServerSessionDRI::TransactionEndExec()
    {
        // ...
    }

    void TelnetServerSessionDRI::CommandQueueEmpty()
    {
        ESS_ASSERT(m_waitForCompletion);

        m_waitForCompletion = false;

        BeginInputCommand();
        TryToExecute(); 
    }

    // -----------------------------------------------------

    DriHintDb& TelnetServerSessionDRI::HintDb()
    {
        return m_domain.getDomain().ExeDRI().HintDB();
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::SessionExitRequest()
    {
        CloseTelnetSession();
    }

    // -----------------------------------------------------
    // TelnetServerSessionDRI::SessionStats impl
    
    TelnetServerSessionDRI::SessionStats::SessionStats() : 
        m_completeCmdCount(0)
    {
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::SessionStats::AddCmd()
    {
        ++m_completeCmdCount; 
    }

    // -----------------------------------------------------
            
    QString TelnetServerSessionDRI::SessionStats::ToString() const
    {
        return QString("[Completed commands: %1]").arg(m_completeCmdCount);
    }
            
    // -----------------------------------------------------

    void TelnetServerSessionDRI::PrintCmdData(QString data, bool lineFeed)
    {
        data.replace("\n", "\x0d\x0a");
        m_line->Print(data, lineFeed);
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::CommandStarted(shared_ptr<CommandDRI> cmd)
    {
        if (!cmd->IsScripted()) return;

        PrintCmdData(cmd->CommandToString(">"), true);
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::CommandOutput(CommandDRI::CommandType type, 
                                               QString data, 
                                               bool lineFeed)
    {
        if (type == CommandDRI::ctService) return;
        
        PrintCmdData(data, lineFeed);
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::OnExit(iCore::MsgTimer *pT)
    {
        m_owner.Unregister(this);
    }

    // -----------------------------------------------------

    void TelnetServerSessionDRI::CloseTelnetSession()
    {
        if (m_state == stClosed) return;

        m_state = stClosed;
        Log("Session closed");

        // wait: need time to print all commands on screen
        const int waitToExitMs = 100;
        m_suspendExitTimer.Start(waitToExitMs, false);
    }
    
    // -----------------------------------------------------

    void TelnetServerSessionDRI::Log(const QString &data)
    {
        if (!m_log->LogActive(m_tagInfo)) return;

        *m_log << m_tagInfo << data << EndRecord;
    }
    
    
}  // namespace DRI


