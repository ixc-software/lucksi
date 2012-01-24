#ifndef __TELNETSERVERSESSION__
#define __TELNETSERVERSESSION__

#include "TelnetSession.h"
#include "ITelnet.h"
#include "TelnetUtils.h"
#include "TelnetDataLogger.h"

#include "iCore/MsgThread.h"
#include "Utils/SafeRef.h"

namespace Telnet
{
    using boost::shared_ptr;
    using Utils::SafeRef;

    // server side class for data excange through Telnet protocol
    // linked with one client
    class TelnetServerSession : public TelnetSession
    {
        typedef TelnetServerSession T;
        typedef TelnetHelpers TH;

        SafeRef<ITelnetServerSessionEvents> m_pOwner;
        TelnetOptionList m_optList;
        string m_codecName;
        std::vector<string> m_varList; // name of envir. vars
        std::vector<string> m_userVarList;
        TelnetDataLogger m_logger;

        // win terminal sends enter as 0x0d 0x0a
        // lin terminal sends enter as 0x0d 0x00
        // if on DO ECHO we received:
        //  WONT ECHO --> lin terminal,  
        //  WILL ECHO --> win terminal
        bool m_connectedWithLinuxClient;
        
        void ParseCmd(byte cmd, byte opt);                                  // override TelnetSession
        void ParseSb(byte opt, const QByteArray &params);                   // override
        void ParseText(const QByteArray &data);                             // override
        void SendToOwnerDataInd(const QString &data);                       // override
        void SendToOwnerDiscInd();                                          // override
        void SendToOwnerSocketErrorInd(shared_ptr<iNet::SocketError> error); // override
        void StartSession();                                                // override
        TelnetDataLogger& GetLogger();                                      // override
        
    private:
        // handlers
        byte CaseReplyCommand(byte inCmd, byte inOpt);
        //void CheckEstablish(byte cmd, byte opt);
        void CheckEstablish(byte cmd);
        void CheckChangeMode(byte opt, bool enable);
        void SendToSocketSb(byte opt);

        // SB parcer specializations
        void ParseSbTtype(const QByteArray &params);
        void ParseSbNaws(const QByteArray &params);
        void ParseSbEnviron(const QByteArray &params);
        void ParseSbXdisploc(const QByteArray &params);

        // msg events to owner
        void OnTelnetDataInd(shared_ptr<QString> pData);
        void OnTelnetDiscInd();
        void OnTelnetEstablishInd();
        void OnTelnetChangeWinSizeInd(shared_ptr<WinSize> pSize);
        void OnTelnetChangeSendingModeInd(SendingMode mode);
        void OnTelnetSocketErrorInd(shared_ptr<iNet::SocketError> error);

        // msg
        void OnSendCmd(boost::shared_ptr<CmdAndOpt> pData);
        void OnSend(boost::shared_ptr<QString> pData);
        
    public:
        TelnetServerSession(MsgThread &thread,
                            shared_ptr<iNet::ITcpSocket> socket,
                            SafeRef<ITelnetServerSessionEvents> owner,
                            bool traceEnabled = false)
        : TelnetSession(thread, socket/*, m_logger*/), 
          m_pOwner(owner), 
          m_logger("Telnet server session log", traceEnabled),
          m_connectedWithLinuxClient(true)
        {
            // cause create AFTER socket connected
            StartSession();
        }

        void Send(const QString &data);
        void SendCmd(byte cmd, byte opt);
        
        // option
        SendingMode getSendMode() { return m_optList.getOwnerSendingMode(); }

        bool ConnectedWithLinuxClient() const { return m_connectedWithLinuxClient; }

    
        TelnetDataLogger &getHistory() { return m_logger; }
    };
} // namespace Telnet

#endif
