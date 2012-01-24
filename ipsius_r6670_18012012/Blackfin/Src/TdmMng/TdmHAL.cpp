#include "stdafx.h"

#include "iLog/LogWrapper.h"
#include "BfDev/VdkThreadDcb.h"
#include "iDSP/FullCicleGen.h"

#include "TdmHAL.h"
#include "TdmEchoSuppress.h"

using BfTdm::IUserBlockInterface;

namespace 
{
	using Platform::byte;
	using Platform::word;
	
    enum
    {
        CDisableDCB = false,
        CDisableMsg = false,
        CEnableEchoDataCopy = true,  // must be true for echo suppress
        
        CAssertOnCollision = false,

        CEnableGenCh = -1,   // -1 for disable
                
        CChLoopback = -1,   // debug, -1 for disable

        // VDK DCB params
        CDcbDeep = 128,
        CDcbMsgSize = 128,
    };


    const int CMaxChBuffSize = 512;
    byte GTSBuff[CMaxChBuffSize];

    void SinusGen(IUserBlockInterface &Ch, int chNum)
    {
        static byte table[] = {0x3a, 0xd5, 0xba, 0xa3, 0xba, 0xd5, 0x3a, 0x23};   // 1 KHz    	
    
        int tssize = Ch.GetTSBlockSize();
        ESS_ASSERT(tssize < CMaxChBuffSize);
    
        int sin_num = 0;
        for (int i = 0; i < tssize; i++)
        {
            GTSBuff[i] = table[sin_num++];

            if (sin_num >= sizeof(table)) sin_num = 0;
        }
                                   
        Ch.PutTxTSBlockFrom(chNum, GTSBuff, 0, tssize);
    }

    void ChLoopback(IUserBlockInterface &Ch0, IUserBlockInterface &Ch1, int ch)
    {
        word tssize = Ch0.GetTSBlockSize();
        ESS_ASSERT(tssize < CMaxChBuffSize);

        // read 
        Ch0.CopyRxTSBlockTo(ch, GTSBuff, 0, tssize);

        // write
        Ch0.PutTxTSBlockFrom(ch, GTSBuff, 0, tssize);
    }

    // ----------------------------------------------

    class DummyWriter : public BfTdm::IUserBlockInterface
    {

    // IUserBlockInterface impl
    public:

        void CopyRxTSBlockTo(byte timeSlot, byte *tsBuff, word offset , word tsSampleCount)
        {
        }        
        void CopyRxTSBlockTo(byte timeSlot, word *tsBuff,  word offset, word tsSampleCount)
        {        	
        }

        void PutTxTSBlockFrom(byte timeSlot, const byte *tsBuff, word offset , word tsSampleCount)
        {
        }
        void PutTxTSBlockFrom(byte timeSlot, const word *tsBuff, word offset , word tsSampleCount)
        {
        }

        void CopyTxTSBlockTo(byte timeSlot, byte *tsBuff, word buffSize)
        {
        }

        void UserFree()
        {
        }

        word GetTSBlockSize() const
        {
            return 160;
        }

        void CopySyncTxTo(byte timeSlot, byte *tsBuff,  word offset, word tsSampleCount)
        {
        }
        void CopySyncTxTo(byte timeSlot, word *tsBuff,  word offset, word tsSampleCount)
        {
        }

        const word *GetRxBlock(word frameNum) const { return 0; }
        word *GetTxBlock(word frameNum) const { return 0; }

    };

    DummyWriter GDummyWriter;

    /*
    template<class T>
    typename std::vector<T>::iterator VectorFind(const std::vector<T> &v, const T &val)
    {
        return std::find(v.begin(), v.end(), val);
    } */

    template<class TCont>
    typename TCont::iterator ContainerFind(TCont &v, const typename TCont::value_type &val)
    {
        return std::find(v.begin(), v.end(), val);
    }

    template<class TCont>
    bool ContainerRemove(TCont &v, const typename TCont::value_type &val, bool mustBeSuccess = true)
    {
        typename TCont::iterator i = ContainerFind(v, val);

        if (i == v.end())
        {
            if (mustBeSuccess) ESS_HALT("ContainerRemove -- value not found!");
            return false;
        }

        v.erase(i);
        return true;
    }

   
} // namespace 

// ---------------------------------------------------------------

namespace TdmMng
{
    // TDM SPORT single channel
    class TdmSportChannel : boost::noncopyable
    {
        ITdmOwner &m_owner;

