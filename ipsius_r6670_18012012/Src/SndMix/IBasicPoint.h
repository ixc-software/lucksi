#ifndef __IBASICPOINT__
#define __IBASICPOINT__

#include "Utils/SafeRef.h"
#include "Utils/HostInf.h"
#include "Utils/IntToString.h"
#include "Utils/StatisticElement.h"

#include "iRtp/IRtpCoreToUser.h"
#include "iRtp/RtpCoreSession.h"
#include "iLog/iLogSessionCreator.h"
#include "TdmMng/EventsQueue.h"

#include "MixerError.h"
#include "MixerCodec.h"
#include "IMixPoint.h"
#include "IConfToTdm.h"
#include "MixUtils.h"

namespace SndMix
{
    using boost::scoped_ptr;
    using Platform::dword;
    using Utils::SafeRef;

    class IMixPoint;

    // ---------------------------------------------------------

    class IBasicPoint : public Utils::IBasicInterface
    {
    public:

        // TdmPoint only
        virtual bool GetTdmPointInfo(std::string &devName, int &chNum) const = 0;

        // RtpPoint only (ProcessRead using in GenPoint)
        virtual bool RtpSend(const std::string &dstIp, int dstPort) = 0;
        virtual void ProcessRead(dword ticks, Utils::StatElementForInt &sockStat) = 0;

        // both
        virtual void AddToConf(SafeRef<IMixPointOwner> owner, PointMode mode) = 0;
        virtual void RemoveFromConf() = 0;
        virtual IMixPoint* ConfPoint() = 0;  // can be null 
    };

    // ---------------------------------------------------------

    class IConferenceMng : public Utils::IBasicInterface
    {
    public:
        virtual const iRtp::RtpParams& RtpParams() const = 0;
        virtual iRtp::IRtpInfra& RtpInfra() = 0;
        virtual TdmMng::EventsQueue& Queue() = 0;
        virtual IConfToTdm& ConfToTdm() = 0;
        virtual SafeRef<iLogW::ILogSessionCreator> LogCreator() = 0;
    };

    /*
    class IBasicPointOwner : public IConferenceMng
    {
    public:
        virtual SafeRef<IMixPointOwner> MixerOwner() = 0;
        virtual Utils::SafeRef<iLogW::ILogSessionCreator> LogCreator() = 0;
    }; */
       
}  // namespace SndMix

#endif
