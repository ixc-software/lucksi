
#ifndef __IIDENTIFICATORVALIDATOR__
#define __IIDENTIFICATORVALIDATOR__

#include "Utils/IBasicInterface.h"

namespace DRI
{
    // Base interface for checking validation of object's and method's names
    class IIdentificatorValidator : public Utils::IBasicInterface
    {
    public:
        virtual bool IsCorrect(const QString &name) = 0;
    };

} // namespace DRI


#endif

