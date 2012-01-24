#ifndef __TDMSYNCTXCOPY__
#define  __TDMSYNCTXCOPY__

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"
#include "Utils/BidirBuffer.h"

#include "iDSP/g711codec.h"
#include "BfTdm/IUserBlockInterface.h"
#include "iDSP/EchoCancel.h"

namespace TdmMng
{
    using Platform::byte;
    
    // класс содержит копию TX данных, синхронных с RX данными
    // для каналов, на которых активна эхокомпенскация
    class TdmSyncTxCopy : boost::noncopyable
    {
        class ChannelData;

        iDSP::ICodec &m_codec;
        Utils::ManagedList<ChannelData> m_channels;

    public:

        TdmSyncTxCopy(int channelsCount, int channelBuffSize, iDSP::ICodec &codec);
        ~TdmSyncTxCopy();

        void IrqEchoDataCopy(int chNum, BfTdm::IUserBlockInterface &block);

        void ClearEchoData(int chNum);

        // bool AppyEcho(byte chNum, Utils::BidirBuffer &buff, Echo::EchoCancel &echo);
        const std::vector<byte>* GetTxBlock(int chNumber);
                
    };
    
}  // namespace TdmMng

#endif
