#ifndef __PLATFORMEXITHANDLE__
#define __PLATFORMEXITHANDLE__

namespace Platform
{

    typedef void (*ExitHandleFn)(const char *p);
    
    ExitHandleFn SetExitHandle(ExitHandleFn newHandle);
    
}  // namespace Platform

#endif
