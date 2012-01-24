#ifndef __MSGTHREADFN__
#define __MSGTHREADFN__

#include "MsgThread.h"

namespace iCore
{

    /* 
        Warning: use these functions with care, you must be sure -- 
        destination object (closured in functor) will be alive 
    */

    void MsgThreadFn(MsgThread &thread, const boost::function<void ()> &fn);
    void MsgThreadFnDelayed(MsgThread &thread, const boost::function<void ()> &fn, int delayMs);
        
}  // namespace iCore

#endif

