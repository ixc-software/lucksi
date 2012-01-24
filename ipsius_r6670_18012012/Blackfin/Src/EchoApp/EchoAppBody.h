#ifndef __ECHOAPPBODY__
#define __ECHOAPPBODY__

#include "iLog/LogManager.h"
#include "iLog/LogWrapper.h"
#include "iLog/iLogSessionCreator.h"
#include "Utils/SafeRef.h"
#include "Utils/ManagedList.h"
#include "Utils/BidirBufferCreator.h"
#include "BfTdm/tdm.h"
#include "TdmMng/TdmHAL.h"
#include "TdmMng/TdmCodecs.h"
#include "TdmMng/EventsQueue.h"
#include "FreqRecv/TdmFreqRecvPool.h"

#include "EchoEvent.h"
#include "SfxProto.h"
#include "SfxProtoResp.h"

namespace EchoApp
{
    using iLogW::ILogSessionCreator;
    using Platform::word;
    using Platform::dword;
    using Platform::byte;

    enum SportCh
    {
        ChToLIU = 0,
        ChToCPU = 1,
    };

    enum
    {
        CSportNumber = 0,

        CBlockCount = 3,
        CBlockCapacity = 160,
    };

    // -----------------------------------------------------

    class IAppBody : public Utils::IBasicInterface
    {
    public:
        virtual const iDSP::ICodec& GetChCodec() const = 0;
        virtual void PushEvent(const EchoEvent &e) = 0;
        virtual Utils::BidirBuffer* CreateBidirBuffer() = 0;
    };

    // -----------------------------------------------------

    class SfxChannel : public TdmMng::IFreqRecvOwner, boost::noncopyable
    {
        IAppBody &m_owner;
        const int m_number;

        int m_echoMode;  // 0 | taps
        
        boost::scoped_ptr<Utils::BidirBuffer> m_rxCh0, m_rxCh1;

        FreqRecv::FreqRecvPoolLocal m_freqRecvs;

    // TdmMng::IFreqRecvOwner impl
    private:

        /*
        void PushFreqRecvEvent(const std::string &recvName, const std::string &event)
        {
            std::string ev = EchoAsyncEvent::CFreqRecvEvent();
            ev += EchoAsyncEvent::CSep() + recvName;

            m_owner.PushEvent( EchoEvent(ev, m_number, event) );
        } */

        void PushDialBeginEvent(const std::string &devName, int chNum)
        {
            // nothing
        }

        void PushFreqRecvEvent(const std::string &devName, int chNum,
            const std::string &freqRecvName, const std::string &data)
        {
            std::string ev = EchoAsyncEvent::CFreqRecvEvent();
            ev += EchoAsyncEvent::CSep() + freqRecvName;

            m_owner.PushEvent( EchoEvent(ev, m_number, data) );
        }

        const iDSP::ICodec& GetChCodec() const
        {
            return m_owner.GetChCodec();
        }

        FreqRecv::ITdmFreqRecvLocal* CreateFreqRecv(const std::string &name, 
            const std::string &params)
        {
            return FreqRecv::FreqRecvPoolLocal::CreateLocalFreqRecv(name, params);
        }

        Utils::BidirBuffer* CreateBidirBuffer()
        {
            return m_owner.CreateBidirBuffer();
        }

    public:

        SfxChannel(IAppBody &owner, int number, int blockSize) : 
          m_owner(owner),
          m_number(number),
          m_echoMode(0),
          m_freqRecvs(*this)
        {
            m_rxCh0.reset( owner.CreateBidirBuffer() );
            m_rxCh1.reset( owner.CreateBidirBuffer() );
        }

        void ProcessRead(SportCh ch, TdmMng::TdmReadWrapper &block)
        {
            Utils::BidirBuffer *pBuff = (ch == ChToLIU) ? m_rxCh0.get() : m_rxCh1.get();
            
            pBuff->Clear();
            block.Read(m_number, *pBuff);

            if (ch == ChToLIU)
            {
                m_freqRecvs.ProcessData(*pBuff, false, "", m_number);
            }
        }

        void ProcessWrite(SportCh ch, TdmMng::TdmWriteWrapper &block)
        {
            // cross channels write
            Utils::BidirBuffer *pBuff = (ch == ChToLIU) ? m_rxCh1.get() : m_rxCh0.get();
            
            block.Write(m_number, 0, pBuff->Front(), pBuff->Size());
        }

