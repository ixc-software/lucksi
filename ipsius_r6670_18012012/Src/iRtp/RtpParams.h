#ifndef RTPPARAMS_H
#define RTPPARAMS_H

#include "RtpSocketInterfaces.h"
#include "RtpPayload.h"

#include "Platform/PlatformTypes.h"

namespace Utils {   class MetaConfig;   };

namespace iRtp
{
    using Platform::word;

    class RtpInfraParams
    {        
        std::string m_address;
        int m_minPort;
        int m_maxPort;                

    public:
        RtpInfraParams() 
        {            
            m_minPort = 0;
            m_maxPort = 0;
        }

        RtpInfraParams(const std::string &localAddress, 
            int minPort, int maxPort) 
        {            
            m_address = localAddress;
            m_minPort = minPort;
            m_maxPort = maxPort;
            ESS_ASSERT( IsValid() );
        }


        RtpInfraParams(int minPort, int maxPort) 
        {            
            m_minPort = minPort;
            m_maxPort = maxPort;
            ESS_ASSERT( IsValid() );
        }
		
		struct Check
		{
			Check(const RtpInfraParams &par)
			{
				ESS_ASSERT( par.IsValid() );
			}
		};

        bool IsValid() const
        {
            if(m_maxPort == 0 && m_minPort == 0) return true;
            
			if(m_minPort == 0 || m_maxPort == 0) return false;

            if (m_maxPort <= m_minPort) return false;

            return !(m_minPort & 1);
        }
        int MinPort() const { return m_minPort; }
        int MaxPort() const { return m_maxPort; }

        int PortCount() const
        {
            return m_maxPort - m_minPort + 1;
        }

        void SetLocalHostAddress(const std::string &address) 
        {
            m_address = address;
        }
        const std::string &GetLocalHostAddress() const
        {
            return m_address;
        }
        void RegisterInMetaConfig(Utils::MetaConfig &cfg, 
            const std::string &shortPreffix = "");
    };

    // ---------------------------------------

    // Sequence filter settings
    struct SeqCounterSettings
    {
        SeqCounterSettings() : 
            MaxDropOut(3000),
            MaxMisOrder(100),
            MinSequential(2){}

        word MaxDropOut;
        word MaxMisOrder;
        word MinSequential;
        //word m_seqMod;
    };

    // ---------------------------------------

    struct RtpParams
    {                
        bool UseRtcp;        
        SeqCounterSettings SeqConterSettings;         
        
        bool PcTraceInd;
        bool CoreTraceInd;   
        bool TraceFull; // if false write only important message (no trace data exchange).
        
        RtpParams() : 
            UseRtcp(false), 
			PcTraceInd(false), 
			CoreTraceInd(false), 
            TraceFull(false)
        {}
    };


} // namespace iRtp

#endif

