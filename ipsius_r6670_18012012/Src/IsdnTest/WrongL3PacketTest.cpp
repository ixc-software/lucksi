#include "stdafx.h"
#include "WrongL3PacketTest.h"
#include "ISDN/L3PacketFactory.h"
#include "ISDN/IeFactory.h"
#include "infratest.h"
#include "Domain/DomainClass.h"
#include "iLog/LogManager.h"
#include "iCore/MsgObject.h"
#include "ISDN/DssWarning.h"
#include "Utils/TimerTicks.h"
#include "RandomPatternGen.h"

#include "ISDN/L3PacketTypes.h"

namespace 
{
    
} // namespace 

namespace IsdnTest
{
    using Domain::DomainClass;
    using Platform::byte;

    class WrongL3PacketTest::Impl : public iCore::MsgObject
    {
        Utils::Random m_rnd;
        IsdnInfraTest m_infra;
        ISDN::IeFactory m_ieFactory;
        ISDN::L3PacketFactory m_packFactory;
        Utils::TimerTicks m_timeout;

        const NObjWrongL3PacketTestSettings& m_prof;
        
        boost::scoped_ptr<RandomPatternGen> m_dataGen;
        
        ITestResult& m_callback;
        bool m_active;        

        // statistic
        int m_allPackCount;
        int m_badPackCount;
        Utils::TimerTicks m_statTimer;
        int m_printCount;


        bool genByPattern()
        {
            return m_rnd.Next(100) > m_prof.m_rndPackPercent;
        }

        QVector<byte> getRndData()
        {   
            if (genByPattern()) return m_dataGen->Next();

            QVector<byte> rndData;
            int rndSize = m_rnd.Next(m_prof.m_MaxLen - m_prof.m_MinLen) + m_prof.m_MinLen;
            for (int i = 0; i < rndSize; ++i)
            {
                rndData.push_back( m_rnd.NextByte() );
            }
            return rndData;            
        }

        void PrintStatIfNeed()
        {
            if ( !m_statTimer.TimeOut() ) return;

            m_printCount++;

            Platform::dword elapsed = m_printCount * m_prof.m_verboseTimeSec;

            QString msg = 
                QString("\nWrongL3PacketTestStatistic:\nCreated packet: %1\nBad packet: %2\nParse complete count: %3\nTime elapsed: %4 sec GenerationRate: %5 pack/sec")
                .arg(m_allPackCount)
                .arg(m_badPackCount)
                .arg(m_allPackCount - m_badPackCount)
                .arg(elapsed)
                .arg(m_allPackCount / elapsed);

            m_callback.PrintInfo(msg);
        }

        void OnRndParse()
        {
            if (!m_active) return;

            if ( m_timeout.TimeOut() ) 
            {
                m_callback.TestFinished(true, "Test Complete.");                         
                return;
            }
            
            PrintStatIfNeed();            
            
            int count = 100;

            while(--count)
            {
                try
                {
                    m_allPackCount++;
                    QVector<byte> pack = getRndData();
                    delete m_packFactory.CreateL3Pack(pack);                
                }
                catch(const ISDN::DssWarningExcept& msg)
                {
                    // normal
                    m_badPackCount++;
                }
                catch(const std::exception& e)
                {                
                    m_callback.TestFinished(false, e.what());
                }             
            }


            PutMsg(this, &Impl::OnRndParse);
        }

    public:        

        Impl(DomainClass &domain, const NObjWrongL3PacketTestSettings& prof, ITestResult& callback)
            : iCore::MsgObject(domain.getMsgThread()),
            m_infra(&m_rnd, domain.Log().getSessionInterface(), domain.Log().getLogSessionProfile()),
            m_ieFactory(m_infra),
            m_packFactory(m_infra, m_ieFactory),            
            m_prof(prof),            
            m_callback(callback),
            m_active(true)
        {            
            m_rnd.setSeed(Platform::GetSystemTickCount());
            PutMsg(this, &Impl::OnRndParse); // async start test
            m_timeout.Set(m_prof.m_Duration * 1000);
            
            if (m_prof.m_verboseTimeSec > 0) // ?
                m_statTimer.Set(m_prof.m_verboseTimeSec * 1000);
            m_badPackCount = 0;
            m_allPackCount = 0;
            m_printCount = 0;
            
            std::vector<QByteArray> p;
            FillDssPatterns(p);
            m_dataGen.reset(new RandomPatternGen(p, prof.PatternProf.getProf()));
        }

        void AbortTest()
        {
            m_active = false;
        }
    }; 

    // ------------------------------------------------------------------------------------
    // surface class

    WrongL3PacketTest::~WrongL3PacketTest()
    {
    }

    // ------------------------------------------------------------------------------------

    WrongL3PacketTest::WrongL3PacketTest( DomainClass &domain, const TNObjProfile& prof, ITestResult& callback )
        : m_impl(new Impl(domain, prof, callback))
    {}

    // ------------------------------------------------------------------------------------

    void WrongL3PacketTest::AbortTest()
    {
        m_impl->AbortTest();
    }
} // namespace IsdnTest
