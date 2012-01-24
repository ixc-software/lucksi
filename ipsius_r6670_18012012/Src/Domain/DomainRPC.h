#ifndef __DOMAINRPC__
#define __DOMAINRPC__

#include "stdafx.h"

#include "Utils/HostInf.h"

#include "RemoteCallIO.h"

namespace Domain
{

    /* 
        ���������� �����-����� �� �������� ��������� ������� ���������� � 
        ����������� RPC � ���� ��������������� �������� RemoteCall
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

