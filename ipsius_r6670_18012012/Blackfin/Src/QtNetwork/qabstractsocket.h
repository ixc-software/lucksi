
#ifndef QABSTRACTSOCKET_H
#define QABSTRACTSOCKET_H

// Using by QHostAddress
class QAbstractSocket
{
public:
    enum NetworkLayerProtocol
    {
        IPv4Protocol,
        IPv6Protocol,
        UnknownNetworkLayerProtocol = -1
    };
};

#endif
