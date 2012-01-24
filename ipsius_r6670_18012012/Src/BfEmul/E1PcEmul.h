#ifndef __E1PCEMUL__
#define __E1PCEMUL__

#include "Platform/Platform.h"
#include "Utils/BidirBuffer.h"
#include "Utils/BidirBufferCreator.h"
#include "Utils/ManagedList.h"
#include "Utils/VirtualInvoke.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"

// ------------------------------------------------------

namespace BfTdm
{

    class TdmProfile;
    class ITdmObserver;

}  // namespace BfTdm

// ------------------------------------------------------

namespace  BfEmul
{
    using Platform::dword;

    struct BoardPair
    {
        std::string B1;
        std::string B2;

        BoardPair()
        {
            B1 = "";
            B2 = "";
        }
		bool operator == (const BoardPair &pair) const 
		{
			return B1 == pair.B1 && B2 == pair.B2;
		}
		BoardPair(const std::string &b1, const std::string &b2) :
			B1(b1), B2(b2)
        {}

        bool Empty() const
        {
            return B1.size() == 0 || B2.size() == 0;
        }

    };

    // ---------------------------------------------------

    struct HdlcPack
    {
        std::string Destination;
        Utils::BidirBuffer *PData;

        HdlcPack() : Destination(""), PData(0)
        {
        }

        HdlcPack(const std::string &dst, Utils::BidirBuffer *pData) :
            Destination(dst),
            PData(pData)
        {
            ESS_ASSERT(dst.size() > 0);
            ESS_ASSERT(pData != 0);
        }

        void FreeData()
        {
            delete PData;
        }
    };

    // ---------------------------------------------------

    class TdmEmulPoint;

    // ---------------------------------------------------

    class IPairResolve : public Utils::IBasicInterface
    {
    public:
        virtual std::string FindDestination(const std::string &src) = 0;
    };

    // ---------------------------------------------------
    class TdmEmul : public iCore::MsgObject
    {
        IPairResolve                    &m_resolve;
		boost::scoped_ptr<Utils::IVirtualInvoke>      m_tdmBlockClosed;

        iCore::MsgTimer                 m_irqTimer;
        dword                           m_lastIrqTime;
        Utils::ManagedList<TdmEmulPoint>       m_tdm;
        Platform::Mutex                 m_mutex;
        bool                            m_blockProcessActive;

        void OnIrqTimer(iCore::MsgTimer *pTimer);

        void TryStartBlockProcess();
        void TryCloseBlockProcess();

        void TdmBlockClosed();
        void OnTdmBlockClosed();

        TdmEmulPoint* FindPointByName(const std::string &name);

    public:

        TdmEmul(iCore::MsgThread &thread, IPairResolve &resolve);
        ~TdmEmul();

        TdmEmulPoint* RegisterTdmPoint(const BfTdm::TdmProfile &profile, BfTdm::ITdmObserver &observer);
        void UnregisterTdmPoint(TdmEmulPoint *p);
    };

    // ---------------------------------------------------
    
    class PcEmul : public IPairResolve
    {
        typedef std::list<HdlcPack> HdlcPacksList;
		typedef std::vector<BoardPair> BoardPairList;
        BoardPairList          m_pairs;

        iCore::MsgThread                m_thread;
        bool                            m_dummiThreadRun;  // just for thread run in initilization list

        Utils::BidirBufferCreator<>     m_bidirPool;
        Platform::Mutex                 m_hdlcMutex;
        HdlcPacksList                   m_hdlcPacksToSend;

        TdmEmul                         m_tdm;
        
        PcEmul();

        static bool MsgThreadRun(iCore::MsgThread &thread)
        {
            thread.Run();
            return true;
        }

        void HdlcListClean(const std::string &dst);

    // IPairResolve impl
    private:

        std::string FindDestination(const std::string &src);

    public:

        // socks thread
        iCore::MsgThread& SocksThread();

        // HDLC
        void SendHdlcPack(const std::string &src, void *p, int size);
        Utils::BidirBuffer* RecvHdlcPack(const std::string &dst);

        // TDM
        TdmEmul& TDM() { return m_tdm; }
        
        // add pair
        void AddPair(const BoardPair &pair);
        // delete pair        
        void DeletePair(const BoardPair &pair);

        // static stuff
        static void Init();
        static PcEmul& Instance();
    };
    
    
}  // namespace E1App

#endif
