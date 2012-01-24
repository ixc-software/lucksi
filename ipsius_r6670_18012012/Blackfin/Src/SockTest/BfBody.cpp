#include "stdafx.h"
#include "Platform/Platform.h"
#include "Lw/UdpSocket.h"
#include "Lw/TcpSocket.h"
#include "Lw/TcpServerSocket.h"

#include "BfBody.h"
#include "ProfileSetup.h"
#include "CommonUtils.h"

using namespace SockTest;
using boost::scoped_ptr;
using Platform::byte;
using Utils::HostInf;

// --------------------------------------------

namespace
{
	
    void Print(const std::string &s)
    {
        std::cout << s << std::endl;
    }

    // -----------------------------------------------------------

    void UdpTestClientside()
    {
        std::string CPrefix("UdpTest: ");

        UdpTestClientsideProfile profile = UdpTestClientsideProfile::Create();
        boost::scoped_ptr<DataStream> data( profile.CreateDataStream() );

        Lw::UdpSocket sock(profile.OwnPort());
        
        HostInf remoteAddr(profile.RemoteHost(), profile.RemotePort());
        // Lw::HostAddr rh(profile.RemoteHost());
        
        while(true)
        {
            // send
            {
                std::vector<byte> buff;
                if (data->Peek(buff)) 
                {
                	sock.SendTo(remoteAddr, buff);
                }
                
            }

            // recv
            {
                std::vector<byte> buff;
                HostInf h;

                if (sock.Recv(buff, h))
                {  
                    // Print(CommUtils::DumpVector(recvBuff, 8));  // dump

                	data->Put(buff);
                }
            }

            // condition
            if (data->State() != DssInProgress)
            {
                std::string msg = data->StateAsString();
                Print(CPrefix + msg);
                break;
            }

            // sleep
            Platform::ThreadSleep(5);
        }
    }

    // -----------------------------------------------------------

    void TcpSockLoop(Lw::TcpSocket *pSock, DataStream *pData, const std::string &prefix)
    {
        while(true)
        {
            // recv            
            {            	
                std::vector<byte> buff;

                if (pSock->Recv(buff))
                {  
                    // Print(CommUtils::DumpVector(recvBuff, 8));  // dump

                    pData->Put(buff);
                } 
            } 

            // send
            {
                std::vector<byte> buff;
                if (pData->Peek(buff)) 
                {
                    pSock->Send(buff);
                }

            }

            // disconnect
            if (!pSock->Connected())
            {
                Print(prefix + "Disconnected!");
                break;
            }

            // condition
            if (pData->State() != DssInProgress)
            {
                std::string msg = pData->StateAsString();
                Print(prefix + msg);
                break;
            }

            // debug
            /*
            if (Platform::GetSystemTickCount() - syncTime > 500)
            {
            Print(".");
            } */

            // sleep
            Platform::ThreadSleep(5);
        }

    }

    // -----------------------------------------------------------

    void TcpSockTestClientside()
    {
        std::string CPrefix("TcpSockTest: ");

        TcpSockTestClientsideProfile profile = TcpSockTestClientsideProfile::Create();
        boost::scoped_ptr<DataStream> data( profile.CreateDataStream() );

        Lw::TcpSocket sock;

        if ( !sock.Connect( HostInf(profile.RemoteHost(), profile.RemotePort()) )  )   // Lw::HostAddr(profile.RemoteHost()), profile.RemotePort() )
        {
            Print(CPrefix + "Can't connect!");
            return;
        }

        int syncTime = Platform::GetSystemTickCount();

        TcpSockLoop(&sock, data.get(), CPrefix);
    }

    // --------------------------------------------------------------

    Lw::TcpSocket* AcceptClient(Lw::TcpServerSocket &srvSock, int timeout)
    {
        int startTime = Platform::GetSystemTickCount();        

        while(Platform::GetSystemTickCount() - startTime < timeout)
        {
            Lw::TcpSocket *pSock = srvSock.Accept();
            if (pSock != 0) return pSock;
        }

        return 0;
    }

    void TcpSrvTestClientside(int acceptTimeout)
    {
        std::string CPrefix("TcpSrvTest: ");

        TcpSrvTestClientsideProfile profile = TcpSrvTestClientsideProfile::Create();

        Lw::TcpServerSocket srvSock;
        srvSock.Listen(profile.ListenPort());

        scoped_ptr<Lw::TcpSocket> sock( AcceptClient(srvSock, acceptTimeout) );

        if (sock.get() == 0)
        {
            Print(CPrefix + "accept timeout");
            return;
        }

        Print(CPrefix + "accepted!");

        boost::scoped_ptr<DataStream> data( profile.CreateDataStream() );

        TcpSockLoop(sock.get(), data.get(), CPrefix);
    }

}


// --------------------------------------------

namespace SockTest
{
	
	void RunBfBody()
	{
		while(true)
	    {
        	UdpTestClientside(); 
	    	
            TcpSrvTestClientside(20 * 1000);

            TcpSockTestClientside();

        	Platform::ThreadSleep(2 * 1000);
	    }
	}
		
}  // namespace SockTest
