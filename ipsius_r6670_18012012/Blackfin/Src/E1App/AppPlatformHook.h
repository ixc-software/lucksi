#ifndef __APPPLATFORMHOOK__
#define __APPPLATFORMHOOK__

namespace E1App
{

    void AppPlatformHookSetup(bool enabled, bool useUart, bool autoReboot, int uartSpeed = 115200);    
    
}  // namespace E1App

#endif
