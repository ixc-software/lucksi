#include "stdafx.h"
#include "SuperToneDetect.h"

#include "iDSP/span_super_tone_rx.h"

//#include "Utils/ExecutorManager.h"

namespace 
{
    
    
} // namespace 

namespace iDSP
{

    // callback function:
    void SignalDetect(void *data, int code, int level, int delay/*?*/)
    {
        static_cast<iDSP::SuperToneDetect*>(data)->OnSignal(code, level, delay);
    }
    /*
    void SegmentDetect(void *data, int f1, int f2, int duration)
    {
        static_cast<iDSP::SuperToneDetect*>(data)->OnSegment(f1, f2, duration);        
    }
    */

    // ------------------------------------------------------------------------------------
    

    class SuperToneDetect::State
    {
        super_tone_rx_descriptor_t m_desc;
        super_tone_rx_state_t *m_super;
        SuperToneDetect& m_callback;
    public:
        State(SuperToneDetect& callback) : m_callback(callback)
        {
            super_tone_rx_make_descriptor(&m_desc);
            m_super = 0;
        }
        ~State()
        {
            super_tone_rx_free(m_super);
        }
        super_tone_rx_descriptor_t* Descr()
        {
            return &m_desc;
        }
        void ResetSuper()
        {
            super_tone_rx_free(m_super);
            m_super = 0;

            // Init detector, set callback        
            m_super = super_tone_rx_init(0, &m_desc, SignalDetect, &m_callback);
            //super_tone_rx_segment_callback(m_super, SegmentDetect);  
        } 
        void UpdateState(const Platform::int16* data, int size)
        {
            super_tone_rx(m_super, data, size);
        }

    };

    // ------------------------------------------------------------------------------------

    class SuperToneDetect::SignalInfo : public SignalHandle, boost::noncopyable
    {        
        int m_spanIndex;
        Utils::SafeRef<ISuperToneEvents> m_observer;

    public:
        SignalInfo(const SupertoneSignal& signal, int spanIndex, Utils::SafeRef<ISuperToneEvents> observer)
            : SignalHandle(signal),
            m_spanIndex(spanIndex),
            m_observer(observer)
        {}

        int SpanIndex() const
        {
            return m_spanIndex;
        }        

        void SendSignalToObserver()
        {
            m_observer->OnSignal(this);
        }
    }; 

    // ------------------------------------------------------------------------------------

    SuperToneDetect::SuperToneDetect()
    {
        Reset();
    }

    // ------------------------------------------------------------------------------------

    SuperToneDetect::~SuperToneDetect()
    {
    }

    // ------------------------------------------------------------------------------------

    SignalHandle* SuperToneDetect::Add(Utils::SafeRef<ISuperToneEvents> observer, const SupertoneSignal& signal)
    {
        ESS_ASSERT(!m_processStarted);

        // todo защита от одинаковых и подобных сигналов        

        int spanIndex = super_tone_rx_add_tone(m_spanState->Descr());              

        const int CLenTolerance = 10;
        for (int i = 0; i < signal.SegmentCount(); ++i)
        {           
            super_tone_rx_add_element(m_spanState->Descr(), spanIndex, 
                signal.SegmentAt(i).Freq1,
                signal.SegmentAt(i).Freq2,
                signal.SegmentAt(i).MinDuration(CLenTolerance),
                signal.SegmentAt(i).MaxDuration(CLenTolerance)
                );
        }    

        m_spanState->ResetSuper();
        
        m_signals.Add( new SignalInfo(signal, spanIndex, observer) );
        return m_signals.Back();
    }

    // ------------------------------------------------------------------------------------

    void SuperToneDetect::OnSignal( int code, int, int )
    {
        if (code == -1)  
            return; //?

        // find signal
        int signalIndex = -1;
        for (int i = 0; i < m_signals.Size(); ++i)
        {
            if (m_signals[i]->SpanIndex() != code) continue;
            signalIndex = m_signals[i]->SpanIndex();
            break;
        }
                
        ESS_ASSERT(signalIndex != -1);
        
        m_signals[signalIndex]->SendSignalToObserver();
    }

    void SuperToneDetect::Reset()
    {
        m_spanState.reset(new State(*this));
        m_processStarted = false;
    }

    // main process 
    void SuperToneDetect::Process(const Platform::int16* data, int size )
    {
        m_processStarted = true;
        m_spanState->UpdateState(data, size);
    }

    void SuperToneDetect::Process(const std::vector<Platform::int16>& data )
    {
        if (data.empty()) return;
        Process(&data[0], data.size());
    }

    void SuperToneDetect::Process( Platform::int16 sample )
    {
        Process(&sample, 1);
    }
    
} // namespace iDSP
