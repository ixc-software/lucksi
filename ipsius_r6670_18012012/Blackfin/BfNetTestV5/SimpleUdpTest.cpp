#include "stdafx.h"
#include "Lw/UdpSocket.h"
#include "Lw/TcpServerSocket.h"
#include "iVDK/VdkUtils.h"
#include "BfDev/SysProperties.h"
#include "SockTest/BfBody.h"
#include "SimpleUdpTest.h"
#include "TcpServerTest.h"

// ---------------------------------------

using Lw::HostAddr;

namespace
{
    enum 
    { 
        CLocalPort = 10340,
        CRemotePort = 10342,
    };
    
    void UdpTest()
    {
        Lw::UdpSocket sock(CLocalPort);

        Lw::Packet sendPack, recvPack;

        sendPack.push_back(0x11);
        sendPack.push_back(0x66);

        HostAddr rh("192.168.0.1");

        int count = 0;

        while(true)
        {
            // send
            sock.SendTo(rh, CRemotePort, sendPack);

            // recv
            {
                recvPack.resize(0x100);
                HostAddr h;
                int port;
                int sum = 0;

                if (sock.Recv(recvPack, h, port))
                {
                    int len = recvPack.size();
                    for(int i = 0; i < len; ++i)
                    {
                        sum += recvPack.at(i);
                    }

                    if (count == 0)  // debug
                    {
                        std::ostringstream ss;
                        ss << h.get() << ":" << port << " size " << len;
                        std::cout << ss.str() << std::endl;
                    }

                    count++;
                } 
            }

            // sleep
            iVDK::Sleep(200);
        }
    }

}  // namespace

extern "C"
{
	void RunSimpleUdpTest()
	{		
		BfDev::SysProperties::InitFromProjectOptions();
		
		{
			std::string s("192.168.0.1");
			HostAddr h(s);
			ESS_ASSERT(h.get() == s);
		}
					
		{
            /*
            lwip_gethostbyname()
			HostAddr h1("localhost");
			HostAddr h2("127.0.0.1");
			ESS_ASSERT(h1.getRaw() == h2.getRaw()); */
		}
		
		SockTest::RunBfBody();
		
		// LwTest::TcpServerTest();
				
		// ServerTest();

        // UdpTest();
		
	}
}


