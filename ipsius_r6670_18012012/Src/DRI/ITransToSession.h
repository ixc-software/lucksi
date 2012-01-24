#ifndef __ITRANSTOSESSION__
#define __ITRANSTOSESSION__

#include "Utils/IBasicInterface.h"

namespace DRI
{
    class CommandDRI;
    
    class ITransToSession : public Utils::IBasicInterface
    {
    public:
        virtual void ExecuteCommand(shared_ptr<CommandDRI> cmd) = 0;
        virtual void TransactionCompleted() = 0;
        virtual void TransactionAbortedCommand(shared_ptr<CommandDRI> cmd) = 0;
    };
    
}  // namespace DRI

#endif
