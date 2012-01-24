#include "stdafx.h"

#include "Ds2155/boardds2155.h"
#include "Utils/Random.h"
#include "Utils/TimerTicks.h"
#include "Utils/BidirBuffer.h"
#include "Utils/ManagedList.h"
#include "Utils/IntToString.h"

#include "HdlcTest.h"


// -------------------------------------------------------

using namespace Ds2155;
using boost::scoped_ptr;
using Utils::BidirBuffer;
using Utils::ManagedList;
using Platform::byte;

namespace
{
    enum
    {
        // HDLC
        CHdlcMemPoolBlockSize      = 256,
        CHdlcBlocksNum             = 32,
        CHdlcBuffOffset            = 0,
        CHdlcMaxPackSize           = 256,        
    };

    BOOST_STATIC_ASSERT(CHdlcMaxPackSize <= CHdlcMemPoolBlockSize);

    // --------------------------------------------------

    class Board
    {
        BoardDs2155 m_board;

    public:

        Board(ILogHdlc *log) :
            m_board("HdlcTest", Ds2155::CBaseAddress, 
                    LiuProfile(), 
                    HdlcProfile(CHdlcMemPoolBlockSize,
		                    	CHdlcBlocksNum,
		                    	CHdlcBuffOffset,
		                    	CHdlcMaxPackSize),
		            log)

        {
        	m_board.EnableLoopBack();
        }
        
        BoardDs2155& get() { return m_board; }

        void Process()
        {
            m_board.HdlcPolling();
        }

    };

    // --------------------------------------------------

    class SendTest
    {
        enum
        {
            CTestTimeToRun = 60 * 1000,

            CBurstSendPackCount = 4,  
            CMaxPacketSize = 60,     
                                      
            CRecvTimeout   = 1000,
                                      
            CSleepInterval = 10,

            CPrintBadFlags = false,
            CPrintTestProgressTime = 0,
            CPrintPacks = false,
        };

        BOOST_STATIC_ASSERT(CBurstSendPackCount < CHdlcBlocksNum);
        BOOST_STATIC_ASSERT(CMaxPacketSize <= CHdlcMaxPackSize);
        BOOST_STATIC_ASSERT(CMaxPacketSize > 3);


        static void Print(const std::string &msg, bool lf = true)
        {
            std::cout << msg;
            if (lf) std::cout << std::endl;
        }

        class Packet
        {
            std::vector<byte> m_buff;

            static int MakeSeed(int ch, int packNumber)
            {
                return packNumber + (ch * 2 + 1024);
            }

            static int MakeSize(Utils::Random &rnd)
            {
                return rnd.Next(CMaxPacketSize - 3) + 3;
            }

        public:

            Packet(int ch, int packNumber)
            {
                Utils::Random rnd( MakeSeed(ch, packNumber) );

                int size = MakeSize(rnd);
                m_buff.reserve(size);

                for(int i = 0; i < size; ++i) m_buff.push_back( rnd.NextByte() );
            }

            void Send(Hdlc &hdlc)
            {
                if (CPrintPacks)
                {
                    std::string s;
                    Utils::DumpToHexString(&m_buff[0], m_buff.size(), s);

                    Print(">> ", false);
                    Print(s);
                }

                hdlc.SendPack(&m_buff[0], m_buff.size());
            }

            static bool Verify(int ch, int packNumber, BidirBuffer &buff)
            {
                Utils::Random rnd( MakeSeed(ch, packNumber) );
                int size = MakeSize(rnd);               

                if (size != buff.Size()) return false;

                for(int i = 0; i < size; ++i)
                {
                    int val = rnd.NextByte();

                    if (val != buff[i])
                    {
                        return false;
                    }
                }

                return true;
            }
        };

        class HdlcTest
        {
            int m_ch;
            scoped_ptr<Hdlc> m_hdlc;
            Utils::Random m_rnd;

            // state
            int m_packsToSend;
            int m_packsToRecv;
            Utils::TimerTicks m_recvTime;

            // stats
            int m_totalSendPacks;
            int m_totalRecvPacks;
            int m_badRecv;
            int m_timeouts;
            int m_badFlags;

            void EnterSendMode()
            {
                m_packsToSend = m_rnd.Next(CBurstSendPackCount) + 1;
                m_packsToRecv = m_packsToSend;
            }

            void EnterRecvMode()
            {
                m_recvTime.Reset();
            }

            void SendPack()
            {
                Packet pack(m_ch, m_totalSendPacks++);
                pack.Send( *(m_hdlc.get()) );
            }

