#ifndef __DATASTREAMTEST__
#define __DATASTREAMTEST__

#include "DataStream.h"
#include "ProfileSetup.h"

namespace SockTest
{
    using Platform::byte;
	
    class DataStreamTest
    {
    public:

        static void Run()
        {
            UdpTestClientsideProfile profCli = UdpTestClientsideProfile::Create();
            boost::scoped_ptr<DataStream> cli( profCli.CreateDataStream() );

            UdpTestSrvsideProfile profSrv = UdpTestSrvsideProfile::Create();
            boost::scoped_ptr<DataStream> srv( profSrv.CreateDataStream() );

            while(true)
            {
                std::vector<byte> buff;

                if (cli->Peek(buff)) 
                {
                    srv->Put(buff);
                }

                if (srv->Peek(buff)) 
                {
                    cli->Put(buff);
                }
                    
                if (cli->RunCompleted() && srv->RunCompleted())
                {
                    ESS_ASSERT(cli->State() == DssCompleted);
                    ESS_ASSERT(srv->State() == DssCompleted);
                    break;
                }
            }
        }
    };
		
}  // namespace SockTest

#endif
