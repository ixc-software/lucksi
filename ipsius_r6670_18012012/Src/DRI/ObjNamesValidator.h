
#ifndef __OBJNAMESVALIDATOR__
#define __OBJNAMESVALIDATOR__

#include "IIdentificatorValidator.h"
#include "Domain/Identificator.h"

namespace DRI
{
    // Using for checking validation of object's and method's names.
    // Check's performing by Domain::Identificator.
    class ObjNamesValidator
        : public IIdentificatorValidator, public virtual Utils::SafeRefServer
    {
    // IIdentificatorValidator impl
    private:
        bool IsCorrect(const QString &name)
        {
            return Domain::Identificator::CheckObjectName(name);
        }
    };

} // namespace DRI 


#endif

