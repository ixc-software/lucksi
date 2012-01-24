#ifndef ITASKMANAGER_H
#define ITASKMANAGER_H

#include "Utils/IBasicInterface.h"

namespace BfBootCli
{
    // будет заменен на ITaskManager
    class ITaskManager : public Utils::IBasicInterface
    {
    public:        
        virtual QString StateInfo() = 0; 
    };
    
} // namespace BfBootCli

#endif
