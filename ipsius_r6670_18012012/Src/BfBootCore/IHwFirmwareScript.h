#ifndef IHWFIRMWARESCRIPT_H
#define IHWFIRMWARESCRIPT_H

#include "Utils/IBasicInterface.h"

namespace BfBootCore
{
    class IHwFirmwareScript : public Utils::IBasicInterface
    {
    public:
        virtual std::string Name() const = 0;
        virtual std::string Value() const = 0;
    };
} // namespace BfBootCore

#endif
