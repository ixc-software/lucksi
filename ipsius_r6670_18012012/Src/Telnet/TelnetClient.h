#ifndef __TELNETCLIENT__
#define __TELNETCLIENT__

#include "ITelnet.h"
#include "TelnetSession.h"
#include "TelnetUtils.h"
#include "TelnetDataLogger.h"

namespace Telnet
{
    using boost::shared_ptr;
    using Utils::SafeRef;

    // client side class for data excange through Telnet protocol
    class TelnetClient : public TelnetSession
    {
    public:
        class TelnetInitParams
        {
            QString m_ttype;
            Location m_location;
            word m_winWidth;
            word m_winHeight;
            string m_codecName;
    
        public:
            TelnetInitParams() ;
            void setTtype(const QString &ttype) { m_ttype = ttype; }
            QString getTtype() { return m_ttype; }
    
            void setLocation(const Location &loc) { m_location = loc; }
            Location getlocation() { return m_location; }
    
            void setWinSize(word width, word height) 
            {
                m_winWidth = width;
                m_winHeight = height;
            }
            word getWinWidth() { return m_winWidth; }
            word getWinHeight() { return m_winHeight; }
    
            void setCodecName(const string &name = "") { m_codecName = name; };
            string getCodecName() { return m_codecName; }
        };

    private:
        typedef TelnetClient T;
        typedef TelnetHelpers TH;

        SafeRef<ITelnetClientEvents> m_pOwner;
        TelnetOptionList m_optList;
        TelnetInitParams m_params;
        TelnetDataLogger m_logger;
        
        void ParseText(const QByteArray &data);                             // override TelnetSession
        void ParseCmd(byte cmd, byte opt);                                  // override
        void ParseSb(byte opt, const QByteArray &params);                   // override
        void StartSession();                                                // override
        void SendToOwnerDataInd(const QString &data);                       // override
        void SendToOwnerDiscInd();                                          // override
        void SendToOwnerSocketErrorInd(shared_ptr<iNet::SocketError> error); // override
        TelnetDataLogger& GetLogger();                                      // override
        
        // handlers
        byte CaseReplyCommand(byte inCmd, byte inOpt);
        //void CheckEstablish(byte cmd, byte opt);
        void CheckEstablish();
        void CheckChangeMode(byte opt, bool enable);

        void SendToSocketSb(const QByteArray &data);
        // data == params from ParseSb()
        void SendToSocketSbEnviron(byte environOpt, 
                                   const QByteArray &data);
        void SendToSocketSbTtype();
        // void SendToSocketSbTspeed();
        void SendToSocketSbNaws();
        void SendToSocketSbXdisploc();
        void SendToSocketSbXopt(SendingMode mode);

        // msg events from owner
        void OnChangeSendingModeReq(SendingMode mode);
        void OnSendWinSize(shared_ptr<WinSize> pSize);
        void OnSendTerminalType(shared_ptr<QString> pName);
        void OnSendXDisplayLocation(shared_ptr<Location> pLoc);
        
        // msg events to owner
        void OnTelnetDataInd(shared_ptr<QString> pData);
        void OnTelnetDiscInd();
        void OnTelnetEstablishInd();
        void OnTelnetChangeModeInd(SendingMode mode);
        void OnTelnetSocketErrorInd(shared_ptr<iNet::SocketError> error);
        
        // msg
        void OnSendCmd(boost::shared_ptr<CmdAndOpt> pData);
        void OnSend(boost::shared_ptr<QString> pData);

    public:
        TelnetClient(MsgThread &thread,
                     SafeRef<ITelnetClientEvents> owner,
                     const TelnetInitParams &params,
                     bool traceEnabled = false)
        : TelnetSession(thread/*, m_logger*/), 
            m_pOwner(owner), 
            m_params(params),
            m_logger("Telnet client log", traceEnabled)
        {
        }

        // messages
        void ChangeSendingModeReq(SendingMode mode, const string &codecName = "");
        void SendWinSize(word width, word height);
        void SendTerminalType(const QString &name);
        //void SendTerminalSpeed(int transmit, int reseive);
        void SendXDisplayLocation(const Location &loc);
        void Send(const QString &data);
        void SendCmd(byte cmd, byte opt);

        // option
        SendingMode getSendMode();
        TelnetDataLogger& getHistory() { return m_logger; }
    };


} // namespace Telnet

#endif
