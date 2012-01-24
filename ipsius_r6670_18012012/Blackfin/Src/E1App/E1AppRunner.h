#ifndef _E1_APP_RUNNER_H_
#define _E1_APP_RUNNER_H_

#include "E1AppConfig.h"
#include "BfBootSrv/IReload.h"
#include "ICloseApp.h"
#include "Platform/PlatformTypes.h"

namespace BfBootSrv { class BoardSetup ; };

namespace E1App 
{    
    void RunE1Application(BfBootSrv::IReload& reloader,
        ICloseApp& closeImpl,
        const E1AppConfig &config,
        const BfBootCore::BroadcastMsg &msg,
        Platform::word cmp);
};

#endif
