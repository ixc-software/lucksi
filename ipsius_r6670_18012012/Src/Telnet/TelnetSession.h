#ifndef __TELNETSESSION__
#define __TELNETSESSION__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/HostInf.h"

#include "iNet/ITcpSocketToUser.h"
#include "iNet/ITcpSocket.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"

#include "ITelnet.h"
#include "TelnetDataCollector.h"
#include "TelnetOptionList.h"
#include "TelnetSockets.h"
#include "TelnetUtils.h"

namespace Telnet
{
    using iCore::MsgThread;
    using iCore::MsgObject;
    using boost::shared_ptr;
    using Utils::SafeRef;
    

    // base class for TelnetClient and TelnetServerSession
    // implement common features from both client and server proto sides
    class TelnetSession : public MsgObject, 
                          public iNet::ITcpSocketToUser,
                          public ITelnetDataCollectorEvents,
                          public boost::noncopyable
    {
        typedef TelnetSession T;
        typedef TelnetHelpers TH;

        shared_ptr<iNet::ITcpSocket> m_pSocket;

        // fields
        //TelnetDataLogger &m_logger;
        TelnetDataCollector m_collector;

    // implement iNet::ITcpSocketToUser
    private:
        void Connected(iNet::SocketId id);
        void ReceiveData(shared_ptr<iNet::SocketData> data);
        void Disconnected(boost::shared_ptr<iNet::SocketError> error);
        void SocketErrorOccur(shared_ptr<iNet::SocketError> error);

    private:
        // msg events
        void OnEstablishReq(boost::shared_ptr<Utils::HostInf> pHostInf);
        void OnDisconnectReq();
        void OnSendBytes(shared_ptr<QByteArray> pData);

    protected:
        enum States
        {
            ConnectedSt,      // socked connected
            EstablishedSt,    // can exchange data
            DisconnectedSt,   // socked disk
        };

    private:
        States m_state;
        void ChangeState (States state);

    // implement ITelnetDataCollectorEvents
    private:
        // collector <--> telnet
        virtual void ParseText(const QByteArray &data) = 0;
        virtual void ParseCmd(byte cmd, byte opt) = 0;
        virtual void ParseSb(byte opt, const QByteArray &params) = 0;
        virtual TelnetDataLogger& GetLogger() = 0;

    private:
        // handlers (telnet specific)
        virtual void StartSession() = 0; // for client: call when socket connect conf
                                         // for server do establish

        virtual void SendToOwnerDataInd(const QString &data) = 0;
        virtual void SendToOwnerDiscInd() = 0;
        virtual void SendToOwnerSocketErrorInd(shared_ptr<iNet::SocketError> error) = 0;
        
    protected:
        void SendToSocketCmd(byte cmd, byte opt);
        void SendToSocketCmd(byte cmd);
        void SendBytes(const QByteArray &data, bool logIt = true);

        // options
        void setState(States state) { m_state = state; }
        States getState() { return m_state; }
        
        static byte At(const QByteArray &array, size_t index);
        static void Append(QByteArray &array, byte val);
        
    public:

        ESS_TYPEDEF(SocketErrorInfo);

        TelnetSession(MsgThread &thread/*, TelnetDataLogger &logger*/)
        : MsgObject(thread), 
          m_pSocket(shared_ptr<iNet::ITcpSocket>(new TelnetSocket(thread, this))),
          /*m_logger(logger), */m_collector(this/*, logger*/), m_state(DisconnectedSt)
        {}

        TelnetSession(MsgThread &thread, 
                      shared_ptr<iNet::ITcpSocket> socket/*, 
                      TelnetDataLogger &logger*/)
        : MsgObject(thread), m_pSocket(socket), /*m_logger(logger), */m_collector(this/*, logger*/),
          m_state(ConnectedSt)
        {
            m_pSocket->LinkUserToSocket(this);
        }

        // messages
        void EstablishReq(const Utils::HostInf &host);
        void DisconnectReq();
    };

} // namesapce Telnet

#endif
