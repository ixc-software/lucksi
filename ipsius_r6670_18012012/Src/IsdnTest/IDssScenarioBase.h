#ifndef IDSSSCENARIOBASE_H//IDSSSUBSEVENTS_H
#define IDSSSCENARIOBASE_H//IDSSSUBSEVENTS_H

#include "Domain/IDomain.h"
#include "Utils/IBasicInterface.h"
#include "iCore/MsgThread.h"
#include "ISDN/IsdnStack.h" // можно заменить на DssUpIntf
#include "LogOut.h"
#include "infratest.h"

namespace IsdnTest
{
    class Logger;

    // implemented in DssScenario
    class IDssScenarioBase : public Utils::IBasicInterface
    {
    public:
        // Event to scn:
        //...

        // get property:        
        virtual Domain::IDomain& GetIDomain() = 0;
        virtual ISDN::IsdnStack& GetLeftTEstack() = 0;
        virtual ISDN::IsdnStack& GetRightNTstack() = 0;
        
        virtual iLogW::ILogSessionToLogStore& GetLogStoreIntf() = 0;
        virtual const iLogW::LogSessionProfile& GetLogSessionProf() = 0;
        virtual const Logger& GetLogger() = 0;

        virtual IsdnInfraTest& GetInfra() = 0;
        virtual Utils::Random& GetRandom() = 0;
    };

} // namespace IsdnTest

#endif

