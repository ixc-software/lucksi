#include "stdafx.h"
#include "DtmfDetector.h"

#include "span_dtmf.h"
#include "Utils/GlobalMutex.h"

namespace 
{
    const std::string CPossibleSymbols("0123456789*#ABCD");                

} // namespace 

namespace iDSP
{
    void EvDigitDetected(void *user_data, const char *digits, int len)
    {
        static_cast<iDSP::DtmfDetector*>(user_data)->OnDetect(digits, len);
    }

    class DtmfDetector::DtmfState
    {
        dtmf_rx_state_t m_state;

    public:
        DtmfState(const DtmfDetectorProfile& prof, iDSP::DtmfDetector& owner)
        {            
            Utils::GlobalMutexLocker lock; // workaround for singltone goertcel in dtmf init        

            // Init state with default adjustment:
            dtmf_rx_init(&m_state, EvDigitDetected, &owner);             

            // Adjust detector:
            dtmf_rx_parms(&m_state, /*prof.Rej350_440*/false, prof.FwTwist, prof.RevTwist, prof.Threshold);
        }

        dtmf_rx_state_t* State()
        {
            return &m_state;
        }
    };

    // ------------------------------------------------------------------------------------

    DtmfDetector::DtmfDetector( const DtmfDetectorProfile& prof ) : m_prof(prof)
    {
        ESS_ASSERT(m_prof.IsValid());
        Reset();
    }

    void DtmfDetector::Process( Platform::int16 sample )
    {        
        Process(&sample, 1);
    }

    void DtmfDetector::Process(const std::vector<Platform::int16>& data )
    {
        if (data.size() == 0) return;                
        Process(&data[0], data.size());                      
    }

    void DtmfDetector::Process(const Platform::int16* data, int size )
    {
        dtmf_rx(m_stateImpl->State(), &data[0], size);            
    }    

    const std::string& DtmfDetector::ReadDetected()
    {                               
        if (m_detected.empty()) return m_detected;

        m_cash = m_detected;
        m_detected.clear();
        return m_cash;      
    }

    DtmfDetector::~DtmfDetector()
    {
    }

    void DtmfDetector::Reset()
    {
        m_stateImpl.reset(new DtmfState(m_prof, *this));        
        m_detected.clear();
    }

    void DtmfDetector::OnDetect( const char* digit, int len )
    {           
        ESS_ASSERT(len > 0);
        int off = m_detected.size();
        m_detected.append(digit, len);
        ESS_ASSERT(std::string::npos == m_detected.find_first_not_of(PossibleSymbols(), off));
    }

    const std::string& DtmfDetector::PossibleSymbols()
    {
        return CPossibleSymbols;
    }    

    Platform::dword DtmfDetector::StateCRC(Platform::dword crc) const /* evaluate current state checksum */
    {      
        const int CTabSize = 4;
        for (int i = 0; i < CTabSize; ++i)
        {
            crc = m_stateImpl->State()->row_out[i].CRC(crc);
            crc = m_stateImpl->State()->col_out[i].CRC(crc);            
        }

        return crc;
    }
} // namespace iDSP
