#ifndef _I_EXIT_TASK_OBSERVER_H_
#define _I_EXIT_TASK_OBSERVER_H_

#include "stdafx.h"
#include "Utils/IBasicInterface.h"

namespace Utils
{
    class IExitTaskObserver : public Utils::IBasicInterface
    {
    public:
        virtual bool IsTaskOver() const = 0;
    };

};

#endif
