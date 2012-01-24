#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "Utils/SafeRef.h"
#include "Utils/IBasicInterface.h"


// ----------------------------------------------------

namespace
{
    using Utils::SafeRef;

    template<class TIntf>
    class IntfUser
    {
        SafeRef<TIntf> m_ref;

    public:
        IntfUser(SafeRef<TIntf> ref) : m_ref(ref)
        {
        }
    };

    class IInterfaceOne : public Utils::IBasicInterface {};
    class IInterfaceTwo : public Utils::IBasicInterface {};

    class Server : 
        public Utils::SafeRefServer, 
        public IInterfaceOne, 
        public IInterfaceTwo
    {
    };

}

// ----------------------------------------------------

namespace UtilsTests
{
    using Platform::byte;

    void RunSafeRefTest(bool showInfo)
    {
        ESS_ASSERT( Utils::SafeRefDebugModeEnabled() );

        // server
        byte buffSrv[sizeof(Server)];
        Server *pServer = new(buffSrv) Server();

        // client one
        byte buffOne[sizeof(IntfUser<IInterfaceOne>)];
        new(buffOne) IntfUser<IInterfaceOne>(pServer);

        // client two
        byte buffTwo[sizeof(IntfUser<IInterfaceTwo>)];
        new(buffTwo) IntfUser<IInterfaceTwo>(pServer);

        // test exception
        bool wasException = false;

        {
            // translate assert to exception ESS::HookRethrow
            ESS::ExceptionHookRethrow<> globalHook;  

            try
            {
                pServer->~Server();
            }
            catch (ESS::HookRethrow &e)
            {
                wasException = true;
                e.NoStackTraceInMsg();
                if (showInfo) std::cout << e.what() << std::endl;
            }
        }

        ESS_ASSERT(wasException);
    }

}  // namespace UtilsTests