        TdmEchoSuppress m_echo;
        bool m_tdmForceLinear;

    public:

        TdmSportChannel(ITdmOwner &owner, int maxChannels, 
            const BfTdm::TdmProfile &tdmProfile, bool useAlaw) : 
          m_owner(owner),
          m_echo(maxChannels, tdmProfile, useAlaw),
          m_tdmForceLinear(tdmProfile.GetCompandingLaw() != BfTdm::CNoCompanding)
        {            
            // ...
        }

        // ITdmOwner& Owner() { return m_owner; }

        bool OwnerEqual(ITdmOwner &owner)
        {
            return (&owner == &m_owner);
        }

        void ProcessRead(BfTdm::IUserBlockInterface &block, TdmProcessReadStatHelper *pHelper)
        {
            TdmReadWrapper wrapper(block, pHelper, m_echo, m_tdmForceLinear);
            m_owner.TdmFrameRead(wrapper); // вычитывание данных из TDM
        }

        void ProcessWrite(TdmWriteWrapper &wrapper)
        {
            m_owner.TdmFrameWrite(wrapper);
        }

        void ProcessWrite(BfTdm::IUserBlockInterface &block)
        {
            TdmWriteWrapper wrapper(block, m_tdmForceLinear ? &m_echo.getCodec() : 0);
            m_owner.TdmFrameWrite(wrapper);
        }

        void StartEchoSuppress(int chNumber, int taps)
        {
            m_echo.EnableForChannel(chNumber, taps);
        }

        void StopEchoSuppress(int chNumber)
        {
            m_echo.DisableForChannel(chNumber);
        }

        // IRQ context!!
        void IrqEchoDataCopy(BfTdm::IUserBlockInterface &ch)
        {
            m_echo.IrqEchoDataCopy(ch);
        }

    };

    // ---------------------------------------------------------------

    // single TDM SPORT (2x channels)
    class TdmHAL::TdmSport : boost::noncopyable
    {
        const BfTdm::TdmProfile m_tdmProfile;
        BfTdm::Tdm m_tdm;

        // channels, direct map vector index -> channel with same number
        std::vector< shared_ptr<TdmSportChannel> > m_channels;

    public:

        TdmSport(BfTdm::ITdmObserver &observer, int sportNumber, const BfTdm::TdmProfile &profile) :
          m_tdmProfile(profile),
          m_tdm(observer, sportNumber, profile)
        {            
            m_tdm.Enable();

            // reserve two channels
            shared_ptr<TdmSportChannel> pEmpty; 
            m_channels.push_back(pEmpty);
            m_channels.push_back(pEmpty);
        }

        BfTdm::Tdm& Tdm() { return m_tdm; }

        void AddChannel(int chNum, ITdmOwner &owner, bool useAlaw)
        {
            ESS_ASSERT(m_channels.at(chNum) == 0);

            const int CMaxChannels = 32;

            TdmSportChannel *p = 
                new TdmSportChannel(owner, CMaxChannels, m_tdmProfile, useAlaw);

            m_channels.at(chNum) = shared_ptr<TdmSportChannel>(p);
        }

        void RemoveChannel(int chNum)
        {
            ESS_ASSERT(m_channels.at(chNum) != 0);

            // clear
            m_channels.at(chNum) = shared_ptr<TdmSportChannel>();
        }

        int OwnerToChannel(ITdmOwner &owner)
        {
            for(int i = 0; i < m_channels.size(); ++i)
            {
                if (m_channels.at(i) == 0) continue;

                if (m_channels.at(i)->OwnerEqual(owner)) return i;
            }

            return -1;
        }

        void Stop()
        {
            m_tdm.Disable();
        }

        void ProcessRead(ITdmOwner &owner, IrqInfoMsg &info, TdmProcessReadStatHelper *pHelper)
        {
            int i = OwnerToChannel(owner);
            ESS_ASSERT(i >= 0);
            m_channels.at(i)->ProcessRead(info.BlockInterface(i), pHelper);
        }

        void ProcessWrite(ITdmOwner &owner, IrqInfoMsg &info)
        {
            int i = OwnerToChannel(owner);
            ESS_ASSERT(i >= 0);            
            m_channels.at(i)->ProcessWrite( info.BlockInterface(i) );
        }

        TdmSportChannel& Channel(int chNum)
        {
            ESS_ASSERT(m_channels.at(chNum) != 0);
            return *(m_channels.at(chNum));
        }

