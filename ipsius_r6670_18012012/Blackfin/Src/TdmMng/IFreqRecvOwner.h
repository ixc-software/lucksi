#ifndef __IFREQRECVOWNER__
#define __IFREQRECVOWNER__

#include "Utils/IBasicInterface.h"
#include "Utils/BidirBuffer.h"
#include "iDSP/g711codec.h"

namespace FreqRecv
{
    class ITdmFreqRecvLocal;
}

namespace TdmMng
{

    class IFreqRecvOwner : public Utils::IBasicInterface
    {
    public:

        // virtual void PushFreqRecvEvent(const std::string &recvName, const std::string &event) = 0;

        virtual void PushDialBeginEvent(const std::string &devName, int chNum) = 0;
        virtual void PushFreqRecvEvent(const std::string &devName, int chNum,
            const std::string &freqRecvName, const std::string &data) = 0;

        virtual const iDSP::ICodec& GetChCodec() const = 0;
        virtual FreqRecv::ITdmFreqRecvLocal* CreateFreqRecv(const std::string &name, 
            const std::string &params) = 0;
        virtual Utils::BidirBuffer* CreateBidirBuffer() = 0;
    };
    
}  // namespace TdmMng

#endif
