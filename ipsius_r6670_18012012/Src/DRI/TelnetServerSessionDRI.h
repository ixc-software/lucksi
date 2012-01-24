#ifndef __TELNETSERVERSESSIONDRI__
#define __TELNETSERVERSESSIONDRI__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "Utils/IntToString.h"
#include "Utils/WeakRef.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"

#include "Domain/DomainClass.h"

#include "Telnet/TelnetServerSession.h"

#include "iLog/LogWrapper.h"
#include "iLog/LogManager.h"

#include "SessionDRI.h"
#include "ParsedDRICmd.h"
#include "DRICommandStreamParser.h"
#include "ITelnetDRI.h"

namespace iReg
{
    class IDriSessionLog;
}


namespace DRI
{
    using boost::shared_ptr;
    using boost::scoped_ptr;
    using Platform::word;
    using Telnet::SendingMode;
    using iLogCommon::LogString;
    using iLogW::EndRecord;

    class TelnetServerSessionDRI;

    class DriHintDb;

    // --------------------------------------------------------------------

    class ITelnetBuffOwner : public Utils::IBasicInterface
    {
    public:
        virtual Telnet::TelnetServerSession& TelnetSession() = 0;
        virtual DriHintDb& HintDb() = 0;

        virtual void CloseTelnetSession() = 0;
        virtual void EnterLine(QString line) = 0;
    };

    // --------------------------------------------------------------------

    // DRI session over Telnet
    class TelnetServerSessionDRI :
        // public virtual Utils::SafeRefServer,
        public iCore::MsgObject, // for timer
        public Telnet::ITelnetServerSessionEvents,
        public ISessionDriOwner,
        public ITelnetBuffOwner,
        public ITelnetSessionDRIToOwner, 
        boost::noncopyable
    {
        class InputLine;

        class SessionStats
        {
            int m_completeCmdCount;
            // ...
        public:
            SessionStats();

            void AddCmd();
            QString ToString() const;
        };

        enum State
        {
            stNone,
            stInputLogin,
            stInputPassword,
            stLogged,
            stClosed,
        };

        ITelnetSessionDRIToServer &m_owner;
		Domain::IDomain &m_domain;

		// log
		scoped_ptr<iLogW::LogSession> m_log;
		iLogW::LogRecordTag m_tagInfo;
        int m_sessionID;
        
        scoped_ptr<Telnet::TelnetServerSession> m_session;
        scoped_ptr<DRICommandStreamParser> m_driParser;

        SessionDRI* m_pSessionDRI;  // ExecutiveDRI is owner of this object 
        Utils::WeakRefHost m_host;
        
        State m_state;
        QString m_userName;
        scoped_ptr<InputLine> m_line;        
        bool m_waitForCompletion;  // transaction executed
        int m_incompletedCmd;

        // other
        SessionStats m_stats;
        iCore::MsgTimer m_suspendExitTimer;
        QString m_startupInfo;  // local/remote peer, start time 
        Utils::HostInf m_remotePeer;
        boost::scoped_ptr<iReg::IDriSessionLog> m_regLog;
        
        void InitLog(iLogW::ILogSessionCreator &logCreator);
        void EnterInputLoginState(QString msg = "");
        void EnterInputPasswordState();
        void EnterLoggedState();

        void BeginInputCommand();

        void ExecuteCommand(const QString &line);
        void TryToExecute();
        void PrintCmdData(QString data, bool lineFeed);
        void OnExit(iCore::MsgTimer *pT);
        void Log(const QString &data);

    // ITelnetBuffOwner impl
    private:

        Telnet::TelnetServerSession& TelnetSession() { return *m_session; }
        DriHintDb& HintDb();
        void CloseTelnetSession();
        void EnterLine(QString line);

    // ISessionDriOwner impl
    private:

        void CommandQueueEmpty();
        void TransactionBeginExec();
        void TransactionEndExec();
        void SessionExitRequest();
        void CommandStarted(shared_ptr<CommandDRI> cmd);
        void CommandOutput(CommandDRI::CommandType type, QString data, bool lineFeed);
        void CommandCompleted(shared_ptr<CommandDRI> cmd);

        QString GetFullSessionInfo() const
        {
            return "Telnet " + m_startupInfo;
        }

    // ITelnetServerSessionEvents impl
    private:

        void TelnetDataInd(const QString &data);

        void TelnetDiscInd()
        {
            Log("DISC");
            CloseTelnetSession();
        }

        void TelnetEstablishInd();

        void TelnetChangeWinSizeInd(word width, word height)
        {
            Log(QString("WINSIZE %1x%2").arg(width).arg(height));
        }

        void TelnetChangeSendingModeInd(SendingMode mode)
        {
            Log(QString("MODE %1").arg(mode));
        }

        void TelnetSocketErrorInd(boost::shared_ptr<iNet::SocketError> error) 
        {
            Log(error->getErrorString());
            CloseTelnetSession();
        }

    // ITelnetSessionDRIToOwner
    private:

        bool IsSame(const TelnetServerSessionDRI *pSession) const { return pSession == this; }

        QString GetStats() const { return m_stats.ToString(); }  

        QString GetInfo() const { return m_startupInfo; }

        void Close() { CloseTelnetSession(); }        

        QString UserName() const { return m_userName; }

    public:

        TelnetServerSessionDRI( Domain::IDomain &domain, 
			ITelnetSessionDRIToServer &owner, 
			iLogW::ILogSessionCreator &logCreator,
			shared_ptr<iNet::ITcpSocket> socket);

        ~TelnetServerSessionDRI();
    };
}  // namespace DRI

#endif
