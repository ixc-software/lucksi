#ifndef __ITELNET__
#define __ITELNET__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Platform/PlatformTypes.h"
#include "iNet/SocketError.h"
#include "iNet/ITcpSocket.h"

#include "TelnetSockets.h"
#include "TelnetUtils.h"
#include "TelnetDataLogger.h"
//#include ""

namespace Telnet
{
    // TelnetDataCollector callback interface 
    class ITelnetDataCollectorEvents : public Utils::IBasicInterface
    {
    public:
        virtual void ParseCmd(byte cmd, byte opt) = 0;
        virtual void ParseSb(byte opt, const QByteArray &params) = 0;
        virtual void ParseText(const QByteArray &data) = 0;
        virtual TelnetDataLogger& GetLogger() = 0;
    };
    
    // -----------------------------------
    // TelnetClient callback interface 
    class ITelnetClientEvents : public Utils::IBasicInterface
    {
    public:
        virtual void TelnetDataInd(const QString &data) = 0;
        virtual void TelnetDiscInd() = 0;
        virtual void TelnetEstablishInd() = 0;
        virtual void TelnetChangeModeInd(SendingMode mode) = 0;
        virtual void TelnetSocketErrorInd(boost::shared_ptr<iNet::SocketError> error) = 0;
    };
    
    // -----------------------------------
    // TelnetServerSession callback interface
    class ITelnetServerSessionEvents : public Utils::IBasicInterface
    {
    public:
        virtual void TelnetDataInd(const QString &data) = 0;
        virtual void TelnetDiscInd() = 0;
        virtual void TelnetEstablishInd() = 0;
        virtual void TelnetChangeWinSizeInd(word width, word height) = 0;
        virtual void TelnetChangeSendingModeInd(SendingMode mode) = 0;
        virtual void TelnetSocketErrorInd(boost::shared_ptr<iNet::SocketError> error) = 0;
    };

    // -----------------------------------
    // TelentServer callback interface
    class ITelnetServerEvents : public Utils::IBasicInterface
    {
    public:
        virtual void ServerNewClientConnectInd(boost::shared_ptr<iNet::ITcpSocket> socket) = 0;
        virtual void ServerSocketErrorInd(boost::shared_ptr<iNet::SocketError> error) = 0;
    };

} // namespace Telnet

#endif

