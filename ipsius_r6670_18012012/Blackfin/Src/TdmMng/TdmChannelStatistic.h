#ifndef __TDMCHANNELSTATISTIC__
#define __TDMCHANNELSTATISTIC__

#include "iRtp/ssrcstatistic.h"
#include "SafeBiProto/ISerialazable.h"

#include "RtpToTdmBufferStat.h"
#include "tdmtotdmbuffstat.h"


namespace TdmMng
{
    
    struct TdmChannelStatistic : public SBProto::ISerialazable
    {
        RtpToTdmBufferStat RtpBuffStat;
        TdmToTdmBuffStat TdmBuffStat;
        iRtp::RtpStatistic RtpStat;
		
		std::string ToString(const char *sep = "\n") const
        {
            std::ostringstream ss;

            ss << "RTP buff: { " << RtpBuffStat.ToString() << "}" << sep
                << "TDM buff: {"  << TdmBuffStat.ToString() << "}" << sep
                << "RTP session: { " << RtpStat.ToString() << "}";

            return ss.str();
        }

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
        {
			RtpToTdmBufferStat::Serialize(data.RtpBuffStat, s);
			TdmToTdmBuffStat::Serialize(data.TdmBuffStat, s);
			iRtp::RtpStatistic::Serialize(data.RtpStat, s);
        }

    };
    
}  // namespace TdmMng


#endif
