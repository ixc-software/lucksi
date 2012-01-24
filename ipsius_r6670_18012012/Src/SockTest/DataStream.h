#ifndef __DATASTREAM__
#define __DATASTREAM__

#include "Platform/Platform.h"
#include "Utils/Random.h"

namespace SockTest
{
    using Platform::byte;

    enum DataStreamState
    {
        DssIdle,

        DssInProgress,

        DssErrRecvWrong,
        DssErrRecvExtra,
        DssErrTimeout,

        DssCompleted,
    };

    // In-out socket data streaming, cross platform
    class DataStream : boost::noncopyable
    {
        DataStreamState m_state;

        Utils::Random m_recv;
        int m_recvCount;

        Utils::Random m_send;
        Utils::Random m_sendGen;  // отвечает за период и порции данных
        int m_sendCount;
        int m_timeToSend;

        int m_startTime;
        int m_testTimeout;

        void CompleteCheck()
        {
            if (m_state != DssInProgress) return;

            if ((m_recvCount <= 0) && (m_sendCount <= 0))
            {
                m_state = DssCompleted;
            }
        }

        static int NextTimeToSend(Utils::Random &rnd)
        {
            return Platform::GetSystemTickCount() + (5 + rnd.Next(20));
        }

        static int BytesToSend(Utils::Random &rnd, int max)
        {
            int val = 10 + rnd.Next(200);
            if (val > max) val = max;

            return val;
        }

        void TestTimeoutCheck()
        {
            if (m_state != DssInProgress) return;

            if (Platform::GetSystemTickCount() - m_startTime > m_testTimeout)
            {
                m_state = DssErrTimeout;
            }
        }

        void BeginTest()
        {
            m_state = DssInProgress;
            m_startTime = Platform::GetSystemTickCount();
        }

    public:

        DataStream(int recvSeed, int recvCount, int sendSeed, int sendCount, 
            int testTimeout, bool startTest)
        {
            m_state = DssIdle;

            m_recv.setSeed(recvSeed);
            m_recvCount = recvCount;

            m_send.setSeed(sendSeed);
            m_sendCount = sendCount;
            m_sendGen.setSeed(sendSeed + sendCount);
            m_timeToSend = NextTimeToSend(m_sendGen);

            m_startTime = 0;
            m_testTimeout = testTimeout;

            if (startTest) BeginTest();
        }

        void Put(const std::vector<byte> &data)
        {
            if (m_state == DssIdle) BeginTest();
            TestTimeoutCheck();

            if (m_state != DssInProgress) return;

            for(int i = 0; i < data.size(); ++i)
            {
                if (data.at(i) == m_recv.NextByte())
                {
                    if (m_recvCount > 0) 
                    {
                        m_recvCount--;
                        continue;
                    }

                    m_state = DssErrRecvExtra;
                    return;
                }

                m_state = DssErrRecvWrong;
                return;
            }

            CompleteCheck();
        }

        bool Peek(std::vector<byte> &data)
        {
            TestTimeoutCheck();

            if (m_state != DssInProgress) return false;
            if (m_sendCount <= 0) return false;

            // TODO -- fix it, work after counter overflow
            if (Platform::GetSystemTickCount() < m_timeToSend) return false;

            // fill
            data.clear();
            int count = BytesToSend(m_sendGen, m_sendCount);

            for(int i = 0; i < count; ++i)
            {
                data.push_back( m_send.NextByte() );
            }

            m_sendCount -= count;

            // update
            m_timeToSend = NextTimeToSend(m_sendGen);
            CompleteCheck();

            return true;
        }

        DataStreamState State() const { return m_state; }

        bool RunCompleted() const 
        { 
            return (m_state != DssInProgress) && (m_state != DssIdle); 
        }

        std::string StateAsString() const
        {
           if (m_state == DssErrRecvWrong) return "Wrong recv";
           if (m_state == DssErrRecvExtra) return "Extra recv";
           if (m_state == DssCompleted)    return "Completed";
           if (m_state == DssErrTimeout)   return "Timeout";

           ESS_HALT("StateAsString");
           return "";
        }
    };


}  // namespace SockTest

#endif

