
#ifndef __ITCPSOCKETEMULINTERFACES__
#define __ITCPSOCKETEMULINTERFACES__

// ITcpSocketEmulInterfaces.h


namespace TestUtils
{
    // dummi socket error class
    class SocketError
    {
    public:
        SocketError() {}
        std::string ToString() { return "Socket error"; }
    };

    // ------------------------------------------------------
    // call-back interface to owner
    class ITcpSocketEmulToUser : public Utils::IBasicInterface
    {
    public:
        virtual void Connected() = 0;
        virtual void Disconnected(boost::shared_ptr<SocketError> error) = 0;
        virtual void ReceiveData(boost::shared_ptr<Utils::ManagedMemBlock> data) = 0;
    };

    // -----------------------------------------
    // test-socket interface
    class ITcpSocketEmul : public Utils::IBasicInterface
    {
    public:
        virtual void setOther(Utils::SafeRef<ITcpSocketEmul> pOther) = 0;
        virtual void Connected() = 0;
        virtual void OnReceiveData(boost::shared_ptr<Utils::ManagedMemBlock> pData) = 0;
        virtual void resetOther() = 0;
        virtual bool IsConnected() = 0;
        virtual const Utils::HostInf& PeerHostInf() const = 0;
        virtual const Utils::HostInf& LocalHostInf() const = 0;
        virtual void LinkUserToSocket(Utils::SafeRef<ITcpSocketEmulToUser> iTcpSocketToUser) = 0; 
        virtual void ConnectToHost(const Utils::HostInf &host) = 0;
        virtual void SendData(const void *pData, size_t size) = 0;
        virtual void DisconnectFromHost() = 0;
    };

    // -----------------------------------------
    // test-server socket interface
    class ITcpServerSocketEmul : public Utils::IBasicInterface
    {
    public:
        virtual void NewConnection(Utils::SafeRef<ITcpSocketEmul> pSender) = 0;
        virtual const Utils::HostInf &LocalInf() const = 0;
    };
    
} // namespace TestUtils

#endif
