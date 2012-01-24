#ifndef ISDNINTFGROUP_H
#define ISDNINTFGROUP_H

#include "stdafx.h"
#include "Utils/SafeRef.h"
#include "IsdnIntf.h"
#include "IBDistribStrategy.h"
#include "idsstogroup.h"
#include "BChannelsWrapper.h"

namespace ISDN
{
    class IsdnIntf;

    // ------------------------------------------------------------------------------------

    // управляет списком интерфейсов. Используется при формировании запросов в Dss1. Задается в профайле
    class IsdnIntfGroup : boost::noncopyable,
        public Utils::SafeRefServer,
        public IDssToGroup
    {        

    private:
        IntfList m_listIntf;        
        boost::scoped_ptr<IBDistribStrategy> m_srategy;
    // IDssToGroup
    private:
        int CountIntf() const;
        SharedBChansWrapper CaptureAny(int count, int intfId, bool isOutgoing);
        SharedBChansWrapper Capture(const SetBCannels& inquredSet, int intfId, bool exclusive, bool isOutgoing);        
    // user methods:
    public:
        IsdnIntfGroup(int number, const SetBCannels &bChannels);
        IsdnIntfGroup(){}

        void AddIntf(int number, const SetBCannels& set);
        void RemoveIntf(int number);
        void RemoveAllIntf();
        IsdnIntf* Interface(int number);
        const IsdnIntf* Interface(int number) const;

        template<class TStrategy>
        void SetStrategy()
        {            
            m_srategy.reset(new TStrategy());
        }
        
    };
} // namespace ISDN

#endif
