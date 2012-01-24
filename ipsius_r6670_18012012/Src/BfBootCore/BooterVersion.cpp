#include "stdafx.h"

#include "Utils/IntToString.h"

#include "BooterVersion.h"

// Rebuild control:
#include "BootControlProto.h"
#include "BfBootSrv/BootServer.h"
#include "BfBootSrv/BootControl.h"
#include "BfBootSrv/BoardHardcodedConfig.h"
#include "StateInfoCode.h"
//...
#include "BfBootSrv/Config.h"
#include "BfBootSrv/FactorySettings.h"

namespace 
{
    
    enum
    {
        CBuildVer = 16,  // current version

        CCfgVer = BfBootSrv::Config::CVer,
        CFactoryVer = BfBootSrv::FactorySettings::CVer,
    };


    //BOOST_STATIC_ASSERT(CBuildVer == 0 && CCfgVer == 6 && CFactoryVer == 5); // used for test
    
    // Version history:
    // BOOST_STATIC_ASSERT(CBuildVer == 7 && CCfgVer == 5 && CFactoryVer == 5);
    // BOOST_STATIC_ASSERT(CBuildVer == 8 && CCfgVer == 6 && CFactoryVer == 5);
    // BOOST_STATIC_ASSERT(CBuildVer == 9 && CCfgVer == 6 && CFactoryVer == 5); 
    // BOOST_STATIC_ASSERT(CBuildVer == 10 && CCfgVer == 6 && CFactoryVer == 5); // release, lock interupts before load main
    // BOOST_STATIC_ASSERT(CBuildVer == 11 && CCfgVer == 6 && CFactoryVer == 5); // + invalidate cash
    //BOOST_STATIC_ASSERT(CBuildVer == 12 && CCfgVer == 6 && CFactoryVer == 5); // change in SpbConnection, use Watchdog::SwReset
    //BOOST_STATIC_ASSERT(CBuildVer == 13 && CCfgVer == 6 && CFactoryVer == 5); // changed tcpipbf537.dlb, start dhcp is async
    //BOOST_STATIC_ASSERT(CBuildVer == 14 && CCfgVer == 6 && CFactoryVer == 5); // use crc from utils
    //BOOST_STATIC_ASSERT(CBuildVer == 15 && CCfgVer == 7 && CFactoryVer == 5); // add booter version to cfg
    BOOST_STATIC_ASSERT(CBuildVer == 16 && CCfgVer == 7 && CFactoryVer == 5);   // change spi managment
    
} // namespace 


namespace BfBootCore
{
    using std::string;

    const string& BooterVersionInfo()
    {
        const std::string CSep = "/";
        static const string CBooterVersion(Utils::IntToString(BooterVersionNum()) + CSep 
            +  string(__DATE__) + CSep + string(__TIME__) );

        //static const string CBooterVersion( CRev + "Debug!");

        return CBooterVersion;        
    }

    Platform::dword BooterVersionNum()
    {
        return CBuildVer;
    }

} // namespace BfBootCore

