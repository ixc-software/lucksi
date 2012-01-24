
#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/HostInf.h"

namespace Utils 
{
    class ThreadSyncEvent;
};
 
namespace iNet
{
    class SocketData;
};

namespace {
    
	using namespace Platform;
    // класс регистрирующий типы в системе мета-типов Qt
    class RegisterNetMetaInf
    {
    public:
        RegisterNetMetaInf()
        {
            qRegisterMetaType<Utils::HostInf> ("Utils::HostInf");  
            
            qRegisterMetaType< boost::shared_ptr<iNet::SocketData> >
                ("boost::shared_ptr<iNet::SocketData>");  

            qRegisterMetaType< boost::shared_ptr<Utils::ThreadSyncEvent> >
                ("boost::shared_ptr<Utils::ThreadSyncEvent>"); 
			
			qRegisterMetaType< size_t > ("size_t");
			qRegisterMetaType< byte > ("byte"); 
			qRegisterMetaType< word > ("word"); 
			qRegisterMetaType< dword > ("dword"); 

        }
    };
    RegisterNetMetaInf  G_RegisterNetMetaInf;    
};

