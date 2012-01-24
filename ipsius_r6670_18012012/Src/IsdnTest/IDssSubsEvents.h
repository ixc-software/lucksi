#ifndef IDSSSUBSEVENTS_H
#define IDSSSUBSEVENTS_H

#include "Domain/IDomain.h"
#include "Utils/IBasicInterface.h"
#include "iCore/MsgThread.h"
#include "ISDN/isdn_st.h" // можно заменить на DssUpIntf
#include "LogToFile.h"
#include "infratest.h"

namespace IsdnTest
{
    // implemented in DssScenario
    class IDssSubsEvents : public Utils::IBasicInterface
    {
    public:
        // Event to scn:
        //...

        // get property:
        //virtual iCore::MsgThread& GetThread() = 0;?
        virtual Domain::IDomain& GetIDomain() = 0;
        virtual ISDN::IsdnStack& GetLeftTEstack() = 0;
        virtual ISDN::IsdnStack& GetRightNTstack() = 0;
        virtual LogToFile& GetLog() = 0;
        virtual IsdnInfraTest& GetInfra() = 0;
        virtual Utils::Random& GetRandom() = 0;
    };

} // namespace IsdnTest

#endif
