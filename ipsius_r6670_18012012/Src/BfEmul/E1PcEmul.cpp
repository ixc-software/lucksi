#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "BfTdm/tdm.h"

#include "E1PcEmul.h"

// ----------------------------------

namespace
{
    using Platform::byte;
    using Platform::word;
    using Utils::IVirtualInvoke;

    enum
    {
        CHdlcPoolBlockSize              = 200,
        CHdlcPoolSize                   = 64,
        CHdlcMaxPacksForSingleDst       = (CHdlcPoolSize / 4),
        CHdlcAssertNoDst                = false,

        CIrqTimerIntervalMs     = 10,
        CTdmAssertNoDst         = false,
    };
    
    BfEmul::PcEmul   *PPcEmul;
    
}  // namespace


// ----------------------------------

namespace  BfEmul
{
                
    PcEmul& PcEmul::Instance()
    {
        ESS_ASSERT(PPcEmul != 0);
        
        return *PPcEmul;
    }

    void PcEmul::Init()
    {
		if(PPcEmul != 0) return;
        PPcEmul = new PcEmul();
    }

    // --------------------------------------------------------

    void PcEmul::SendHdlcPack( const std::string &src, void *p, int size )
    {
        ESS_ASSERT(src.size() > 0);
        ESS_ASSERT(p != 0);
        ESS_ASSERT(size != 0);

        std::string dst = FindDestination(src);

        if (dst.size() == 0)
        {
            if (CHdlcAssertNoDst) ESS_HALT("");
            return;
        }

        {
            Platform::MutexLocker lock(m_hdlcMutex);

            HdlcListClean(dst);

            Utils::BidirBuffer *pBuff = m_bidirPool.CreateBidirBuff();  // thread safe - ?!
            pBuff->PushBack(p, size);

            m_hdlcPacksToSend.push_back( HdlcPack(dst, pBuff) );        
        }
        
    }

    // --------------------------------------------------------

    Utils::BidirBuffer* PcEmul::RecvHdlcPack( const std::string &dst )
    {
        Platform::MutexLocker lock(m_hdlcMutex);

        HdlcPacksList::iterator i = m_hdlcPacksToSend.begin();

        while(i != m_hdlcPacksToSend.end())
        {
            if ((*i).Destination == dst)
            {
                Utils::BidirBuffer *p = (*i).PData;
                m_hdlcPacksToSend.erase(i);
                return p;
            }

            ++i;
        }

        return 0;
    }

    // --------------------------------------------------------

    iCore::MsgThread& PcEmul::SocksThread()
    {
        return m_thread;
    }

    // --------------------------------------------------------

    PcEmul::PcEmul() : 
        m_thread("PcEmul::PcEmul", Platform::Thread::LowestPriority),
        m_dummiThreadRun( MsgThreadRun(m_thread) ),
        m_bidirPool(CHdlcPoolBlockSize, CHdlcPoolSize, 0),
        m_tdm(m_thread, *this)
    {
    }

    // --------------------------------------------------------

    std::string PcEmul::FindDestination( const std::string &src )
    {
        for(int i = 0; i < m_pairs.size(); ++i)
        {
            if (m_pairs.at(i).B1 == src) return m_pairs.at(i).B2;
            if (m_pairs.at(i).B2 == src) return m_pairs.at(i).B1;
        }

        return "";
    }

    // --------------------------------------------------------

    void PcEmul::AddPair( const BoardPair &pair )
    {
        ESS_ASSERT( !pair.Empty() );
		
		ESS_ASSERT(std::find(m_pairs.begin(), m_pairs.end(), pair) == m_pairs.end());

        m_pairs.push_back(pair);
    }

	void PcEmul::DeletePair(const BoardPair &pair)
	{
        ESS_ASSERT( !pair.Empty() );

		BoardPairList::iterator i = std::find(m_pairs.begin(), m_pairs.end(), pair);        
		ESS_ASSERT(i != m_pairs.end());

        m_pairs.erase(i);
	}

