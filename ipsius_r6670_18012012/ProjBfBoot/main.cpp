#include "stdafx.h"


// Config
#include "BfConfig/BfConfigDiskIO.h"
#include "BfConfig/BfConfigCommon.h"
#include "BfServer/BfServerConfig.h"

// Server
#include "BfServer/BooterServer.h"
#include "BfServer/TCPSocketListener.h"

#include "BfUtils/StringConvertions.h"
#include "BfUtils/Log.h"


using namespace std;
using namespace BfServer;
using namespace BfConfig;
using namespace BfUtils;


//-----------------------------------------------------------------------------


namespace
{
    class LogStore : public BfUtils::ILogStore, boost::noncopyable
    {

    // BfUtils::ILogStore impl
    private:

        void StoreLogRecord(const std::string &s)
        {
            std::cout << s;
        }

    public:

    };



    LogStore logStore;
    void Log(string msg)
    {
        BfUtils::Logger logger(logStore);
        logger.Out() << msg << endl;
    }
}


//-----------------------------------------------------------------------------


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    // BfConfig* tests
    BfConfigCommon::Test();
    BfConfigParser::Test();
    BfConfigComposer::Test();



    // Read config
    StringList config;
    BfConfigDiskIO::ReadConfig("test.cfg", config);

    BfServerConfig serverConfig(config);
    serverConfig.LogVariables();

    Executer executer;

    BooterServer booterServer(serverConfig, executer, logStore, true);

    TCPSocketListener socketListener(
        executer,
        serverConfig.SocketListenerPollingPeriod.getValue(),
        serverConfig.TransportPollingPeriod.getValue(),
        booterServer,
        logStore, true);


    executer.Execute();

    return a.exec();
}