        // IRQ context!!
        void IrqEchoDataCopy(BfTdm::IUserBlockInterface &ch0, BfTdm::IUserBlockInterface &ch1)
        {
            if (m_channels.at(0) != 0) m_channels.at(0)->IrqEchoDataCopy(ch0);
            if (m_channels.at(1) != 0) m_channels.at(1)->IrqEchoDataCopy(ch1);
        }

    };

}  // namespace TdmMng

// ---------------------------------------------------------------

namespace TdmMng
{

    void TdmHAL::MsgOnNewBlock(shared_ptr<IrqInfoMsg> info)
    {       
        if (m_pBlockProcProfiler) m_pBlockProcProfiler->OnProcessingBegin();

        // echo shadow sync TX copy  <-- this code moved from IRQ context!
        if (CEnableEchoDataCopy)
        {
            int sportNum = info->SportNum();
            if (m_sports.at(sportNum) != 0) 
            {
                m_sports.at(sportNum)->IrqEchoDataCopy(info->BlockInterface(0), 
                                                       info->BlockInterface(1));
            }
        }

        // process
        info->EnableAutoClose();

        for(int i = 0; i < m_handlers.size(); ++i)
        {
            m_handlers.at(i)->OnTdmHalIrq(info->SportNum(), *info);
        }

        if (m_pBlockProcProfiler) m_pBlockProcProfiler->OnProcessingEnd();
    }

    // ----------------------------------------------------------------------

    void TdmHAL::MsgOnBlockCollision()
    {
        for(int i = 0; i < m_handlers.size(); ++i)
        {
            m_handlers.at(i)->OnTdmHalBlockCollision();
        }
    }

    // ----------------------------------------------------------------------


    // DCB context!
    void TdmHAL::CallFromDCB(DcbParam param)
    {
        if (CDisableMsg)
        {
            iVDK::CriticalRegion cr;  // vdk thread vs real DCB context - ?
            param.pCh0->UserFree();
            param.pCh1->UserFree();
            return;
        }

        // put message
        m_statDcbCalls++;

        IrqInfoMsg *p = new IrqInfoMsg(*this, param.sportNum, param.pCh0, param.pCh1);
        shared_ptr<IrqInfoMsg> info(p);

        // m_mng.ProcessIRQinDCB(info);
        PutMsg(this, &TdmHAL::MsgOnNewBlock, info);
    }

    // ----------------------------------------------------------------------

    // DCB context!
    void TdmHAL::OnBlockCollisionDCB(int count)
    {
        PutMsg(this, &TdmHAL::MsgOnBlockCollision);

        // m_mng.OnBlockCollision();

        // std::cout << "TDM block collision " << count << std::endl;
    }


    // ----------------------------------------------------------------------

    void TdmHAL::BlockInfoToDcb(word sportNum, BfTdm::IUserBlockInterface &Ch0, BfTdm::IUserBlockInterface &Ch1)
    {
        DcbParam param;

        param.sportNum = sportNum;
        param.pCh0 = &Ch0;
        param.pCh1 = &Ch1;

        TdmProfiler *p = (sportNum == 0) ? m_profilerSport0.get() : 
                                           m_profilerSport1.get();

        if (p != 0) p->ExtBegin();

        m_dcb->PutMsg(&TdmHAL::CallFromDCB, *this, param);

        if (p != 0) p->ExtEnd();
    }


    // ----------------------------------------------------------------------

    // IRQ context!
    bool TdmHAL::IrqNewBlockProcess(word sportNum, BfTdm::IUserBlockInterface &Ch0, BfTdm::IUserBlockInterface &Ch1)
    {
        if (CEnableGenCh >= 0)
        {
            SinusGen(Ch0, CEnableGenCh);

            // dummy put
            BlockInfoToDcb(sportNum, GDummyWriter, GDummyWriter);

            return false;        
        }       

        if (CChLoopback >= 0)
        {
            ChLoopback(Ch0, Ch1, CChLoopback);

            // dummy put
            BlockInfoToDcb(sportNum, GDummyWriter, GDummyWriter);

            return false;
        }


        if (CDisableDCB)
        {            
            return false;
        }        

        // route message to DCB
        BlockInfoToDcb(sportNum, Ch0, Ch1);

        return true;
    }

    // ----------------------------------------------------------------------

