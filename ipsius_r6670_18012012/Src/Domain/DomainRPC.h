#ifndef __DOMAINRPC__
#define __DOMAINRPC__

#include "stdafx.h"

#include "Utils/HostInf.h"

#include "RemoteCallIO.h"

namespace Domain
{

    /* 
        Соединение домен-домен по которому двоичными данными передаются и 
        принимаются RPC в виде сериализованных объектов RemoteCall
    */
    class DomainRPC
    {
    public:

        DomainRPC(const Utils::HostInf &rp)
        {
            // ...
        }

        void Send(const RemoteCallIO &stream)
        {
            // send stream to socket
            // ...
        }

    };
	
}  // namespace Domain


#endif

