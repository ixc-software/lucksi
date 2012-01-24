#include "stdafx.h"
#include "TestSupertoneDetect.h"

#include "SuperToneDetect.h"
#include "iDSP/g711gen.h"
#include "Utils/QtHelpers.h"

namespace 
{
    class SegmentGenerator : public boost::noncopyable
    {
        iDSP::LineMultiFreqGenerator m_gen;
        int m_restSample;
    public:
        SegmentGenerator(const iDSP::SupertoneSignal::Segment& params)
        {
            if(params.Freq1)
            {
                m_gen.Add(params.Freq1, params.Level1);
                if(params.Freq2) m_gen.Add(params.Freq2, params.Level2);
            }
            m_restSample = params.LengthMs * 8000 / 1000;
        }        
        bool Complete() const
        {
            return m_restSample == 0;
        }
        
        int Next()
        {
            --m_restSample;
            return m_gen.Next();
        }
    };

    // ------------------------------------------------------------------------------------

    class Test : public iDSP::ISuperToneEvents, virtual public Utils::SafeRefServer
    {
        bool m_silenceMode;
        QString m_testName;
        iDSP::SupertoneSignal m_signal;

        iDSP::SuperToneDetect* m_detect;
        const iDSP::SignalHandle* m_signalHandle;

        bool m_complete;

    // ISuperToneEvents
    private:
        void OnSignal(const iDSP::SignalHandle* h)
        {
            TUT_ASSERT(m_signalHandle == h);
            m_complete = true;
        }
        
    public:
        Test(QString testName, bool silenceMode) : m_silenceMode(silenceMode)
        {
            m_signalHandle = 0;
            m_detect = 0;
            m_complete = false;
            m_testName = testName;
        }

        void AddSegment(int f1, int f2, int l1, int l2, int lengthMs)
        {
            m_signal.AddSegment(f1, f2, l1, l2, lengthMs);
        }
        void RegDetector(iDSP::SuperToneDetect& detector)
        {
            ESS_ASSERT(m_signalHandle == 0);
            m_signalHandle = detector.Add(this, m_signal);
            m_detect = &detector;
        }

        void Process()
        {            
            ESS_ASSERT(m_detect != 0);
            if (!m_silenceMode) 
            {
                std::cout << "Begin test " << m_testName << "..." << std::endl;
            }

            for (int i = 0; i < m_signal.SegmentCount(); ++i)
            {
                SegmentGenerator gen(m_signal.SegmentAt(i));            
                while( !gen.Complete() ) m_detect->Process(gen.Next());                      
            }

            TUT_ASSERT(m_complete);

            if (!m_silenceMode) 
            {
                std::cout << "Test " << m_testName << " complete." << std::endl;
            }

        }
    };

} // namespace 

namespace iDSP
{
    void TestSupertoneDetect(bool silenceMode)
    {        
        Utils::ManagedList<Test> testList;
               
        // Add Signals to test
        {
            testList.Add(new Test("busy_tone", silenceMode));
            testList.Back()->AddSegment(425, 0, -12, 0, 400);
            testList.Back()->AddSegment(0, 0, 0, 0, 400);

            testList.Add(new Test("waiting_tone", silenceMode));
            testList.Back()->AddSegment(950, 0, -12, 0, 333);    
            testList.Back()->AddSegment(1400, 0, -12, 0, 333);
            testList.Back()->AddSegment(1800, 0, -12, 0, 333);    
            testList.Back()->AddSegment(0, 0, 0, 0, 1000);  

            testList.Add(new Test("ringback_tone", silenceMode));
            testList.Back()->AddSegment(425, 0, -12, 0, 800);
            testList.Back()->AddSegment(0, 0, 0, 0, 3200);   
        }              

        SuperToneDetect detector;
        // Add signals to detector, register test as detector observer, remember handler in test
        for (int i = 0; i < testList.Size(); ++i)
        {
            testList[i]->RegDetector(detector);       
        }
        
        // Run tests, validate results
        for (int i = 0; i < testList.Size(); ++i)        
        {            
            testList[i]->Process();            
        }        
    }
} // namespace iDSP
