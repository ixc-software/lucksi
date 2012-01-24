
#ifndef __IDOMAINAUTORUN__
#define __IDOMAINAUTORUN__

// IDomainAutorun.h

#include "IDomain.h"

namespace Domain
{
    // Using by DominClass
    class IDomainAutorunToOwner : public Utils::IBasicInterface
    {
    public:
        virtual void AutorunFinished(const QString &error = "") = 0; // call via AsyncTask
        virtual void LogAutorunOutput(const QString &text) = 0;
    };
    
} // namespace Domain

#endif
