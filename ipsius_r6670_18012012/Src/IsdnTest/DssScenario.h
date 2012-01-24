#ifndef DSSSCENARIO_H
#define DSSSCENARIO_H

#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"

#include "Domain/DomainClass.h"

#include "Utils/Random.h"
#include "Utils/AtomicTypes.h"

//#include "iLog/DefaultLogFormating.h"
//#include "LogSubsystemPreparer.h"
#include "iLog/LogSessionProfileSetup.h"

#include "ISDN/IsdnStack.h"
#include "ISDN/IsdnStackSettings.h"
#include "ISDN/IsdnRole.h"
#include "ISDN/IsdnIntfGroup.h"
#include "ISDN/L2Profiles.h"
#include "ISDN/L3Profiles.h"

#include "NObjDssTestSettings.h"

#include "infratest.h"
#include "LogOut.h"

#include "DssCallBackAdapter.h"
#include "IDssScenarioBase.h"
#include "UniquePhoneNumberGen.h"
#include "SubScenariesList.h"
#include "IncommingCallRouter.h"
#include "RealSubs.h"

#include "DssConstructHalper.h"

#include "ITestResult.h"


namespace IsdnTest
{
    

    using Domain::DomainClass;

    //Базовый класс конкретных сценариев, место сборки 2х dss - систем
    class DssScenario : boost::noncopyable,
        public iCore::MsgObject,
        public IDssScenarioBase

