#ifndef GOERTZELDETECTOR_H
#define GOERTZELDETECTOR_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace iDSP
{
    // todo overflow indication

    class GoertzelDetector 
        : boost::noncopyable 
    {
        class GoertzelState;
        boost::scoped_ptr<GoertzelState> m_state;        

        static int To_dBm0(float energy, int samplesCount);
        
    public:
        GoertzelDetector(int freq, int maxSampleCount = 160);
        ~GoertzelDetector();

        void Process(Platform::int16 sample);
        void Process(const std::vector<Platform::int16>& data);
        void Process(const Platform::int16* data, int size);
        
        int Release() const; // return result energy & reset state;

        enum {CdBm0_Precision = 1};
        int Release_dBm0() const; // as Release, but return dBm0. Work to slow (used float)!
    };
} // namespace iDSP

#endif
