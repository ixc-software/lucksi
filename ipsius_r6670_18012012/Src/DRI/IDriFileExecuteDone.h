
#ifndef __IFILEEXECUTEDONE__
#define __IFILEEXECUTEDONE__

// IDriFileExecuteDone.h

#include "Utils/IBasicInterface.h"

namespace DRI
{
    class FileExecuteResult;
    
    class IFileExecuteDone : public Utils::IBasicInterface
    {
    public:
        virtual void OnFileExecuteDone(boost::shared_ptr<FileExecuteResult> result) = 0;
    };

    
} // namespace DRI


#endif
