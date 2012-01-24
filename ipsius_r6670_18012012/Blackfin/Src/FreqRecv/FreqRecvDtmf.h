#ifndef __FREQRECVDTMF__
#define __FREQRECVDTMF__

#include "iDSP/DtmfDetector.h"

#include "itdmfreqrecv.h"

namespace FreqRecv
{

    // control DTMF recv for Sfx client
    /*
    class RecvDtmfSfx : public ITdmFreqRecv, boost::noncopyable
    {

    // ITdmFreqRecv impl
    private:

        bool Equal(bool useInternal, const std::string &params)
        {
            ESS_UNIMPLEMENTED;
        }

        void On()
        {
            ESS_UNIMPLEMENTED;
        }

        void Off()
        {
            ESS_UNIMPLEMENTED;
        }

        bool WaitData()
        {
            ESS_UNIMPLEMENTED;
        }

        void ProcessData(const std::vector<Platform::int16> &data)
        {
            ESS_UNIMPLEMENTED;
        }

        std::string PeekEvent()
        {
            ESS_UNIMPLEMENTED;
        }


    public:

        RecvDtmfSfx(const std::string &params)
        {
            // ...
        }

    }; */

    // ------------------------------------------------------

    // control DTMF recv for Sfx client
    class RecvDtmfLocal : public ITdmFreqRecvLocal, boost::noncopyable
    {
        bool m_active;
        iDSP::DtmfDetector m_detector;
        std::string m_evBuff;

        void ClearData()
        {
            m_detector.ReadDetected();  
            m_evBuff.clear();
        }

    // ITdmFreqRecvLocal impl
    private:

        bool Equal(const std::string &params)
        {
            if (params != "") return false;
            return true;
        }

        void On()
        {
            if (m_active) return;

            ClearData();
            m_detector.Reset();
            m_active = true;
        }

        void Off()
        {
            m_active = false;

            ClearData();
        }

        bool WaitData()
        {
            return m_active;
        }

        void ProcessData(const std::vector<Platform::int16> &data)
        {
            ESS_ASSERT(m_active);

            m_detector.Process(data);
        }

        std::string PeekEvent()
        {
            if (!m_active) return "";

            m_evBuff += m_detector.ReadDetected();
            if (m_evBuff.empty()) return "";

            std::string res;
            res += m_evBuff[0];
            m_evBuff = m_evBuff.substr(1);

            return res;
        }

    public:

        RecvDtmfLocal(const std::string &params) : m_active(false)
        {
        }

    };

    
}  // namespace FreqRecv

#endif
