#ifndef IEXIT_H
#define IEXIT_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
//#include "Utils/IExecutor.h"

namespace BfBootSrv
{
    using boost::shared_ptr;

    class IBootCtrlOwner : public Utils::IBasicInterface
    {
    public:
        virtual void AsyncExit() = 0;          
    };
} // namespace BfBootSrv

#endif