    // --------------------------------------------------------

    // This function check condition when list has too many packets for dst. 
    // This can be when dst side don't read packets and blocks in m_bidirPool is out
    void PcEmul::HdlcListClean( const std::string &dst )
    {
        // first cycle -- count packets for dst
        HdlcPacksList::iterator i = m_hdlcPacksToSend.begin();
        int count = 0;

        while(i != m_hdlcPacksToSend.end())
        {
            if ((*i).Destination == dst) ++count;

            ++i;
        }

        // second cycle -- clean
        if (count < CHdlcMaxPacksForSingleDst) return;

        i = m_hdlcPacksToSend.begin();
        
        while(i != m_hdlcPacksToSend.end())
        {
            if ((*i).Destination == dst)
            {
                (*i).FreeData();
                i = m_hdlcPacksToSend.erase(i);
                continue;
            }

            ++i;
        }

    }

}  // namespace E1App

// --------------------------------------------------------

namespace  BfEmul
{

    // Block that was written on the read/write stage on the next iteration will be given to the paired object for reading
    // This class emulate TDM channel 0, channel 1 is unused
    class TdmEmulPoint : public BfTdm::IUserBlockInterface
    {
        enum 
        {
            CChannelsCount = 32,
        };

        // unused channel 1 
        class UnusedUserBlock : public BfTdm::IUserBlockInterface
        {

        // IUserBlockInterface impl
        private:

            void CopyRxTSBlockTo(byte timeSlot, byte *tsBuff, word offset, word tsSampleCount)
            {
                ESS_UNIMPLEMENTED;
            }

            void CopyRxTSBlockTo(byte timeSlot, word *tsBuff, word offset, word tsSampleCount)
            {
                ESS_UNIMPLEMENTED;
            }

            void PutTxTSBlockFrom(byte timeSlot, const byte *tsBuff, word offset, word tsSampleCount)
            {
                ESS_UNIMPLEMENTED;
            }

            void PutTxTSBlockFrom(byte timeSlot, const word *tsBuff, word offset, word tsSampleCount)
            {
                ESS_UNIMPLEMENTED;
            }

            void CopyTxTSBlockTo(byte timeSlot, byte *tsBuff, word buffSize)
            {
                ESS_UNIMPLEMENTED;
            }

            void UserFree()
            {
                // nothing
            }

            word GetTSBlockSize() const
            {
                ESS_UNIMPLEMENTED;
                return 0;
            }

            void CopySyncTxTo(byte timeSlot, byte *tsBuff,  word offset, word tsSampleCount)
            {
                ESS_UNIMPLEMENTED;
            }


            void CopySyncTxTo(byte timeSlot, word *tsBuff,  word offset, word tsSampleCount)
            {
                ESS_UNIMPLEMENTED;
            }

            const word *GetRxBlock(word frameNum) const 
            { 
                ESS_UNIMPLEMENTED;
                return 0; 
            }

            word *GetTxBlock(word frameNum) const 
            { 
                ESS_UNIMPLEMENTED;
                return 0; 
            }

        };

        IVirtualInvoke    *m_pBlockClosed;
        BfTdm::TdmProfile m_profile;
        BfTdm::ITdmObserver &m_observer;
        UnusedUserBlock     m_unusedBlock;

        bool              m_blockProcessActive;
        std::vector<byte> m_rx, m_tx;            

        int GetBlockAccessIndex(byte timeSlot, const byte *tsBuff, word offset, word tsSampleCount)
        {
            ESS_ASSERT(m_blockProcessActive);
            ESS_ASSERT(offset + tsSampleCount <= GetTSBlockSize());
            ESS_ASSERT(timeSlot < CChannelsCount);
            ESS_ASSERT(tsBuff != 0);

            return (timeSlot * GetTSBlockSize()) + offset;
        }

    // IUserBlockInterface impl
    private:

