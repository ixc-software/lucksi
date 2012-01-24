#ifndef IL2PROFILE_H
#define IL2PROFILE_H

#include "IsdnConfig.h"
#include "stdafx.h"
#include "Utils/IBasicInterface.h"

class IsdnL2;
namespace ISDN
{
    class IL2Profile : public Utils::IBasicInterface
    {
    public:

        virtual void Set(IsdnL2*)const = 0;

        virtual std::string getLogSessionName() const = 0;
        virtual bool getTracedOn() const = 0;
        virtual bool getTracedRaw() const = 0;
    };

} // ISDN

#endif