            bool RecvPack()
            {
                scoped_ptr<BidirBuffer> buff( m_hdlc->GetPack() );
                if (buff.get() == 0) return false;

                if (CPrintPacks)
                {
                    std::string s;
                    Utils::DumpToHexString(buff->Front(), buff->Size(), s);

                    Print("<< ", false);
                    Print(s);
                }

                if (!Packet::Verify(m_ch, m_totalRecvPacks++, *(buff.get()) )) m_badRecv++;

                return true;
            }

            void CheckFlags()
            {
                ErrorFlags flags = m_hdlc->PeekErrors();

                if (!flags.HasErrors()) return;

                m_badFlags++;

                // print
                if (!CPrintBadFlags) return;

                std::ostringstream ss;
                ss << "HDLC " << m_ch << " bad flags " << flags.ToString();
                Print( ss.str() );
            }

			static const int CLogProfile = 0
//											CTxPollingEnabled|
//											CTxUserLevelEnabled|
//											CTxFSMEnabled|
//											CRxPollingEnabled|
//											CRxUserLevelEnabled
//											CRxFSMEnabled                
											;
            
            
        public:

            HdlcTest(Board &board, int ch) : 
                m_ch(ch),
//                m_hdlc(board.get().AllocHdlc(ch)),                 
                m_hdlc(board.get().AllocHdlc(ch, HdlcLogProfile(true, CLogProfile))),
                m_rnd(ch * 2 + 1)
            {
                // clear stats
                m_totalSendPacks = 0;
                m_totalRecvPacks = 0;
                m_badRecv = 0;
                m_timeouts = 0;
                m_badFlags = 0;

                EnterSendMode();
            }

            bool Process()  // false if test aborted
            {
                // flags poll
                CheckFlags();

                // send mode
                if (m_packsToSend > 0)
                {
                    SendPack();

                    m_packsToSend--;

                    if (m_packsToSend == 0) EnterRecvMode();

                    return true;
                }

                // recv mode
                while(true)
                {
                    if (m_recvTime.Get() > CRecvTimeout)
                    {
                        m_timeouts++;
//                        return false;
                        EnterSendMode();
                        break;
                    }

                    if (RecvPack()) m_packsToRecv--;
                    else break;
                    
//                    if (m_badRecv) return false;                    

                    if (m_packsToRecv == 0)
                    {
                        EnterSendMode();
                        break;
                    }
                }

                return true;
                
            }

            void GetState(std::string &state)
            {
                std::ostringstream ss;

                ss << "HDLC " << m_ch << "; Send " << m_totalSendPacks 
                    << "; Recv " << m_totalRecvPacks << "; Bad recv " << m_badRecv
                    << "; Timeouts " << m_timeouts << "; Bad flags " << m_badFlags;

                state = ss.str();
            }
            
        };

        class HdlcList
        {
            ManagedList<HdlcTest> m_list;

        public:

            void Add(HdlcTest *pHdlc)
            {
                m_list.Add(pHdlc);
            }

            bool Process()
            {
                for(int i = 0; i < m_list.Size(); ++i)
                {
                    if (!m_list[i]->Process()) return false;
                }

                return true;
            }

            void PrintInfo()
            {
                for(int i = 0; i < m_list.Size(); ++i)
                {
                    std::string s;
                    m_list[i]->GetState(s);
                    Print(s);
                }
            }
        };

    public:

        static void Test(ILogHdlc *log)
        {
            Board board(log);

            HdlcList list;
            list.Add( new HdlcTest(board, 16) );
//            list.Add( new HdlcTest(board, 15) );

            Utils::TimerTicks tComplete;
            Utils::TimerTicks tPrintProgress;
            
            Print("Started!");
            
            while(true)
            {
                if ( (CTestTimeToRun > 0) && 
                     (tComplete.Get() >= CTestTimeToRun) )
                {
                    Print("Test completed!");
                    list.PrintInfo();
                    break;
                }

                board.Process();

                if (!list.Process())
                {
                    Print("Test aborted!");
                    list.PrintInfo();
                    break;
                }

                Platform::ThreadSleep(CSleepInterval);

                // print test state
                if ((CPrintTestProgressTime > 0) && 
                    (tPrintProgress.Get() >= CPrintTestProgressTime))
                {
                    list.PrintInfo();
                    tPrintProgress.Reset();
                }
            }
            
			for (int i = 0; i< 5; ++i)
			{			
	            board.Process();            
	            Platform::ThreadSleep(CSleepInterval);            
			}
            

        }
    };


} // namespace


// -------------------------------------------------------

namespace Ds2155
{
	
	void RunHdlcTests(ILogHdlc *log)
    {
        SendTest::Test(log);
    }
		
}  // namespace Ds2155
