#include "stdafx.h"
#include "RtpToTdmBufferStat.h"

namespace TdmMng
{


    /*
    RtpToTdmBufferStat::RtpToTdmBufferStat()
    {
        Clear();        
    }

    void RtpToTdmBufferStat::Clear()
    {
        BufferingRtpPackSize.Reset();        
        SuccessReadFromBuffCount = 0;
        OverflowFifoCount = 0;
        GoesBufferingCount = 0;
        IncDepthCount = 0;
        RtpEventControlCounter = 0;
        RtpErrorCounter = 0;
        TotalOutByte = 0;
        SlipCount = 0;
    }

    void RtpToTdmBufferStat::SummWith(const RtpToTdmBufferStat& rhs)
    {
        BufferingRtpPackSize.Add(rhs.BufferingRtpPackSize);
        OverflowFifoCount += rhs.OverflowFifoCount;
        SuccessReadFromBuffCount += rhs.SuccessReadFromBuffCount;
        TotalOutByte += rhs.TotalOutByte;
        SlipCount += rhs.SlipCount;
        IncDepthCount += rhs.IncDepthCount;
        GoesBufferingCount += rhs.GoesBufferingCount;
        RtpEventControlCounter += rhs.RtpEventControlCounter;
        RtpErrorCounter += rhs.RtpErrorCounter;
    }

    std::string RtpToTdmBufferStat::getAsString() const
    {
        std::ostringstream ss;

        ss << "BufferingRtpPackSize: " << BufferingRtpPackSize.ToString()
            << ", OverflowFifoCount = " << OverflowFifoCount

            << ", SuccesReadFromBuffCount = " << SuccessReadFromBuffCount
            << ", TotallOutByte = " << TotalOutByte
            << ", SlipCount = " << SlipCount
            
            << ", IncDepthCount = " << IncDepthCount
            << ", GoesBufferingCount = " << GoesBufferingCount            
            << ", RtpEventControlCounter = " << RtpEventControlCounter
            << ", RtpErrorCounter = " << RtpErrorCounter;


        return ss.str();
    } */


} // namespace TdmMng

