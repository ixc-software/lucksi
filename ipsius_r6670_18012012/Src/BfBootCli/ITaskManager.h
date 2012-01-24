#ifndef ITASKMANAGER_H
#define ITASKMANAGER_H

#include "Utils/IBasicInterface.h"

namespace BfBootCli
{
    // ����� ������� �� ITaskManager
    class ITaskManager : public Utils::IBasicInterface
    {
    public:        
        virtual QString StateInfo() = 0; 
    };
    
} // namespace BfBootCli

#endif
