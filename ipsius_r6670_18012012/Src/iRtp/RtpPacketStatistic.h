#ifndef RTPPACKETSTATISTIC_H
#define RTPPACKETSTATISTIC_H

#include "RtpHeaderForUser.h"

namespace iRtp
{
    //----------------------------------------------------------

    //todo: move RtpPacketStatistic to other header
    class RtpPacketStatistic
    {
        int m_packetCount;          // count of processed packet
        dword m_dTsAvg;             // average delta timestamp
        bool m_dTsIsConst;          // delta timestamp is const
        int m_packetSizeAvg;        // average packet size
        bool m_packetSizeIsConst;   // packet size is const        

        // информация о последнем пакете
        RtpHeaderForUser m_lastHeader;
        int m_lastSize;        

        void EvalAvgSize(int newSize);

        void EvalAvgTs(dword newTs);

    public:

        RtpPacketStatistic();

        void Reset();

        void Add(int size, RtpHeaderForUser header);

        int getCount() const;

        dword getAvgTs() const;

        bool TsIsConst() const;

        int getAvgSize() const;

        bool SizeIsConst() const;                

    };
} // namespace iRtp

#endif
