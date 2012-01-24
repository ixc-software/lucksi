#ifndef DSSSUBSCENARIO_H
#define DSSSUBSCENARIO_H

#include "stdafx.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "ISDN/IsdnStack.h"
#include "ISDN/DssCallParams.h"

#include "DssUserCall.h"
#include "logger.h"

namespace IsdnTest
{
    
    using boost::shared_ptr;    
    using boost::scoped_ptr; 
    using iCore::MsgTimer;

    using ISDN::DssPhoneNumber;
    using ISDN::IDssCall;
    using ISDN::DssCallParams;
    using ISDN::IsdnStack;
    
    using ISDN::BinderToDssCall;

    // базовый класс всех подсценариев
    class DssSubScenario
        : public ISubsToUserCall,
        public iCore::MsgObject,
        boost::noncopyable
    {
        struct LogRecordKinds
        {
            void Init(Logger& session)
            {
                General = session.RegNewTag("General info");
                Error = session.RegNewTag("Error!");                
            }
            iLogW::LogRecordTag General;
            iLogW::LogRecordTag Error;            
        };

    protected:

        enum Timeout
        {
            CNoTimeout = 0,
            CLargeTimeout = 2 * 60 * 1000UL,
            CMidleTimeout = 50 * 1000UL,
            CShortTimeout = 30 * 1000UL
        };

    public:

        enum OutgoingSide
        {
            leftSender, // отправитель слева
            rightSender // отправитель справа
        };

        // передавать сюда номер или генератор
        DssSubScenario(IDssScenarioBase& owner, const DssPhoneNumber& numReceiver, OutgoingSide side,
            Timeout timeout, bool enableRndDrop);

        // запуск 
        void Run();

        bool IsComplete();

        void IncomingCall(BinderToDssCall dssCall, shared_ptr<const DssCallParams> pParams, IsdnStack* from);   
    

    protected:        

        void SetCalls(OutgoingDssUserCall* pOutCall, IncommingDssUserCall* pInCall = 0)
        {
            ESS_ASSERT(m_pOutCall.get() == 0);
            m_pOutCall.reset(pOutCall);
            pOutCall->SetStack( (m_side == leftSender) ? m_leftSt : m_rightSt);

            if (pInCall != 0)
            {
                ESS_ASSERT( m_pInCall.get() == 0 );
                m_pInCall.reset(pInCall);                
                pInCall->SetStack( (m_side == rightSender) ? m_leftSt : m_rightSt );
            }
        }

        IDssScenarioBase& getOwner()
        {
            return m_owner;
        }

    private:

        OutgoingSide m_side;
        
        IDssScenarioBase& m_owner;

        IsdnStack& m_leftSt;
        IsdnStack& m_rightSt;

        scoped_ptr<Logger> m_pLog;
        LogRecordKinds m_logTags;

        shared_ptr<IncommingDssUserCall> m_pInCall;
        shared_ptr<OutgoingDssUserCall> m_pOutCall;
        
        Timeout m_timeout;
        bool m_enableRndDrop;
        MsgTimer m_tTimeout;
        MsgTimer m_tDrop;

        DssPhoneNumber m_numReceiver;
        //DssPhoneNumber m_numSender; // пока не нужен

        virtual void RegCalls() = 0; // impl in real subs            

    // implementation ISubsToUserCall
    private:

        DssPhoneNumber& getNumReceiver();        
        
        const Logger& getLogger();        

        IsdnInfraTest& getInfra();

        void DeleteOutCall();

        void DeleteInCall();

        void AsyncDeleteOutCall();

        void AsyncDeleteInCall();

        IDssScenarioBase& GetIScenario();
    
    private:

        void OnTimeout(MsgTimer* );

        void OnDrop(MsgTimer* );

        std::string getSubsName();    

    };
} // namespace IsdnTest

#endif

