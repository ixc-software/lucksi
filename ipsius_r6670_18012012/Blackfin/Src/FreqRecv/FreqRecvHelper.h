#ifndef __FREQRECVHELPER__
#define __FREQRECVHELPER__

#include "TdmMng/EventsQueue.h"
#include "iCmp/ChMngProto.h"
#include "DrvAoz/AozShared.h"

namespace FreqRecv
{
    
    struct FreqRecvHelper
    {

        static void PushFreqRecvEvent(TdmMng::EventsQueue &q, 
            const std::string &devName, int chNum,
            const std::string &freqRecvName, const std::string &data)
        {
            using iCmp::BfTdmEvent;
            q.Push( BfTdmEvent::MakeFreqRecvEvent(devName, chNum, freqRecvName, data) );
        }

        /*
        static void PushFreqRecvEvent(TdmMng::EventsQueue &q, 
            const std::string &eventSrc,
            const std::string &freqRecvName, const std::string &data)
        {
            using iCmp::BfTdmEvent;
            q.Push( BfTdmEvent::MakeFreqRecvEvent(eventSrc, freqRecvName, data) );
        } */

        static void PushDialBeginEvent(TdmMng::EventsQueue &q, 
            const std::string &devName, int chNum)
        {
            using namespace iCmp;

            DrvAoz::AozEvent ev(chNum, BfAbEvent::CLinePulseDialBegin(), 
                                       ChMngProtoParams::ToParams(0) );            

            q.Push( DrvAoz::AbAozEvent(devName, ev) );
        }
    };
    
}  // namespace FreqRecv

#endif