        // получить принятые данные
        void CopyRxTSBlockTo(byte timeSlot, byte *tsBuff, word offset, word tsSampleCount)
        {
            int indx = GetBlockAccessIndex(timeSlot, tsBuff, offset, tsSampleCount);
            std::memcpy(tsBuff, &m_rx[indx], tsSampleCount);
        }

        // данные на отправку 
        void PutTxTSBlockFrom(byte timeSlot, const byte *tsBuff, word offset, word tsSampleCount)
        {
            int indx = GetBlockAccessIndex(timeSlot, tsBuff, offset, tsSampleCount);
            std::memcpy(&m_tx[indx], tsBuff, tsSampleCount);
        }

        // получить принятые данные
        void CopyRxTSBlockTo(byte timeSlot, word *tsBuff, word offset, word tsSampleCount)
        {
            ESS_UNIMPLEMENTED;
        }

        // данные на отправку 
        void PutTxTSBlockFrom(byte timeSlot, const word *tsBuff, word offset, word tsSampleCount)
        {            
            ESS_UNIMPLEMENTED;
        }

        void UserFree()
        {
            ESS_ASSERT(m_blockProcessActive);

            m_blockProcessActive = false;
            m_pBlockClosed->Execute();
        }

        word GetTSBlockSize() const
        {
            ESS_ASSERT(m_blockProcessActive);

            return m_profile.GetDMABlockCapacity();
        }

        void CopySyncTxTo(byte timeSlot, byte *tsBuff,  word offset, word tsSampleCount)
        {
            // unimplemented, just fill with zeroes
            for(int i = 0; i < tsSampleCount; ++i)
            {
                tsBuff[i] = 0;
            }
        }
        void CopySyncTxTo(byte timeSlot, word *tsBuff,  word offset, word tsSampleCount)
        {
            ESS_UNIMPLEMENTED;         
        }

        const word *GetRxBlock(word frameNum) const 
        { 
            ESS_UNIMPLEMENTED;
            return 0; 
        }

        word *GetTxBlock(word frameNum) const 
        { 
            ESS_UNIMPLEMENTED;
            return 0; 
        }

        void CopyTxTSBlockTo(byte timeSlot, byte *tsBuff, word buffSize)
        {
            ESS_UNIMPLEMENTED;
        }

    public:

        TdmEmulPoint(const BfTdm::TdmProfile &profile, 
                     BfTdm::ITdmObserver &observer, 
                     IVirtualInvoke *pBlockClosed) :
          m_pBlockClosed(pBlockClosed),
          m_profile(profile),
          m_observer(observer),
          m_blockProcessActive(false)
        {
            int capacity = m_profile.GetDMABlockCapacity() * CChannelsCount;

            m_rx.resize(capacity);
            m_tx.resize(capacity);
        }

        const BfTdm::TdmProfile& Profile() const { return m_profile; }
        int DMABlockCapacity() const { return m_profile.GetDMABlockCapacity(); }
        const std::string& Name() const { return m_profile.Name(); }

        void StartReadWriteCycle()
        {
            ESS_ASSERT(!m_blockProcessActive);

            m_blockProcessActive = true;
            m_observer.NewBlockProcess(0, *this, m_unusedBlock, false);
        }

        const std::vector<byte>& TxBlock() const { return m_tx; }

        void RxBlock(const std::vector<byte> &block) 
        { 
            ESS_ASSERT( m_rx.size() == block.size() );

            m_rx = block; 
        }

        bool BlockProcessActive() const { return m_blockProcessActive; }
    };

}


// --------------------------------------------------------

namespace  BfEmul
{

    static dword GetMs()
    {
        return Platform::GetSystemMicrosecTickCount() / 1000;
    }

    TdmEmul::TdmEmul(iCore::MsgThread &thread, IPairResolve &resolve) :
        iCore::MsgObject(thread),
        m_resolve(resolve),
        m_tdmBlockClosed( Utils::VirtualInvoke::Create(&TdmEmul::TdmBlockClosed, *this) ),
        m_irqTimer(this, &TdmEmul::OnIrqTimer),
        m_blockProcessActive(false)
    {
        m_irqTimer.Start(CIrqTimerIntervalMs, true);

        m_lastIrqTime = GetMs();
    }

