#ifndef SUPERTONEDETECT_H
#define SUPERTONEDETECT_H

#include "Utils/ErrorsSubsystem.h" // signal

#include "Utils/ManagedList.h"
#include "Utils/SafeRef.h"

namespace iDSP
{      

    class SupertoneSignal
    {            
    public:
        
        struct Segment
        {
            int Freq1;   // if 0 is silence, Freq2 ignored
            int Freq2;   // if 0 is silence
            int Level1;  // db
            int Level2;  // db
            int LengthMs;
            //int tolerancePercent ?

            Segment(int f1, int f2, int l1, int l2, int lengthMs)
            {
                ESS_ASSERT(lengthMs > 0);
                // todo validate level

                Freq1 = f1;
                Freq2 = f2;
                Level1 = l1;
                Level2 = l2;
                LengthMs = lengthMs;
            }  

            int MinDuration(int tolerancePercent) const
            {
                return LengthMs - LengthMs * tolerancePercent / 100 - 30; // 30 ? 
            }

            int MaxDuration(int tolerancePercent) const
            {
                return LengthMs + LengthMs * tolerancePercent / 100 + 30; // 30 ? 
            }
        };

        SupertoneSignal(int cycles = -1/*endless*/) : m_cycles(cycles)
        {            
        }

        void AddSegment(int f1, int f2, int l1, int l2, int lengthMs)
        {
            Segment newItem(f1, f2, l1, l2, lengthMs);          
            m_segments.push_back(newItem);            
        }

        int SegmentCount() const
        {
            return m_segments.size();
        }

        const Segment& SegmentAt(int i) const
        {
            ESS_ASSERT(i < m_segments.size());
            return m_segments[i];
        }

        int Cycles() const
        {
            return m_cycles;
        }      


    private:
        std::vector<Segment> m_segments;
        int m_cycles; // -1 endless
    };

    // ------------------------------------------------------------------------------------

    class SignalHandle
    {   
        SupertoneSignal m_signal;
    protected:
        SignalHandle(const SupertoneSignal& signal) : m_signal(signal) {}
    public:
        //virtual const SupertoneSignal& getSignal() const = 0;
        
        const SupertoneSignal& Signal() const;
    };

    // ------------------------------------------------------------------------------------

    class ISuperToneEvents : Utils::IBasicInterface
    {
    public:        
        virtual void OnSignal(const SignalHandle* ) = 0;
    }; 

    // ------------------------------------------------------------------------------------

    using boost::scoped_ptr;

    class SuperToneDetect : boost::noncopyable
    {        
        class State;     
        scoped_ptr<State> m_spanState;

        class SignalInfo; 
        Utils::ManagedList<SignalInfo> m_signals;

        bool m_processStarted;

    private:
        friend void SignalDetect(void*, int, int, int);        
        void OnSignal(int code, int level, int delay);        

    public:
        SuperToneDetect();
        ~SuperToneDetect();
        SignalHandle* Add(Utils::SafeRef<ISuperToneEvents> observer, const SupertoneSignal& signal);          

        void Process(Platform::int16 sample);
        void Process(const std::vector<Platform::int16>& data);
        void Process(const Platform::int16* data, int size);

        void Reset();
    };
} // namespace iDSP

#endif
