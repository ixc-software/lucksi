#ifndef REALSUBS_H
#define REALSUBS_H


#include "DssSubScenario.h"
#include "RealUserCalls.h"

namespace IsdnTest
{
    class NObjDssTestSettings;

    // подсценарий проверки процедуры установления/разрушения Dss - соединения
    template<bool rndDropEnabled, bool useAlerting = true>
    class SubTypicalConnectDisconnectProcedure
        : public DssSubScenario
    {        
        // настройка DssSubScenario
        void RegCalls() // override
        {
            TypicalOutCall* pOut = new TypicalOutCall(*this, useAlerting);
            TypicalInCall* pIn = new TypicalInCall(*this, useAlerting);
            SetCalls(pOut, pIn);            
        }        

    public:        

        SubTypicalConnectDisconnectProcedure(IDssScenarioBase &owner, const DssPhoneNumber& numReceiver, OutgoingSide side, const NObjDssTestSettings*)
            : DssSubScenario(owner, numReceiver, side, CMidleTimeout, rndDropEnabled )
        {}
    };

    //------------------------------------------------------------------------------

    // подсценарий установки соединения
    class SubConnectOnly
        : public DssSubScenario
    {

        // настройка DssSubScenario
        void RegCalls() // override
        {            
            SetCalls(new OutCreateConnect(*this), new InCreateConnect(*this));
        }

    public:        

        SubConnectOnly(IDssScenarioBase &owner, const DssPhoneNumber& numReceiver, OutgoingSide side,const NObjDssTestSettings*)
            : DssSubScenario(owner, numReceiver, side, CShortTimeout, false )
        {}
    };

    //------------------------------------------------------------------------------

    //подсценарий ожидания отказа с причиной отсутствия свободных каналов
    class SubBusyCheck
        : public DssSubScenario
    {

        // настройка DssSubScenario
        void RegCalls() // override
        {            
            SetCalls(new OutCheckBusyMsg(*this), 0);
        }

    public:

        SubBusyCheck(IDssScenarioBase &owner, const DssPhoneNumber& numReceiver, OutgoingSide side,const NObjDssTestSettings*)
            : DssSubScenario(owner, numReceiver, side, CShortTimeout, false )
        {}
    };

    //------------------------------------------------------------------------------

    //// подсценарий устанавливает соединение и ждет разрыва по причине деактивации Dss
    //class SubConnectAndWaitDeactivate
    //    : public DssSubScenario
    //{
    //    // настройка DssSubScenario
    //    void RegCalls() // override
    //    {            
    //        SetCalls(...);
    //    }

    //public:

    //    SubConnectAndWaitDeactivate(IDssScenarioBase &owner, const DssPhoneNumber& numReceiver, OutgoingSide side, bool enableRndDrop)
    //        : DssSubScenario(owner, numReceiver, side, CShortTimeout, enableRndDrop )
    //    {}
    //};

    // подсценарий для ScnSimilLive c завершением на этапе установки соединения
    class SubDroppedInConnection
        : public DssSubScenario
    {
        // настройка DssSubScenario
        void RegCalls() // override
        {            
            SetCalls( new OutDroppedBeforeConnect(*this), new InDroppedBeforeConnect(*this) );
        }

    public:

        SubDroppedInConnection(IDssScenarioBase &owner, const DssPhoneNumber& numReceiver, OutgoingSide side,const NObjDssTestSettings*)
            : DssSubScenario(owner, numReceiver, side, CNoTimeout, false )
        {
            //
        }

    };

    
    class SubTalkByTime
        : public DssSubScenario
    {
        const NObjSimulLiveSettings& m_prof;

        void RegCalls(); // override

        int GenRndTalkTime();

    public:

        SubTalkByTime(IDssScenarioBase &owner, const DssPhoneNumber& numReceiver, OutgoingSide side,const NObjSimulLiveSettings* pProf)
            : DssSubScenario(owner, numReceiver, side, CNoTimeout, false),
            m_prof(*pProf)
        {
            ESS_ASSERT(pProf);
        }
    };

} // namespace IsdnTest

#endif

