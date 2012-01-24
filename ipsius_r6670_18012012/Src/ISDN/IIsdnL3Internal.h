#ifndef IISDNL3INTERNAL_H
#define IISDNL3INTERNAL_H

#include "Utils/IBasicInterface.h"
#include "Domain/IDomain.h"

#include "iCore/MsgThread.h"
#include "isdninfr.h"
#include "IL3Calls.h"
#include "isdnpack.h"
#include "CallRefGenerator.h"
#include "IIsdnLog.h"
#include "IsdnRole.h"
#include "L3Profiles.h"

#include "ObjLink/ObjectLink.h"

namespace ISDN
{

    class ICallbackDss;
    class StackWarning;
    class BChannelsDistributor;        
    class IeFactory;
    class L3StatCollector;
    class L3Packet;

    // Внутренний интерфейс для IsdnL3 (используется полями)
    class  IIsdnL3Internal : public Utils::IBasicInterface
    {
    public:

        virtual IL3Calls* GetIL3Calls() = 0;
        virtual ObjLink::ObjectLink<ICallbackDss>& GetIDssMng() = 0;
        //virtual void Send(IPacket* pPack) = 0;
        virtual void Send(const L3Packet& pack) = 0;
        
        virtual CallRefGenerator& GetCallRefGen() = 0;
        virtual BChannelsDistributor& GetBChanDistributor() = 0;
        virtual iCore::MsgThread& GetThread() = 0;
        virtual Domain::IDomain& GetIDomain() = 0;
        virtual IsdnInfra& GetInfra() = 0;
        virtual IeFactory& getIeFactory() = 0;
        //virtual bool TraceIeList()const = 0;
        //virtual bool TraceIeContent()const = 0;
        virtual const DssTraceOption& getTraceOption()const = 0;
        //virtual ILoggable* GetLogIntf() = 0; //TODO rename 
        virtual const ILoggable& getParentSession() = 0;
        virtual bool IsUserSide() const = 0;
        virtual const L3Profile::Options& GetOptions()const = 0;
        virtual const DssTimersProf& GetTimersProf()const = 0;
        virtual void SendStackWarning(shared_ptr<const StackWarning>) = 0;
        // Уведомление от списка вызовов
        virtual void AllCallsClearNotification() = 0;
        virtual L3StatCollector& getStat() = 0;
    };

} // ISDN

#endif

