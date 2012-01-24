#ifndef __TDMHAL__
#define __TDMHAL__

#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ManagedList.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/SafeRef.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iVDK/CriticalRegion.h"

#include "BfTdm/TdmProfile.h"
#include "iLog/LogWrapper.h"

#include "BfTdm/tdm.h"

// #include "BfDev/DcbMsgQueue.h"
// #include "ITdmManager.h"

#include "TdmBlockWrappers.h"
#include "TdmChannelsSet.h"
#include "TdmProfiler.h"

namespace BfDev
{
    class VdkThreadDcb;    
}

namespace BfTdm
{
    class ITdmProfiler;
}

namespace TdmMng
{
    using Platform::word;
    using Platform::byte;
    using boost::shared_ptr;   
    using iLogW::LogRecordTag;
    using iLogW::EndRecord;
    using boost::scoped_ptr;

    class TdmEchoSuppress;
    
    // TDM owner, it's associated with (sportNum, chNum)
    class ITdmOwner : public Utils::IBasicInterface
    {
    public:
        // must be sync, 'couse block free after call!
        virtual void TdmFrameRead(TdmReadWrapper &block) = 0;
        virtual void TdmFrameWrite(TdmWriteWrapper &block) = 0;
    };

    // ------------------------------------------------------

    class ITdmBlock : public Utils::IBasicInterface
    {
    public:
        virtual void ProcessReadFor(ITdmOwner &owner, TdmProcessReadStatHelper *pHelper) = 0;
        virtual void ProcessWriteFor(ITdmOwner &owner) = 0;
    };

    // ------------------------------------------------------

    class ITdmHalEvents : public Utils::IBasicInterface
    {
    public:
        virtual void OnTdmHalIrq(int sportNum, ITdmBlock &block) = 0;
        virtual void OnTdmHalBlockCollision() = 0;
    };

    // ------------------------------------------------------

    class TdmHAL;

    /* 
        IRQ info for send as message

          RAII по умолчанию не используется, т.к. ~IrqInfoMsg() может вызываться 
          как подвисшее сообщения на уровне ~MsgObject(), в момент, когда объекты 
          BfTdm::IUserBlockInterface уже уничтожены.
          RAII режим включается после вызова EnableAutoClose(), в момент,
          когда сообщение уже доставлено объекту 
    */
    class IrqInfoMsg : public ITdmBlock
    {
        TdmHAL &m_owner;
        bool m_closeInDestructor;

        word m_sportNum;
        BfTdm::IUserBlockInterface *m_pCh0; 
        BfTdm::IUserBlockInterface *m_pCh1;

    // ITdmBlock impl
    private:

        void ProcessReadFor(ITdmOwner &owner, TdmProcessReadStatHelper *pHelper);
        void ProcessWriteFor(ITdmOwner &owner);

    public:

        IrqInfoMsg(TdmHAL &owner, word sportNum, 
            BfTdm::IUserBlockInterface *pCh0,
            BfTdm::IUserBlockInterface *pCh1) : 
            m_owner(owner), 
            m_closeInDestructor(false),
            m_sportNum(sportNum), 
            m_pCh0(pCh0), 
            m_pCh1(pCh1)
        {
            ESS_ASSERT(m_pCh0 != 0);
            ESS_ASSERT(m_pCh1 != 0);
        }

        ~IrqInfoMsg()
        {
            if (m_closeInDestructor)
            {
                iVDK::CriticalRegion cr;
                m_pCh0->UserFree();
                m_pCh1->UserFree();
            }
        }

        int SportNum() const 
        {
            return m_sportNum;
        }

        BfTdm::IUserBlockInterface& BlockInterface(int chNum)
        {
            if (chNum == 0) return *m_pCh0;
            if (chNum == 1) return *m_pCh1;

            ESS_HALT("");
            return *m_pCh0;  // dummi just for return something
        }

        void EnableAutoClose()
        {
            m_closeInDestructor = true;
        }

    };

    // ------------------------------------------------------

