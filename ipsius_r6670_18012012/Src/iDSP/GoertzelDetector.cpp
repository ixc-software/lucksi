#include "stdafx.h"
#include "GoertzelDetector.h"

#include "span_tone_detect.h"
#include "Utils/ErrorsSubsystem.h"
#include "span_telephony.h"

namespace iDSP
{
    class GoertzelDetector::GoertzelState
    {
        goertzel_state_t m_stateImpl;
    public:
        GoertzelState(int freq, int maxSampleCount)
        {
            goertzel_descriptor_t goertzelDscr;
            make_goertzel_descriptor(&goertzelDscr, freq, maxSampleCount);
            goertzel_init(&m_stateImpl, &goertzelDscr);   
        }

        goertzel_state_t* State()
        {
            return &m_stateImpl;
        }
    };


    // ------------------------------------------------------------------------------------

    GoertzelDetector::GoertzelDetector( int freq, int maxSampleCount )
    {        
        m_state.reset( new GoertzelState(freq, maxSampleCount) );
    }

    void GoertzelDetector::Process( Platform::int16 sample )
    {                       
        Process(&sample, 1);
    }

    void GoertzelDetector::Process(const std::vector<Platform::int16>& data )
    {
        if (data.empty()) return;
        Process(&data[0], data.size());
    }

    void GoertzelDetector::Process(const Platform::int16* data, int size )
    {               
        ESS_ASSERT( goertzel_update(m_state->State(), data, size) == size && "MaxSampleCount limit") ;
    }

    int GoertzelDetector::Release() const /* return result energy & reset state */
    {
        int res = goertzel_result(m_state->State());
        goertzel_reset(m_state->State());
        return res;
    }

    GoertzelDetector::~GoertzelDetector()
    {
        // fix checked_delete in  ~scoped_ptr
    }

    int GoertzelDetector::Release_dBm0() const
    {
        int samples = m_state->State()->current_sample;        
        return To_dBm0(Release(), samples);
    }

    int GoertzelDetector::To_dBm0( float energy, int samplesCount )
    {
        return 20 * log10(sqrt(energy) * 1.4142 / 256.0 / samplesCount) + DBM0_MAX_SINE_POWER;
    }
} // namespace iDSP
