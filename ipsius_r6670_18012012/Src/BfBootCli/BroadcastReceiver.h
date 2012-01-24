#ifndef BROADCASTRECEIVER_H
#define BROADCASTRECEIVER_H

#include "iNet/IUdpSocketToUser.h"
#include "iNet/MsgUdpSocket.h"
#include "BfBootCore/BroadcastMsg.h"

#include "BfBootCore/UdpCommandsOfAutoSender.h"

namespace BfBootCli
{
    class IBroadcastReceiverToOwner : Utils::IBasicInterface
    {
    public:
        virtual void MsgReceived(const BfBootCore::BroadcastMsg& msg) = 0;
    };

    class BroadcastReceiver : 
        public virtual Utils::SafeRefServer, 
        public iNet::IUdpSocketToUser
    {
        IBroadcastReceiverToOwner& m_callback;
        iNet::MsgUdpSocket m_socket;
        int m_trashCounter;

    // IUdpSocketToUser impl
    private:
        void ReceiveData(boost::shared_ptr<iNet::SocketData> data);
        void SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error);

    public:

        ESS_TYPEDEF(BindError);

        BroadcastReceiver(iCore::MsgThread& thread, 
            IBroadcastReceiverToOwner& callback, int listenPort);  // can throw;
        void ResetDevice(const BfBootCore::BroadcastMsg& msg);

        void DirectRequest(const Utils::HostInf& addr);

        int WrongMsgCount() const;
    };
} // namespace BfBootCli

#endif
