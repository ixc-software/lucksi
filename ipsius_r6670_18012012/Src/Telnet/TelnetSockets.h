#ifndef __TELNETSOCKETS__
#define __TELNETSOCKETS__

#include "stdafx.h"

#include "TelnetTests/TcpSocketEmul.h"
#include "TelnetTests/TcpServerSocketEmul.h"

#include "iNet/MsgTcpSocket.h"
#include "iNet/MsgTcpServer.h"

namespace Telnet
{

    /*
    typedef TelnetTests::TcpSocketEmul TelnetSocket;
    typedef TelnetTests::TcpServerSocketEmul TelnetServerSocket;
    */


    typedef iNet::MsgTcpSocket TelnetSocket;
    typedef iNet::MsgTcpServer TelnetServerSocket;
    

} // namesapce Telnet

#endif
