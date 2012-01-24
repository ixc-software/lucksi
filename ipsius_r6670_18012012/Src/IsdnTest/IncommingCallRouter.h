#ifndef INCOMMINGCALLROUTER_H
#define INCOMMINGCALLROUTER_H

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "DssSubScenario.h"
#include "ISDN/DssUpIntf.h"

namespace IsdnTest
{    
    using boost::shared_ptr;
    using ISDN::IDssCall;
    using ISDN::DssCallParams;
    using ISDN::DssPhoneNumber;
    using ISDN::IsdnStack;

    // функциональный объект совместимый с STL для сравнения DssDigits
    struct LessDigits
        : public std::binary_function<ISDN::Dss1Digits, ISDN::Dss1Digits, bool>
    {	
        bool operator()(const ISDN::Dss1Digits& left, const ISDN::Dss1Digits& right) const
        {	
            return (left.getAsString() < right.getAsString());
        }
    };

    // маршрутизатор входящих Dss - вызовов по тел. номеру получателя
    class IncommingCallRouter : boost::noncopyable
    {
        typedef std::map<ISDN::Dss1Digits, DssSubScenario*const, LessDigits> RoutMap;
        RoutMap m_rout;

    public:
        
        void IncomingCall(ISDN::BinderToDssCall dssCall, shared_ptr<const DssCallParams> pParams, IsdnStack* from)
        {
            RoutMap::iterator i;
            i = m_rout.find( pParams->GetCalled().Digits() );
            TUT_ASSERT( i != m_rout.end() && "called number not found");
            i->second->IncomingCall(dssCall, pParams, from); // rout to SubScenario
        }

        void Add(const DssPhoneNumber& num, DssSubScenario* pSubs)
        {
            bool insertOk = m_rout.insert( RoutMap::value_type(num.Digits(), pSubs) ).second;
            ESS_ASSERT(insertOk);
        }
    };
} // namespace IsdnTest

#endif

