#ifndef __TDMECHOSUPPRESS__
#define __TDMECHOSUPPRESS__

#include "Utils/ManagedList.h"
#include "iDSP/EchoCancel.h"
#include "iVDK/CriticalRegion.h"
#include "BfTdm/TdmProfile.h"
#include "iDSP/g711codec.h"

#include "TdmEchoSuppressDoc.h"
#include "tdmsynctxcopy.h"
#include "ITdmDataCapture.h"

namespace TdmMng
{
    using boost::shared_ptr;

    // класс, с расширенной информацией для BfTdm::IUserBlockInterface
    class TdmUserBlockExt : boost::noncopyable
    {
        TdmSyncTxCopy m_syncTx;

    public:

        TdmUserBlockExt(int channelsCount, int channelBuffSize, iDSP::ICodec &codec) : 
            m_syncTx(channelsCount, channelBuffSize, codec)
        {
            // ...
        }

        TdmSyncTxCopy& SyncTx() { return m_syncTx; }

    };

    class ChannelEcho;

    // ------------------------------------------------------------
    
    // класс, отвечающих на уровне TdmHAL за подавление эха на одном SPORT канале
    // описание работы см. TdmEchoSuppressDoc.h
    class TdmEchoSuppress : boost::noncopyable
    {
        Utils::ManagedList<TdmUserBlockExt> m_extBlocks;
        int m_extBlocksUsedCount;

        // index as channel number
        std::vector< shared_ptr<ChannelEcho> > m_echo;

        boost::scoped_ptr<iDSP::ICodec> m_codec;

    public:

        TdmEchoSuppress(int maxChannels, const BfTdm::TdmProfile &tdmProfile, bool useAlaw);

        bool ApplyEcho(byte chNumber, Utils::BidirBuffer& buff, TdmUserBlockExt *pExt,
            ITdmDataCapture *pCapture);

        const iDSP::ICodec& getCodec() const;

        // sync against IRQ!
        void EnableForChannel(int chNumber, int taps);

        // sync against IRQ!
        void DisableForChannel(int chNumber);

        // IRQ context!!
        void IrqEchoDataCopy(BfTdm::IUserBlockInterface &ch);
                
    };
    
}  // namespace TdmMng

#endif
