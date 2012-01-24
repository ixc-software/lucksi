#include "stdafx.h"
#include "RealScenaries.h"

#include "Utils/VirtualInvoke.h"

namespace IsdnTest
{
    void ScnCheckConnectionDisconnectionRoutine::RegisterSubs()
    {        
        AddSubsBlocAndRun<SubTypicalConnectDisconnectProcedure<false, true> >(2, Sub::leftSender);
        AddSubsBlocAndRun<SubTypicalConnectDisconnectProcedure<false, true> >(2, Sub::rightSender);

        AddSubsBlocAndRun<SubTypicalConnectDisconnectProcedure<false> >(3, Sub::leftSender);
        AddSubsBlocAndRun<SubTypicalConnectDisconnectProcedure<false> >(3, Sub::rightSender);

        AddSubsBlocAndRun<SubTypicalConnectDisconnectProcedure<true, true> >(5, Sub::leftSender);
        AddSubsBlocAndRun<SubTypicalConnectDisconnectProcedure<true, true> >(5, Sub::rightSender);        

        AddSubsBlocAndRun<SubTypicalConnectDisconnectProcedure<true, false> >(5, Sub::leftSender);
        AddSubsBlocAndRun<SubTypicalConnectDisconnectProcedure<true, false> >(5, Sub::rightSender);        
    }

    //---------------------------------------------------------------------------

    void ScnCheckBchannelsBusy::RegisterSubs()
    {        
        AddSubsBlocAndRun<SubConnectOnly>(15, Sub::rightSender, &m_channelCatchers);
        AddSubsBlocAndRun<SubConnectOnly>(15, Sub::leftSender, &m_channelCatchers);
        
        m_pollingTimer.Start(CPollInterval, true);        
    }

    void ScnCheckBchannelsBusy::OnPollingTimer(iCore::MsgTimer* pTimer)
    {
        //  огда установленны все соединени€:
        if (m_channelCatchers.AllComplete())
        {
            pTimer->Stop();            
            AddSubsAndRun<SubBusyCheck>(Sub::leftSender, false);
            AddSubsAndRun<SubBusyCheck>(Sub::rightSender, false);            
            
            StartPolling();
        }
    }

    //---------------------------------------------------------------------------

    void ScnSimulLive::RegisterSubs()
    {        
        m_tSubsGenerator.Start( EvalPeriod() );
        m_tPollingTimer.Start(CPollInterval, true);
        m_tFinishTimer.Start(m_prof.m_TestDuration);
        m_prof.Statistic.Start();
    }

    void ScnSimulLive::OnGenNewSubs(iCore::MsgTimer*)
    {
        //удал€ем отработанные
        m_activeSubs.DelIfComplete();

        //restart timer with new random period        
        m_tSubsGenerator.Start( EvalPeriod() );        
        
        //generate new sub (new call)
        if ( RndSelectTalkSub() )
            AddSubsAndRun<SubTalkByTime>( GenRndSide(), &m_prof ); //Sub::xxxSender
        else
        {
            AddSubsAndRun<SubDroppedInConnection>( GenRndSide() );        
            m_prof.Statistic.IncrDropedCall();
        }
    }

    bool ScnSimulLive::RndSelectTalkSub() // TODO назвавние
    {
        int rnd = GetRandom().Next(100);
        return rnd > m_prof.m_PrcentDropedCallInConnection;
    }

    void ScnSimulLive::OnPollingTimer(iCore::MsgTimer*)
    {
        m_activeSubs.DelIfComplete();
    }

    void ScnSimulLive::OnFinishTest(iCore::MsgTimer*)
    {
        //std::cout << "xxxxxxxxxxxxxxxxxxxxxxx Finish SimulLive test xxxxxxxxxxxxxxxxxxxxxxxx\n";
        m_tSubsGenerator.Stop();
        m_prof.Statistic.Finish();
        if (m_prof.m_OutStatistic)
            std::cout << m_prof.Statistic.getAsString();
        FinishTest();
    }

    int ScnSimulLive::EvalPeriod()
    {
        Utils::Random& rnd = GetRandom();        
        return rnd.Next(m_prof.m_MaxCallInterval - m_prof.m_MinCallInterval) + m_prof.m_MinCallInterval;     
    }

    //---------------------------------------------------------------------------

    void ScnDeactivationProcedure::RegisterSubs() // Override
    {
        AddSubsBlocAndRun<SubConnectOnly>(15, Sub::rightSender, &m_channelCatchers);
        AddSubsBlocAndRun<SubConnectOnly>(15, Sub::leftSender, &m_channelCatchers);
        m_pollingTimer.Start(CPollInterval, true);
    }

    void ScnDeactivationProcedure::OnPollingTimer(iCore::MsgTimer* pTimer)
    {
        //  огда установленны все соединени€:
        if (m_channelCatchers.AllComplete() && !m_waitDeactivation)
        {                                    
            m_from = &GetLeftTEstack();
            m_from->GetDssIntf()->Deactivate();
            m_waitDeactivation = true;
        }
    }

    void ScnDeactivationProcedure::DssDeactivated(ISDN::IsdnStack* from)
    {
        if (from == m_from && m_waitDeactivation)
            StartPolling(); // test complete
    }

    // ------------------------------------------------------------------------------------
   

    void ScnRestartProcedure::RegisterSubs() /* Override */
    {
        AddSubsBlocAndRun<SubConnectOnly>(15, Sub::rightSender, &m_chanHolderList);
        AddSubsBlocAndRun<SubConnectOnly>(15, Sub::leftSender, &m_chanHolderList);

        m_state = st_waitCapture;
        m_timer.Start(50, true);
    }

    void ScnRestartProcedure::OnTimer( iCore::MsgTimer* )
    {
        if (m_state != st_waitCapture) return;

        if (m_chanHolderList.AllComplete())                        
        {                
            RestartReq(
                m_prof.m_restartFromUserSide,                
                new Utils::VIMethod<ScnRestartProcedure>(&ScnRestartProcedure::OnRestartComplete, *this)
                ); // todo side from profile
            m_state = st_waitRestartComplete;
            m_timer.Stop();
        }
    }
} // namespace IsdnTest


