
#ifndef __IMFPROCESSEVENTS__
#define __IMFPROCESSEVENTS__

// IMfProcessEvents.h

#include "Utils/IBasicInterface.h"

namespace Utils
{
    class ThreadSyncEvent;

} // namespace 

// -----------------------------------------------------

namespace MiniFlasher
{
    // Events of the processes (tasks) runned by NObjMiniFlasher 
    class IMfProcessEvents : public Utils::IBasicInterface
    {
    public:
        virtual void MsgLog(const std::string &msg, bool eof = true) = 0;
        virtual void MsgProcessEnd(bool ok, const std::string &msg = "") = 0;
        // result != 0 --> true
        virtual boost::shared_ptr<Utils::ThreadSyncEvent> IsAborted() = 0;
    };
    
} // namespace MiniFlasher

#endif 
