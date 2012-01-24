#include "stdafx.h"
#include "TcpServerTest.h"
#include "Lw/TcpServerSocket.h"
#include "Platform/Platform.h"

// --------------------------------------------

/*
    void ServerTest()
    {
    	const int CPort = 7071;    
    	Lw::HostAddr host("192.168.0.1");
    	
    	Lw::TcpServerSocket srvSock;    	
    	srvSock.Listen(CPort);

        Lw::TcpSocket sock;
        bool res = sock.Connect(host, CPort);
    	
    	boost::scoped_ptr<Lw::TcpSocket> pSocket; 
    	pSocket.reset( srvSock.Accept() );
    }
*/

namespace
{
    const int CPort = 44044;    
    Lw::HostAddr CSrvHost("192.168.0.49");
    Lw::HostAddr CConnectToHost("192.168.0.1");

    class ClientThread : Platform::Thread
    {

        void run()  // override
        {
            Lw::TcpSocket sock;
            bool res = sock.Connect(CConnectToHost, CPort);
            Sleep(100);
        }

    public:

        ClientThread()
        {
            start(LowPriority);
        }

    };

    void Run()
    {
        Lw::TcpServerSocket srvSock;    	
        srvSock.Listen(CPort, CSrvHost);

        // ClientThread thread;

        int count = 0;

        while(true)
        {
            Lw::TcpSocket *pSocket = 0;
            pSocket = srvSock.Accept();

            if (pSocket != 0)
            {
                count++;
            }

            Platform::ThreadSleep(10);

        }

    }


}  // namespace

// --------------------------------------------

namespace LwTest
{
	
	void TcpServerTest()
	{
        Run();
	}
	
}  // namespace LwTest

