#ifndef REALSCENARIES_H
#define REALSCENARIES_H

#include "DssScenario.h"
#include  "NObjSimulLiveSettings.h"
#include "NObjDssRestartTestSettings.h"
#include "Domain/DomainClass.h"

#include "ITestResult.h"


namespace IsdnTest
{

    using Domain::DomainClass;

    // сценарий проверки процедуры установления/разрушения Dss - соединений
    class ScnCheckConnectionDisconnectionRoutine
        : public DssScenario
    {
        void RegisterSubs(); // Override

    public:
        // for use in NObjIsdnTestRunner
        typedef NObjDssTestSettings TNObjProfile;     
        
        ScnCheckConnectionDisconnectionRoutine(DomainClass &domain, NObjDssTestSettings& prof, ITestResult& callback)
            : DssScenario(domain, prof, callback, "ScnConDisc")
        {            
            AsyncRun();
        }
    };       

    //-------------------------------------------------------------------------------------------

    // TODO сценарий перезагрузки должен сам контролировать активацией/деактивацией Dss

    // Сценарий проверки механизма уведомления об отсутствии свободных каналов
    class ScnCheckBchannelsBusy
        : public DssScenario
    {
        enum
        {
            CPollInterval = 100 //интервал опроса подсценариев (msec)
        };

        SubScenariesList<false> m_channelCatchers;
        iCore::MsgTimer m_pollingTimer;

        void RegisterSubs(); // Override

        void OnPollingTimer(iCore::MsgTimer*);

        // override
        void AbortRealScenario()
        {
            m_pollingTimer.Stop();
        }

    public:

        // for use in NObjIsdnTestRunner
        typedef NObjDssTestSettings TNObjProfile;        

        ScnCheckBchannelsBusy(DomainClass &domain, NObjDssTestSettings& prof, ITestResult& callback)
            : DssScenario(domain, prof, callback, "ScnCheckBusy", false),
            m_pollingTimer(this, &ScnCheckBchannelsBusy::OnPollingTimer)
        {
            AsyncRun();
        }
    };   

    //-------------------------------------------------------------------------------------------

    namespace Detail
    {

        // Сценарий проверки механизма перезагрузки уровней
        template<class TLayerReloader> //reload Laeyr NLayer (1 - 2)
        class ScnCheckReload
            : public DssScenario
        {
            friend class ReloaderDrv;
            friend class ReloaderL2;
            friend class ReloaderL3;
            friend class ReloaderUsr;

            enum
            {
                CPollInterval = 100 //интервал опроса подсценариев (msec)
            };

            enum State
            {
                st_noWait,
                st_waitDeactivation,
                st_waitActivation,
                st_complete
            };

            State m_state;

            SubScenariesList<false> m_channelCatchers;
            iCore::MsgTimer m_pollingTimer;            

            //BOOST_STATIC_ASSERT( nLayer >= 1 && nLayer <= 4 ); // избыточно
            
            void OnPollingTimer(iCore::MsgTimer* pTimer)
            {
                // Когда установленны все соединения:
                if (m_channelCatchers.AllComplete())
                {
                    pTimer->Stop();            
                    
                    m_state = st_waitDeactivation;
                    //ReloadLayer( NLayer<nLayer>() );                
                    TLayerReloader::Reload(*this);
                }
            }

            
            void AbortRealScenario() // Override
            {
                m_pollingTimer.Stop();
            }

            void RegisterSubs() // Override
            {
                if (m_state == st_noWait)
                {
                    AddSubsBlocAndRun<SubConnectOnly>(1, Sub::rightSender, &m_channelCatchers);
                    AddSubsBlocAndRun<SubConnectOnly>(1, Sub::leftSender, &m_channelCatchers);

                    m_pollingTimer.Start(CPollInterval, true);
                }            
                
                if (m_state == st_waitActivation)
                {
                    m_state = st_complete;
                    StartPolling();// TestComplete
                }
            }

            void DssDeactivated( IsdnStack* from )
            {                               
                TUT_ASSERT(m_state == st_waitDeactivation || m_state == st_complete || m_state == st_waitActivation);
                
                m_state = st_waitActivation;
                AsyncRun();
            }

        public:            
            // for use in NObjIsdnTestRunner
            typedef NObjDssTestSettings TNObjProfile;
           
            ScnCheckReload(DomainClass &domain, NObjDssTestSettings& prof, ITestResult& callback)
                : DssScenario(domain, prof, callback, IsdnUtils::TypeToShortName<TLayerReloader>().toStdString(), false),
                m_state(st_noWait),
                m_pollingTimer(this, &ScnCheckReload::OnPollingTimer)
            {
                AsyncRun();
            }
        };

        class ReloaderDrv
        {
        public:

            // for use in TestCollector
            static const QString& getTestName()
            {
                static const QString name("ReloadDrv");
                return name;
            }

            static void Reload(ScnCheckReload<ReloaderDrv>& owner)
            {
                owner.ReloadDriver();
            }
        };

        class ReloaderL2
        {
        public:
            
            // for use in TestCollector
            static const QString& getTestName()
            {
                static const QString name("ReloadL2");
                return name;
            }

            static void Reload(ScnCheckReload<ReloaderL2>& owner)
            {
                owner.ReloadL2();
            }
        };

        class ReloaderL3
        {
        public:
           
