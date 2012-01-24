#include "stdafx.h"

#include "Platform/Platform.h"
#include "BfDev/BfTimerCounter.h"

#include "TdmStreamStat.h"

namespace
{

    std::string ElementTicksFix(const Utils::StatElementForInt &e, int freq)
    {
        std::ostringstream ss;

        int val =  BfDev::BfTimerCounter::TimerCounterToMcs(e.Average(), freq);

        ss << val;

        return ss.str();
    }
}

namespace TdmMng
{       

    TdmStreamStat::TdmStreamStat()
    {
        Clear();
    }

    void TdmStreamStat::Clear()
    {
        TdmBlockCollision = 0;
        FullIRQProcessingTime.Reset(); 
        ReadProcessingTime.Reset();
        ReadProcessingTimeSend.Reset();
        ReadProcessingTimeEcho.Reset(); 
        TotalEchoBlocksCount = 0;
        WriteProcessingTime.Reset();
        SocketPollingTime.Reset();        
        SocketDataIn.Reset();
    }

	std::string TdmStreamStat::ToString(int freq) const
    {
        using Utils::StatElementForInt;
        int sockOutputLevel = StatElementForInt::MaskSum + StatElementForInt::MaskCount;

        if ( FullIRQProcessingTime.Empty() ) return "(empty)";

        std::ostringstream ss;
        
        ss << "ProcessingCount: " << FullIRQProcessingTime.Count() << "\n"
            << "FullIRQProcessingTime: " << ElementTicksFix(FullIRQProcessingTime, freq) << "\n"
            << "ReadProcessingTime: " << ElementTicksFix(ReadProcessingTime, freq)
            << " (send stage " << ElementTicksFix(ReadProcessingTimeSend, freq)
            << ", echo stage " << ElementTicksFix(ReadProcessingTimeEcho, freq) << ")" << "\n"
            << "WriteProcessingTime: " << ElementTicksFix(WriteProcessingTime, freq) << "\n"
            << "SocketPollingTime: " << ElementTicksFix(SocketPollingTime, freq) << "\n"
            << "TotalEchoBlocksCount " << TotalEchoBlocksCount << "\n"
            << "SocketDataIn" << SocketDataIn.ToString(sockOutputLevel);

        if (TdmBlockCollision != 0) ss << "\n" << "Collisions " << TdmBlockCollision << " (!) ";

        return ss.str();
    }

} // namespace TdmMng