        void EchoMode(int val) { m_echoMode = val; }
        int EchoMode() const { return m_echoMode; }

        void SetFreqRecvMode(const std::string &recvName, 
            const std::string &params, bool enable)
        {
            iCmp::FreqRecvMode mode = enable ? iCmp::FrOn : iCmp::FrOff;
            m_freqRecvs.Command(recvName, mode, params);
        }


    };

    // -----------------------------------------------------

    class SfxChannelList 
    {
        Utils::ManagedList<SfxChannel> m_list;

        bool ChNumberOK(int number) const
        {
            if (number >= m_list.Size()) return false;
            if (m_list[number] == 0) return false;

            return true;
        }

    public:

        SfxChannelList(IAppBody &owner, dword chMask) : m_list(true, 0, true)
        {
            const int CMaxChannels = 32;

            m_list.AddEmptyItems(CMaxChannels);

            for(int i = 0; i < CMaxChannels; ++i)
            {
                if (chMask & (1 << i))
                {
                    SfxChannel *pCh = new SfxChannel(owner, i, CBlockCapacity);
                    m_list.Set(i, pCh);
                }
            }
        }

        void ProcessRead(SportCh ch, TdmMng::TdmReadWrapper &block)
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i] != 0) m_list[i]->ProcessRead(ch, block);
            }
        }

        void ProcessWrite(SportCh ch, TdmMng::TdmWriteWrapper &block)
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i] != 0) m_list[i]->ProcessWrite(ch, block);
            }
        }

        void SetEcho(TdmMng::TdmHAL &hal, TdmMng::ITdmOwner &ch, int chNum, int taps)
        {
            // verify params
            if (!ChNumberOK(chNum))
            {
                ESS_THROW_T(EchoException, EchoResp::ER_BadEchoChannel);
            }

            if (taps < 0)
            {
                ESS_THROW_T(EchoException, EchoResp::ER_BadEchoTaps);
            }

            // fix echo mode in channel
            m_list[chNum]->EchoMode(taps);

            // on/off
            if (taps > 0)
            {
                hal.StartEchoSuppress(ch, chNum, taps);
            }
            else
            {
                hal.StopEchoSuppress(ch, chNum);
            }

        }

        int ChannelsWithEcho() const
        {
            int result = 0;

            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i] != 0) 
                {
                    if (m_list[i]->EchoMode() > 0) ++result;
                }
            }

            return result;
        }

        void SetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, bool enable)
        {
            if (!ChNumberOK(chNum))
            {
                ESS_THROW_T(EchoException, EchoResp::ER_BadEchoChannel);
            }

            m_list[chNum]->SetFreqRecvMode(recvName, params, enable);
        }


    };

    // -----------------------------------------------------

    class SportChannel : public TdmMng::ITdmOwner, boost::noncopyable
    {
        enum { CFirstBlockIgnoreCount = 16 };

        SportCh m_ch; 
        SfxChannelList &m_chList;

        int m_ignoreRead, m_ignoreWrite;

    // TdmMng::ITdmOwner impl
    private:

        void TdmFrameRead(TdmMng::TdmReadWrapper &block)
        {
            if (m_ignoreRead > 0) 
            {
                m_ignoreRead--;
                return;
            }

            m_chList.ProcessRead(m_ch, block);
        }

        void TdmFrameWrite(TdmMng::TdmWriteWrapper &block)
        {
            if (m_ignoreWrite > 0)
            {
                m_ignoreWrite--;
                return;
            }

            m_chList.ProcessWrite(m_ch, block);
        }

    public:

        SportChannel(SportCh ch, SfxChannelList &chList) : m_ch(ch), m_chList(chList)
        {
            m_ignoreRead  = CFirstBlockIgnoreCount;
            m_ignoreWrite = CFirstBlockIgnoreCount;
        }
    };

    // -----------------------------------------------------

    struct TdmStat
    {
        dword BlockCollisions;        
        dword IrqCount;

        // runtime
        int   ChannelsWithEcho; 

        TdmStat()
        {
            Clear();
        }

        void Clear()
        {
            BlockCollisions = 0;
            IrqCount = 0;
            ChannelsWithEcho = 0;
        }

        std::string ToString() const
        {
            std::ostringstream oss;
            oss << "IrqCount "        << IrqCount << "; "
                << "BlockCollisions " << BlockCollisions << "; "
                << "ChannelsWithEcho " << ChannelsWithEcho;
            return oss.str();
        }
    };

    // -----------------------------------------------------
    
    class EchoAppBody : 
        public TdmMng::ITdmHalEvents,
        public virtual Utils::SafeRefServer,
        public IAppBody,
        boost::noncopyable
    {

        Utils::BidirBufferCreator<> m_buffCreator; 

        boost::scoped_ptr<iDSP::ICodec> m_codec;
        boost::scoped_ptr<TdmMng::TdmHAL> m_tdm;
        SfxChannelList m_channels;
        SportChannel m_ch0, m_ch1;
        TdmStat m_stats;
        TdmMng::EventsQueue m_queue;

        bool m_enableCopy;       

    // IAppBody impl
    private:

        const iDSP::ICodec& GetChCodec() const
        {
            return *m_codec;
        }

        void PushEvent(const EchoEvent &e)
        {
            m_queue.Push(e);
        }

        Utils::BidirBuffer* CreateBidirBuffer()
        {
            return m_buffCreator.CreateBidirBuff();
        }


    // TdmMng::ITdmHalEvents impl
    private:

        void OnTdmHalIrq(int sportNum, TdmMng::ITdmBlock &block)
        {
            ++m_stats.IrqCount;

            if (!m_enableCopy) return;

            // read stage
            block.ProcessReadFor(m_ch0, 0);
            block.ProcessReadFor(m_ch1, 0);

            // write stage
            block.ProcessWriteFor(m_ch0);
            block.ProcessWriteFor(m_ch1);
        }

        void OnTdmHalBlockCollision()
        {
            ++m_stats.BlockCollisions;
        }
        
    public:

        EchoAppBody(const CmdInitData &data, 
            iCore::MsgThread &thread,
            Utils::SafeRef<ILogSessionCreator> log, 
            bool enableTrace,
            bool tdmProfiling,
            BfTdm::ITdmProfiler* pblockProfiler = 0) :   // can throw
            m_buffCreator(CBlockCapacity, 0, 0, -1),     // unlimited blocks allocation
            m_codec( TdmMng::TdmCodec::CreateCodec(data.UseAlaw) ),
            m_channels(*this, data.VoiceChMask),
            m_ch0(ChToLIU, m_channels),
            m_ch1(ChToCPU, m_channels),
            m_queue("EchoAppBody"),
            m_enableCopy(false)            
        {
            // TDM
            {  
                BfTdm::TdmProfile p(CBlockCount, CBlockCapacity, true, 
                    BfTdm::CNoCompanding, data.T1Mode);

                TdmMng::TdmHalProfile halProfile(thread, p, p, log);

                m_tdm.reset( new TdmMng::TdmHAL(halProfile) );

                if (tdmProfiling) m_tdm->EnableProfiling(CSportNumber);
                m_tdm->SetBlockProcProfiler(pblockProfiler);

                m_tdm->RegisterEventsHandler(this);

                m_tdm->RegisterTdm(CSportNumber, ChToLIU, m_ch0, data.UseAlaw);
                m_tdm->RegisterTdm(CSportNumber, ChToCPU, m_ch1, data.UseAlaw);
            }

        }

        ~EchoAppBody()
        {
            m_tdm->UnregTdm(m_ch0); 
            m_tdm->UnregTdm(m_ch1); 
            
            m_tdm->StopDevices();
        }

        void CmdEcho(int chNum, int taps)  // can throw
        {
            m_channels.SetEcho(*m_tdm, m_ch0, chNum, taps);
        }

        void EnableCopy(bool val) { m_enableCopy = val; } 

        const TdmStat& Stats() 
        { 
            m_stats.ChannelsWithEcho = m_channels.ChannelsWithEcho();
            return m_stats; 
        }

        TdmMng::TdmProfiler::Info GetTdmProfilingInfo(bool base)
        {
            return m_tdm->GetProfilingInfo(CSportNumber, base);
        }

        void SendAllEvents(SBProto::ISafeBiProtoForSendPack &proto)
        {
            m_queue.SendAll(proto);
        }

        void SetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, bool enable)
        {
            m_channels.SetFreqRecvMode(chNum, recvName, params, enable);
        }


    };
    
}  // namespace EchoApp

#endif