    {
        enum
        {
            CPollInterval = 100, //интервал опроса подсценариев (msec)
            CLowLayerDataDelay = 0//задержка прохождения пакета между драйверами
        };

        struct LogKinds
        {
            LogKinds(Logger& log)
                : LDssStat(log.RegNewTag("Left Dss statistic")),
                RDssStat(log.RegNewTag("Right Dss statistic")),
                LLapdStat(log.RegNewTag("Left LAPD statistic")),
                RLapdStat(log.RegNewTag("Right LAPD statistic"))
            {}

            iLogW::LogRecordTag LDssStat;
            iLogW::LogRecordTag RDssStat;
            iLogW::LogRecordTag LLapdStat;
            iLogW::LogRecordTag RLapdStat;
        };

        // ff declaration of internal fsm
        class Fsm;

        NObjDssTestSettings& m_dssTestProf;
        ITestResult& m_callback;
        Utils::Random m_rnd;               

        Logger m_logSession;
        LogKinds m_logTags;

        IsdnInfraTest m_infra;

        // Fields for test logic
        shared_ptr<Fsm> m_pFsm;
        UniquePhoneNumberGen m_numGen;
        SubScenariesList<true> m_SubScnList;
        IncommingCallRouter m_inCallRouter;
        iCore::MsgTimer m_pollingTimer;
        bool m_autoStartpolling;
        DomainClass& m_domain;        

        // поля связанные с конструированием двух Dss - систем
        const NObjLapdTestSettings* m_pLapdPrifile;

        ISDN::IsdnIntfGroup m_userIntfGroup;
        ISDN::IsdnIntfGroup m_netIntfGroup;

        ISDN::IsdnStackSettings m_userStProf;
        ISDN::IsdnStackSettings m_netStProf;        

        DssConstructHelper m_userDss;
        DssConstructHelper m_netDss;        

        iCore::MsgTimer m_timeout;

        void OnTimeout(iCore::MsgTimer* p)
        {
            ESS_ASSERT(p = &m_timeout);
            m_callback.TestFinished(false, "Test timeout.");
        }
        
        NObjLapdTestSettings* CreateLapdTestProfile(NObjDssTestSettings& dssProf)
        {
            NObjLapdTestSettings* pProf = new NObjLapdTestSettings(&dssProf, "LapdTestProfile");
            pProf->m_TracedTest = false;
            pProf->m_pL2TraceOption->m_traceOn = dssProf.m_pL2TraceOption->m_traceOn;            
            pProf->m_pL2TraceOption->m_traceRaw = dssProf.m_pL2TraceOption->m_traceRaw; 
            return pProf;
        }
    
        void OnPollingTimer(iCore::MsgTimer*)
        {
            if ( m_SubScnList.AllComplete() )
            {
                m_pollingTimer.Stop();
                FinishTest();
            }                
        }

        void DssActivated() 
        {
            RegisterSubs(); // регистрация и запуск всех подсценариев

            if (m_autoStartpolling) // запуск опроса подсценариев
                m_pollingTimer.Start(CPollInterval, true); 
        }

        virtual void DssDeactivated(IsdnStack* from)
        {
            //TODO????????
        }

        void OnRun();        
        
       
    // Implementation IDssScenarioBase:
    private:        

        Domain::IDomain& GetIDomain();
        
        //iLogW::LogStore& GetLogStore();

        iLogW::ILogSessionToLogStore& GetLogStoreIntf();

        const iLogW::LogSessionProfile& GetLogSessionProf();

        IsdnInfraTest& GetInfra();            

    protected:

        IsdnStack& GetLeftTEstack();    // override IDssScenarioBase

        IsdnStack& GetRightNTstack();   // override IDssScenarioBase

        const Logger& GetLogger() {return m_logSession;}   // override IDssScenarioBase

        Utils::Random& GetRandom();   // override IDssScenarioBase

        ISDN::L3StatData getLeftDssStat();

        ISDN::L3StatData getRightDssStat();

        typedef DssSubScenario Sub;
        typedef Sub::OutgoingSide OutgoingSide;

        SubScenariesList<true>& GetSubsList()
        {
            return m_SubScnList;
        }

        // Создание, регистрация и запуск подсценария, возвращает адрес нового подсценария
        template <class TSubs, class TSettings>
        TSubs* AddSubsAndRun(OutgoingSide side,const TSettings* pProf = 0)
        {            
            StaticCheckInheritance<TSubs>();
            DssPhoneNumber num( m_numGen.Generate() );
            TSubs* pSubs = new TSubs( *this, num, side, pProf/*параметры*/);
            m_inCallRouter.Add(num, pSubs);
            m_SubScnList.Add(pSubs);
            pSubs->Run();
            return pSubs;
        }

        // Создание, регистрация и запуск count подсценариев,
        // если параметр pCreatedList != 0, пишет сюда созданные
        template <class TSubs, class TSettings>
        void AddSubsBlocAndRun(int count, OutgoingSide side, SubScenariesList<false>* pCreatedList = 0,const TSettings* pProf = 0)
        {       
            StaticCheckInheritance<TSubs>();
            TSubs *pSubs;
            for (int i = 0; i < count; ++i)
            {
                pSubs = AddSubsAndRun<TSubs>(side, pProf);
                if (pCreatedList) pCreatedList->Add(pSubs);
            }
        }

        template <class TSubs>
        TSubs* AddSubsAndRun(OutgoingSide side, const NObjDssTestSettings* pProf = 0)
        {
            return AddSubsAndRun<TSubs, NObjDssTestSettings>(side, pProf);
        }

        template <class TSubs>
        void AddSubsBlocAndRun(int count, OutgoingSide side, SubScenariesList<false>* pCreatedList = 0, const NObjDssTestSettings* pProf = 0)
        {
            return AddSubsBlocAndRun<TSubs, NObjDssTestSettings>(count, side, pCreatedList, pProf);
        }

        template<class T>
        void StaticCheckInheritance() const
        {
            enum { CInherCheck = boost::is_base_of<Sub, T>::value };
            BOOST_STATIC_ASSERT( CInherCheck );
        }

        virtual void RegisterSubs() = 0;

        void StartPolling();

        /*template <int N>
        struct NLayer{};

        void ReloadLayer(NLayer<1>);
        void ReloadLayer(NLayer<2>);
        void ReloadLayer(NLayer<3>);        
        void ReloadLayer(NLayer<4>);*/

        void ReloadDriver();
        void ReloadL2();
        void ReloadL3();
        void ReloadUser();
        void FinishTest();

    

        ~DssScenario();        

        DssScenario(
            DomainClass &domain, NObjDssTestSettings& prof,
            ITestResult& callback, const std::string& traceName, 
            bool autoStartPolling = true);               

        // активация Dss, регистрация подсценариев, включение опроса подсценариев (если autoStartPolling == true)
        void AsyncRun();
        
        virtual void AbortRealScenario()
        {
            // nothing
        }

        void RestartReq(bool userSide, Utils::IVirtualInvoke *pInvoke = 0);

    public:

        
        void AbortTest()
        {            
            m_pollingTimer.Stop();
            AbortRealScenario();
        }
    };

    
}// namespace IsdnTest

#endif