    // called from IRQ context!
    void TdmHAL::NewBlockProcess(word sportNum, 
        BfTdm::IUserBlockInterface &Ch0, BfTdm::IUserBlockInterface &Ch1,
        bool collision)
    {
        ESS_ASSERT(!m_devicesDisabled);

        if (collision)
        {
            m_statBlockCollisions++;

            if (CAssertOnCollision)
            {
                ESS_HALT("");  // silence, 'couse in IRQ
            }
            else
            {
                m_dcb->PutMsg(&TdmHAL::OnBlockCollisionDCB, *this, m_statBlockCollisions);
            }

            // блоки просто НЕ поступают на обработку
            // можно попробовать передавать пустые блоки заглушки (есть трудности)
            // давать блоки на обработку нельзя -- блок оказывается в двух точках конвеера 
            return;
        }

        if (IrqNewBlockProcess(sportNum, Ch0, Ch1)) return;

        // блоки не были переданы дальше
        Ch0.UserFree();
        Ch1.UserFree();                                
    }

    /*
    void TdmHAL::BlocksCollision( word sportNum )
    {
        ESS_ASSERT(!m_devicesDisabled);

        m_statBlockCollisions++;

        // debug
        // iRtp::RtpCoreSocket::CheckSocketState();

        if (CAssertOnCollision)
        {
            ESS_HALT("");  // silence, 'couse in IRQ
        }
        else
        {
            m_dcb->PutMsg(&TdmHAL::OnBlockCollisionDCB, *this, m_statBlockCollisions);
        }
        
    } */

    // ----------------------------------------------------------------------

    TdmHAL::TdmHAL(const TdmHalProfile &profile) : 
        iCore::MsgObject(profile.Thread),
        m_dcb( new BfDev::VdkThreadDcb(CDcbDeep, CDcbMsgSize) ), 
        m_sport0(profile.Sport0),
        m_sport1(profile.Sport1),
        m_log(profile.LogCreator->CreateSession("TdmHAL", true)),
        m_tagWarning(m_log->RegisterRecordKindStr("Warning")),
        m_pBlockProcProfiler(0)
    {
        m_devicesDisabled = false;

        m_statDcbCalls = 0;
        m_statBlockCollisions = 0;

        bool warning = (CDisableDCB) || (CDisableMsg) || (CEnableGenCh >= 0) || 
                       (CChLoopback >= 0);

        if (warning)
        {
			*m_log << m_tagWarning << "Block Processing in DEBUG mode!" << EndRecord;        	
        }

        // init idle buff
        {
            int count = std::max(m_sport0.GetDMABlockCapacity(), 
                                 m_sport1.GetDMABlockCapacity());

            m_idleGen.reserve(count);

            for(int i = 0; i < count; ++i)
            {
                m_idleGen.push_back(iDSP::CSilenceSample);
            }
        }

        // reserve two sports
        shared_ptr<TdmSport> pEmpty;
        m_sports.push_back(pEmpty);
        m_sports.push_back(pEmpty);
    }

    // ----------------------------------------------------------------------

    TdmHAL::~TdmHAL()
    {
        ESS_ASSERT(m_devicesDisabled);

        m_sports.clear();

        m_dcb.reset(0);

        int dummy = 1;
    }

    // ----------------------------------------------------------------------

    BfTdm::Tdm* TdmHAL::RegisterTdm(int sportNum, int chNum, ITdmOwner &owner, bool useAlaw)
    {
        ESS_ASSERT(!m_devicesDisabled);
        ESS_ASSERT(sportNum < 2);

        // SPORT not exists
        if (m_sports.at(sportNum) == 0)
        {
            BfTdm::TdmProfile &profile = (sportNum == 0) ? m_sport0 : m_sport1;
            shared_ptr<TdmSport> p( new TdmSport(*this, sportNum, profile) );
            m_sports.at(sportNum) = p;
        }

        m_sports.at(sportNum)->AddChannel(chNum, owner, useAlaw);

        return &m_sports.at(sportNum)->Tdm();
    }

    // ----------------------------------------------------------------------

    void TdmHAL::UnregTdm( ITdmOwner &owner )
    {   
        int sport, sportCh;
        FindOwner(owner, sport, sportCh);

        m_sports.at(sport)->RemoveChannel(sportCh);
    }

    // ----------------------------------------------------------------------