    struct TdmHalProfile
    {
        iCore::MsgThread &Thread;
        BfTdm::TdmProfile Sport0;
        BfTdm::TdmProfile Sport1;
        Utils::SafeRef<iLogW::ILogSessionCreator> LogCreator;

        TdmHalProfile(iCore::MsgThread &thread, 
            const BfTdm::TdmProfile &sport0,
            const BfTdm::TdmProfile &sport1,
            Utils::SafeRef<iLogW::ILogSessionCreator> logCreator) :
            Thread(thread),
            Sport0(sport0),
            Sport1(sport1),
            LogCreator(logCreator)
        {
            // ... 
        }

    };

    // ------------------------------------------------------

    // HAL for TDM streams
    class TdmHAL : 
        public iCore::MsgObject,
        public BfTdm::ITdmObserver,
        boost::noncopyable
    {
        // DCB call params
        struct DcbParam
        {
            word sportNum;
            BfTdm::IUserBlockInterface *pCh0; 
            BfTdm::IUserBlockInterface *pCh1;
        };

        // single TDM SPORT (2x channels)
        class TdmSport;

        scoped_ptr<BfDev::VdkThreadDcb> m_dcb;
        BfTdm::TdmProfile m_sport0, m_sport1;
        scoped_ptr<TdmProfiler> m_profilerSport0, m_profilerSport1;
        bool m_devicesDisabled;

        // SPORTs, direct map vector index -> sports with same number
        std::vector< shared_ptr<TdmSport> > m_sports;

        // events handlers 
        std::vector< Utils::SafeRef<ITdmHalEvents> > m_handlers;

        boost::scoped_ptr<iLogW::LogSession> m_log;
        LogRecordTag m_tagWarning;

        int m_statDcbCalls;
        int m_statBlockCollisions;

        std::vector<byte> m_idleGen;

        BfTdm::ITdmProfiler* m_pBlockProcProfiler;

        void CallFromDCB(DcbParam param);
        void OnBlockCollisionDCB(int count);

        void BlockInfoToDcb(word sportNum, 
            BfTdm::IUserBlockInterface &Ch0, 
            BfTdm::IUserBlockInterface &Ch1);

        bool IrqNewBlockProcess(word sportNum, 
            BfTdm::IUserBlockInterface &Ch0, 
            BfTdm::IUserBlockInterface &Ch1);

        // find owner's sport and sport channel
        // halt if unsuccess
        void FindOwner(ITdmOwner &owner, int &sport, int &sportCh);

        void MsgOnNewBlock(shared_ptr<IrqInfoMsg> info);
        void MsgOnBlockCollision();

        friend class IrqInfoMsg;
        void ProcessRead(ITdmOwner &owner, IrqInfoMsg &info, TdmProcessReadStatHelper *pHelper);
        void ProcessWrite(ITdmOwner &owner, IrqInfoMsg &info);

    // BfTdm::ITdmObserver impl
    private:

        void NewBlockProcess(word sportNum, 
            BfTdm::IUserBlockInterface &Ch0, 
            BfTdm::IUserBlockInterface &Ch1,
            bool collision);

    public:

        TdmHAL(const TdmHalProfile &profile);
        ~TdmHAL();

        void EnableProfiling(int sportNum);
        TdmProfiler::Info GetProfilingInfo(int sportNum, bool base);

        void SetBlockProcProfiler(BfTdm::ITdmProfiler* p) {m_pBlockProcProfiler = p;}

        void RegisterEventsHandler(Utils::SafeRef<ITdmHalEvents> handler);
        void UnregisterEventsHandler(Utils::SafeRef<ITdmHalEvents> handler);

        BfTdm::Tdm* RegisterTdm(int sportNum, int chNum, ITdmOwner &owner, bool useAlaw);
        void UnregTdm(ITdmOwner &owner);

        void IdleGenerator(ITdmWriteWrapper &buff, int chNum);

        void StartEchoSuppress(ITdmOwner &owner, int chNumber, int taps);
        void StopEchoSuppress(ITdmOwner &owner, int chNumber);

        // called once, before ~TdmHAL()
        void StopDevices();
    };

}  // namespace TdmMng

#endif
