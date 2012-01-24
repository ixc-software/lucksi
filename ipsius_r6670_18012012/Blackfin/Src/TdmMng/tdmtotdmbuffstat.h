#ifndef __TDMTOTDMBUFFSTAT__
#define __TDMTOTDMBUFFSTAT__

namespace TdmMng
{
    
    struct TdmToTdmBuffStat
    {
        int SlipsRecv;
        int SlipsSend;
        int ReadCount;

        TdmToTdmBuffStat()
        {
            Clear();
        }

        void Clear()
        {
            SlipsRecv = 0;
            SlipsSend = 0;
            ReadCount = 0;
        }

        void SummWith(const TdmToTdmBuffStat& rhs)
        {
            SlipsRecv += rhs.SlipsRecv;
            SlipsSend += rhs.SlipsSend;
            ReadCount += rhs.ReadCount;
        }

		std::string ToString(bool withLf = true) const
		{				
            std::string lf = withLf ? "\n" : "  ";
           std::ostringstream ss;
            ss << "ReadCount = " << ReadCount << lf 
				<< "SlipRecv = " << SlipsRecv << lf
				<< "SlipSend = " << SlipsSend;
            return ss.str();
        }

		template<class Data, class TStream>
			static void Serialize(Data &data, TStream &s)
        {
            s
                << data.SlipsRecv
                << data.SlipsSend
                << data.ReadCount;
        }

    };
    
}  // namespace TdmMng

#endif