    void TdmHAL::FindOwner( ITdmOwner &owner, int &sport, int &sportCh )
    {
        for(int i = 0; i < m_sports.size(); ++i)
        {
            if (m_sports.at(i) == 0) continue;

            sportCh = m_sports.at(i)->OwnerToChannel(owner);
            if (sportCh < 0) continue;

            // fix sport and exit
            sport = i;
            return;
        }

        ESS_HALT("Owner not found!");        
    }

    // ----------------------------------------------------------------------

    void TdmHAL::StopDevices()
    {
        ESS_ASSERT(!m_devicesDisabled);

        for(int i = 0; i < m_sports.size(); ++i)
        {
            if (m_sports.at(i) == 0) continue;
            m_sports.at(i)->Stop();
        }

        m_devicesDisabled = true;
    }

    // ----------------------------------------------------------------------

    void TdmHAL::ProcessRead(ITdmOwner &owner, IrqInfoMsg &info, TdmProcessReadStatHelper *pHelper)
    {
        ESS_ASSERT(!m_devicesDisabled);        
        m_sports.at(info.SportNum())->ProcessRead(owner, info, pHelper);
    }

    // ----------------------------------------------------------------------

    void TdmHAL::ProcessWrite(ITdmOwner &owner, IrqInfoMsg &info)
    {
        ESS_ASSERT(!m_devicesDisabled);
        m_sports.at(info.SportNum())->ProcessWrite(owner, info);
    }

    // ----------------------------------------------------------------------

    void TdmHAL::IdleGenerator(ITdmWriteWrapper &buff, int chNum)
    {
        ESS_ASSERT( buff.BlockSize() == m_idleGen.size() );

        buff.Write(chNum, 0, &m_idleGen[0], m_idleGen.size());
    }

    // ----------------------------------------------------------------------

    void TdmHAL::StartEchoSuppress( ITdmOwner &owner, int chNumber, int taps )
    {
        int sport, sportCh;
        FindOwner(owner, sport, sportCh);

        m_sports.at(sport)->Channel(sportCh).StartEchoSuppress(chNumber, taps);
    }

    // ----------------------------------------------------------------------

    void TdmHAL::StopEchoSuppress( ITdmOwner &owner, int chNumber )
    {
        int sport, sportCh;
        FindOwner(owner, sport, sportCh);

        m_sports.at(sport)->Channel(sportCh).StopEchoSuppress(chNumber);
    }

    // ----------------------------------------------------------------------

    void TdmHAL::RegisterEventsHandler( Utils::SafeRef<ITdmHalEvents> handler )
    {
        ESS_ASSERT( ContainerFind(m_handlers, handler) == m_handlers.end() );
        m_handlers.push_back(handler);
    }

    // ----------------------------------------------------------------------

    void TdmHAL::UnregisterEventsHandler( Utils::SafeRef<ITdmHalEvents> handler )
    {
        ContainerRemove(m_handlers, handler, true);
    }

    // ----------------------------------------------------------------------

    void TdmHAL::EnableProfiling( int sportNum )
    {
        ESS_ASSERT(sportNum < m_sports.size());
        ESS_ASSERT(m_sports.at(sportNum) == 0);

        // create profiler
        scoped_ptr<TdmProfiler> &p = (sportNum == 0) ? m_profilerSport0 : m_profilerSport1;
        ESS_ASSERT(p.get() == 0);
        p.reset( new TdmProfiler() );

        // bind profiler to profile
        BfTdm::TdmProfile &prof = (sportNum == 0) ? m_sport0 : m_sport1;
        prof.Profiler( p.get() );
    }

    // ----------------------------------------------------------------------

    TdmProfiler::Info TdmHAL::GetProfilingInfo(int sportNum, bool base)
    {
        ESS_ASSERT(sportNum < m_sports.size());

        scoped_ptr<TdmProfiler> &p = (sportNum == 0) ? m_profilerSport0 : m_profilerSport1;
        ESS_ASSERT(p.get() != 0);

        return p->GetInfo(base);
    }

    // ----------------------------------------------------------------------

    void IrqInfoMsg::ProcessReadFor(ITdmOwner &owner, TdmProcessReadStatHelper *pHelper)
    {
        m_owner.ProcessRead(owner, *this, pHelper);
    }

    // ----------------------------------------------------------------------

    void IrqInfoMsg::ProcessWriteFor(ITdmOwner &owner)
    {
        m_owner.ProcessWrite(owner, *this);
    }

} // namespace TdmMng