    // -------------------------------------------------------------

    void TdmEmul::OnIrqTimer( iCore::MsgTimer *pTimer )
    {
        /*
        Platform::MutexLocker lock(m_mutex);        

        TryStartBlockProcess(); */

        OnTdmBlockClosed();
    }

    // -------------------------------------------------------------

    TdmEmulPoint* TdmEmul::RegisterTdmPoint( const BfTdm::TdmProfile &profile, BfTdm::ITdmObserver &observer )
    {
        Platform::MutexLocker lock(m_mutex);  

        // verify name
        ESS_ASSERT(profile.Name().size() > 0);
        ESS_ASSERT(FindPointByName( profile.Name() ) == 0); // dublicates check

        // All DMABlockCapacity must be equal
        if (m_tdm.Size() > 0) ESS_ASSERT(profile.GetDMABlockCapacity() == m_tdm[0]->DMABlockCapacity());

        // Register
        TdmEmulPoint *p = new TdmEmulPoint(profile, observer, m_tdmBlockClosed.get());
        m_tdm.Add(p);

        return p;
    }

    // -------------------------------------------------------------

    void TdmEmul::UnregisterTdmPoint( TdmEmulPoint *p )
    {
        Platform::MutexLocker lock(m_mutex);        

        int indx = m_tdm.Find(p);
        ESS_ASSERT(indx >= 0);
        m_tdm.Delete(indx);
    }

    // -------------------------------------------------------------

    void TdmEmul::TryStartBlockProcess()
    {       
        if (m_blockProcessActive) return;  // TODO -- processing timeout control - ?!

        if (m_tdm.Size() == 0) return;

        int period = (m_tdm[0]->DMABlockCapacity()) / 8;
        dword currTime = GetMs();

        if ((currTime - m_lastIrqTime) >= period)
        {
            m_blockProcessActive = true;

            // begin read/write cycle on all registred TDM
            for(int i = 0; i < m_tdm.Size(); ++i)
            {
                m_tdm[i]->StartReadWriteCycle();
            }

            m_lastIrqTime = m_lastIrqTime + period;
        }

    }

    // --------------------------------------------------------

    void TdmEmul::TdmBlockClosed()
    {
        PutMsg(this, &TdmEmul::OnTdmBlockClosed);
    }

    void TdmEmul::OnTdmBlockClosed()
    {
        Platform::MutexLocker lock(m_mutex);        

        if (m_blockProcessActive)
        {
            TryCloseBlockProcess();
        }
       
        // try to begin new cycle
        TryStartBlockProcess();
    }

    // --------------------------------------------------------

    void TdmEmul::TryCloseBlockProcess()
    {
        // is all blocks done?
        for(int i = 0; i < m_tdm.Size(); ++i)
        {
            if (m_tdm[i]->BlockProcessActive()) return;
        }

        // close block process cycle, move tx blocks from source to destination
        m_blockProcessActive = false;

        for(int i = 0; i < m_tdm.Size(); ++i)
        {
            std::string dstName = m_resolve.FindDestination(m_tdm[i]->Name());
            TdmEmulPoint *pDst = FindPointByName(dstName);

            if (pDst == 0)
            {
                if (CTdmAssertNoDst) ESS_HALT("");
                continue;
            }

            // move block src.TX -> dst.RX
            pDst->RxBlock( m_tdm[i]->TxBlock() );
        }
    }

    // --------------------------------------------------------

    TdmEmulPoint* TdmEmul::FindPointByName( const std::string &name )
    {
        for(int i = 0; i < m_tdm.Size(); ++i)
        {
            if (m_tdm[i]->Name() == name) return m_tdm[i];
        }

        return 0;
    }

    // --------------------------------------------------------

    TdmEmul::~TdmEmul()
    {
        ESS_ASSERT(m_tdm.Size() == 0);
    }


}  // namespace E1App

