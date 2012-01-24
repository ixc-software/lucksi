#ifndef __ITDMDATACAPTURE__
#define __ITDMDATACAPTURE__

#include "Utils/IBasicInterface.h"
#include "Utils/BidirBuffer.h"

#include "BfTdm/IUserBlockInterface.h"

namespace TdmMng
{
    using Platform::byte;
    
    class ITdmDataCapture : public Utils::IBasicInterface
    {

    // echo mode
    public:

        virtual void SetEchoRxTx(const Utils::BidirBuffer &rx, const std::vector<byte> &tx) = 0;
        virtual void SetEchoFixedRx(const Utils::BidirBuffer &rxFixed) = 0;

    // non-echo mode
    public:

        // tdm  -> rtp
        virtual void SetRxData(const Utils::BidirBuffer &buff) = 0;     

        // rtp  -> tdm; grab data from TDM TX buffer
        virtual void SetTxData(BfTdm::IUserBlockInterface &block) = 0;  

    };

    
}  // namespace TdmMng

#endif
