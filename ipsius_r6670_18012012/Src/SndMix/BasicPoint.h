#ifndef __SNDMIXBASICPOINT__
#define __SNDMIXBASICPOINT__

#include "iRtp/IRtpInfra.h"
#include "iRtp/RtpConstants.h"
#include "iLog/LogWrapper.h"

#include "IBasicPoint.h"
#include "MixPointImpl.h"

namespace SndMix
{
    using boost::scoped_ptr;

    class BasicPoint : public boost::noncopyable
    {
        iRtp::IRtpInfra &m_rtpInfra;
        const std::string m_pointId;
        const int m_handle;        
        bool m_finalized;
        scoped_ptr<iLogW::LogSession> m_log; 
        iLogW::LogRecordTag m_infoTag;

        scoped_ptr<MixPointImpl> m_point;
        scoped_ptr<ICodec> m_codec;

        std::vector<int16> m_tempBuff;  // global for no heap trashing 
        Platform::dword m_rtpSendCounter;

        virtual void BeforeAddToConf(PointMode mode) { /* nothing */ }
        virtual void AfterAddToConf()                { /* nothing */ }
        virtual void AfterRemoveFromConf()           { /* nothing */ }

        // return buffer only if enough data avaible 
        Utils::BidirBuffer* RtpDataPeek()  // no throw 
        {
            ESS_ASSERT(m_point && m_codec);

            int dataSize = m_codec->TypicalBlockSizeForEncode();
            if (m_point->FromMixerBuff().AvaibleForRead() < dataSize)
            {
                return 0;
            }

            return DataPeek(dataSize, iRtp::CRtpHeaderSize);
        }

    protected:

        BasicPoint(iRtp::IRtpInfra &rtpInfra, iLogW::ILogSessionCreator &logCreator,
            const std::string &pointType, int handle) : 
            m_rtpInfra(rtpInfra),
            m_pointId(pointType + "_" + Utils::IntToString(handle)),
            m_handle(handle), 
            m_finalized(false),
            m_log( logCreator.CreateSession("Point" + m_pointId, true) ),
            m_infoTag( m_log->RegisterRecordKindStr("Info", true) ),
            m_rtpSendCounter(0)
        {
        }

        ~BasicPoint()
        {
            // Finalize();  // <- we can't

            ESS_ASSERT(m_finalized);
            ESS_ASSERT(m_point == 0);
        }

        // this method must be called from every childs of BasicPoint 'couse UnregisterPoint 
        // do call to virtual method AfterRemoveFromConf()
        void Finalize()
        {
            ESS_ASSERT(!m_finalized);
            m_finalized = true;

            if (m_point != 0)
            {
                UnregisterPoint();
            }
        }

        MixPointImpl* Point() // can be null, ok 
        {
            return m_point.get(); 
        }

        ICodec& Codec()       
        {
            return *m_codec;
        }

        iLogW::LogSession& Log() const 
        {
            return *m_log;
        }

        const iLogW::LogRecordTag& InfoTag() const
        {
            return m_infoTag;
        }

        void RegisterPoint(SafeRef<IMixPointOwner> owner, PointMode mode, const MixPointImplProfile &profile)
        {
            if (m_point != 0)   ESS_THROW(Error::PointAlreadyInConf);

            BeforeAddToConf(mode);

            m_point.reset(
                new MixPointImpl(m_pointId, m_handle, mode, owner, profile) 
                );

            AfterAddToConf();
        }

        void UnregisterPoint()
        {
            if (m_point == 0) ESS_THROW(Error::PointNotInConf);

            m_point.reset();

            AfterRemoveFromConf();
        }

        void InitCodec(const std::string &codecName, 
                       const std::string &codecParams)
        {
            ESS_ASSERT(m_codec == 0);

            const ICodecInfo *pInfo = CodecMng::Find(codecName, true);
            m_codec.reset( pInfo->CreateCodec(codecParams) );

            ESS_ASSERT(m_codec != 0);
        }

        void InitCodec(bool aLow)
        {
            iRtp::RtpPayload payload = aLow ? iRtp::PCMA : iRtp::PCMU;
            const ICodecInfo *pI = CodecMng::Find(payload, true);            
            InitCodec(pI->Name(), "");
        }

        // forced, never return 0
        Utils::BidirBuffer* DataPeek(int dataSize, int minBuffOffset = 0) // no throw 
        {
            ESS_ASSERT(m_point && m_codec);

            // from cyclic buffer to temp buffer
            m_point->FromMixerBuff().ReadBlock(m_tempBuff, dataSize);

            // encode from temp buffer to BidirBuffer
            Utils::BidirBuffer *pBuff = m_rtpInfra.CreateBuff();
            ESS_ASSERT(pBuff != 0);
            
            if (pBuff->GetDataOffset() < minBuffOffset) 
            {
                pBuff->SetDataOffset(minBuffOffset);
            }

            int maxOutput;
            byte *pOutput = pBuff->BackWritePointer(maxOutput);
            ESS_ASSERT(pOutput != 0);

            int encoded = m_codec->Encode(&m_tempBuff[0], m_tempBuff.size(), pOutput, maxOutput);
            ESS_ASSERT(encoded > 0 && encoded <= maxOutput);

            pBuff->AddSpaceBack(encoded);

            return pBuff;
        }

        void WriteBuff(const Utils::BidirBuffer &buff)
        {
            ESS_ASSERT(m_point && m_codec);            

            // decode to temp buffer
            m_tempBuff.resize( buff.Size() );  // TODO -- fix it for compression codec
            int decoded = m_codec->Decode( buff.Front(), buff.Size(), 
                                           &m_tempBuff[0], m_tempBuff.size() );
            ESS_ASSERT(decoded > 0 && decoded <= m_tempBuff.size());
            m_tempBuff.resize(decoded);

            // put temp buffer to cyclic buffer
            m_point->ToMixerBuff().WriteBlock(m_tempBuff);
        }

        bool RtpSendPacket(iRtp::RtpCoreSession &rtp, bool sendActive)
        {
            try
            {
                Utils::BidirBuffer *pBuff = RtpDataPeek();
                if (pBuff == 0) return false;

                if (sendActive)
                {
                    int size = pBuff->Size();
                    rtp.TxData(true /* unused */, m_rtpSendCounter, pBuff);
                    m_rtpSendCounter += size;
                }
                else
                {
                    delete pBuff;
                }

            }
            catch(/* const */ std::exception &e)
            {                    
                ESS_UNEXPECTED_EXCEPTION(e); // no throw block 'couse pBuff will be leaked
            }

            return true;
        }

        void RtpGetPacket(Utils::BidirBuffer* pBuff, iRtp::RtpHeaderForUser header)
        {
            ESS_ASSERT(pBuff != 0);
            ESS_ASSERT(pBuff->Size() > 0);

            scoped_ptr<Utils::BidirBuffer> p( pBuff );

            if ( header.Payload != Codec().DecoderInfo().PayloadType() )
            {
                // TODO push error - ?
                return;
            }

            if (Point() == 0)
            {
                // TODO push error - ?
                return;
            }

            // put data to mixer
            WriteBuff(*pBuff);
        }

    };



}  // namespace SndMix


#endif