            // for use in TestCollector
            static const QString& getTestName()
            {
                static const QString name("ReloadL3");
                return name;
            }

            static void Reload(ScnCheckReload<ReloaderL3>& owner)
            {
                owner.ReloadL3();
            }
        };

        class ReloaderUsr
        {
        public:
            
            // for use in TestCollector
            static const QString& getTestName()
            {
                static const QString name("ReloadUser");
                return name;
            }

            static void Reload(ScnCheckReload<ReloaderUsr>& owner)
            {
                owner.ReloadUser();
            }
        };

    } // namespace Detail

    typedef Detail::ScnCheckReload<Detail::ReloaderDrv> ScnCheckReloadDriver;
    typedef Detail::ScnCheckReload<Detail::ReloaderL2> ScnCheckReloadL2;
    typedef Detail::ScnCheckReload<Detail::ReloaderL3> ScnCheckReloadL3;
    typedef Detail::ScnCheckReload<Detail::ReloaderUsr> ScnCheckReloadUser;

   

    //-------------------------------------------------------------------------------------------

    // симуляция реальной активности
    // Созданн
    class ScnSimulLive
        : public DssScenario
    {

        enum
        {
            CPollInterval = 50, //интервал опроса подсценариев (msec)        
        };
        
        iCore::MsgTimer m_tSubsGenerator;
        iCore::MsgTimer m_tPollingTimer;
        iCore::MsgTimer m_tFinishTimer;
        const NObjSimulLiveSettings& m_prof;        
        SubScenariesList<true>& m_activeSubs;

        void RegisterSubs(); // Override

        void AbortRealScenario() // Override
        {
            m_tFinishTimer.Stop();
            m_tPollingTimer.Stop();
            m_tSubsGenerator.Stop();
        }

        void OnGenNewSubs(iCore::MsgTimer*);

        void OnPollingTimer(iCore::MsgTimer*);

        void OnFinishTest(iCore::MsgTimer*);

        // вычисляет интенсивность создания вызовов
        int EvalPeriod();

        OutgoingSide GenRndSide()
        {
            // 50/50
            return GetRandom().Next(2) ? Sub::leftSender : Sub::rightSender;
        }

        bool RndSelectTalkSub();

    public:

        // for use in TestCollector
        typedef NObjSimulLiveSettings TNObjProfile;        

        ScnSimulLive(DomainClass &domain, const NObjSimulLiveSettings& prof, ITestResult& callback)
            : DssScenario(domain, *prof.m_pGeneralSettings, callback, "ScnSimulLive", false),
            m_tSubsGenerator(this, &ScnSimulLive::OnGenNewSubs),
            m_tPollingTimer(this, &ScnSimulLive::OnPollingTimer),
            m_tFinishTimer(this, &ScnSimulLive::OnFinishTest),
            m_prof(prof),
            m_activeSubs( GetSubsList() )
        {            
            AsyncRun();
        }
    };  

    //---------------------------------------------------------------------------

    class ScnDeactivationProcedure
        : public DssScenario
    {
        SubScenariesList<false> m_channelCatchers;
        iCore::MsgTimer m_pollingTimer;          
        ISDN::IsdnStack* m_from;
        bool m_waitDeactivation;

        enum
        {
            CPollInterval = 100 //интервал опроса подсценариев (msec)
        };

        void AbortRealScenario() // Override
        {
            m_pollingTimer.Stop();
        }
        void RegisterSubs(); // Override
        void OnPollingTimer(iCore::MsgTimer* pTimer);

        void DssDeactivated(ISDN::IsdnStack* from); // override

    public:
       
        // for use in NObjIsdnTestRunner
        typedef NObjDssTestSettings TNObjProfile;        

        ScnDeactivationProcedure(DomainClass &domain, NObjDssTestSettings& prof, ITestResult& callback)
            : DssScenario(domain, prof, callback, "ScnDeactivation", false),
            m_pollingTimer(this, &ScnDeactivationProcedure::OnPollingTimer),
            m_from(0),
            m_waitDeactivation(false)
        {
            AsyncRun();
        }                
    };

    class ScnRestartProcedure
        : public DssScenario
    {
        enum State
        {
            st_null,
            st_waitCapture,
            st_waitRestartComplete,
            st_waitFree,
            st_complete
        };

        NObjDssRestartTestSettings& m_prof;
        State m_state;
        SubScenariesList<false> m_chanHolderList;
        iCore::MsgTimer m_timer;

        void RegisterSubs(); // Override

        void OnTimer(iCore::MsgTimer*);

        void OnRestartComplete()
        {
            TUT_ASSERT(m_state == st_waitRestartComplete);
            // todo проверить что все каналы освободились
            FinishTest();
        }

    public:
        // for use in NObjIsdnTestRunner
        typedef NObjDssRestartTestSettings TNObjProfile;        

        ScnRestartProcedure(DomainClass &domain, TNObjProfile& prof, ITestResult& callback)
            : DssScenario(domain, *prof.m_pGeneralSettings, callback, "ScnDeactivationProcedure", false),
            m_prof(prof),
            m_timer(this, &ScnRestartProcedure::OnTimer),
            m_state(st_null)
        {
            AsyncRun(); // --> RegisterSubs
        }    
    };

}//namespace IsdnTest

#endif

