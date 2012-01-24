#ifndef __TDMSTREAMSTAT__
#define __TDMSTREAMSTAT__

#include "Utils/StatisticElement.h"
#include "BfDev/BfTimerCounter.h"
#include "SafeBiProto/ISerialazable.h"

namespace TdmMng
{

    class TdmProcessReadStatHelper
    {
        BfDev::BfTimerCounter &m_timer;
        int m_beginTicks;
        int m_readSendStageSum;

        int m_beginEchoTicks;
        int m_echoStageSum;
        int m_echoBlocks;

    public:

        TdmProcessReadStatHelper(BfDev::BfTimerCounter &timer)
            : m_timer(timer), m_beginTicks(0), m_beginEchoTicks(0)
        {
            m_readSendStageSum = 0;

            m_echoStageSum = 0;
            m_echoBlocks = 0;
        }

        // send stage
        void BeginReadSend()
        {
            m_beginTicks = m_timer.GetCounter();
        }

        void EndReadSend()
        {
            m_readSendStageSum += (m_timer.GetCounter() - m_beginTicks);
        }

        int ReadSendStageSum() const { return m_readSendStageSum; }

        // echo stage
        void BeginEcho()
        {
            m_beginEchoTicks = m_timer.GetCounter();
        }

        void EndEcho()
        {
            m_echoStageSum += (m_timer.GetCounter() - m_beginEchoTicks);
            m_echoBlocks++;
        }

        int EchoStageSum() const { return m_echoStageSum; }
        int EchoBlocks() const   { return m_echoBlocks; }

    };

    // -----------------------------------------------


    struct TdmStreamStat : public SBProto::ISerialazable
    {
        // ��� ��������� � ����� ������� BfTimerCounter - !
        int TdmBlockCollision;
        Utils::StatElementForInt FullIRQProcessingTime;   // ������ ������� ���������� ��������� IRQ TDM
        Utils::StatElementForInt ReadProcessingTime;      // ����� ����� ������
        Utils::StatElementForInt ReadProcessingTimeSend;  //  -//-  ������ �������� ������
        Utils::StatElementForInt ReadProcessingTimeEcho;  //  -//-  ������ ���������� ���
        Platform::int64          TotalEchoBlocksCount;    // ���������� ������������ ������ TDM � ����������� ���
        Utils::StatElementForInt WriteProcessingTime;     // ����� ����� ������ 
        Utils::StatElementForInt SocketPollingTime;       // ����� ������ �������
        Utils::StatElementForInt SocketDataIn;            // ����� ��������� RTP ��������

        TdmStreamStat();

        void Clear();
		std::string ToString(int freq) const;

		template<class Data, class TStream>
	    static void Serialize(Data &data, TStream &s)
        {
            s << data.TdmBlockCollision;
			Utils::StatElementForInt::Serialize(data.FullIRQProcessingTime, s);
			Utils::StatElementForInt::Serialize(data.ReadProcessingTime, s);
			Utils::StatElementForInt::Serialize(data.ReadProcessingTimeSend, s);
			Utils::StatElementForInt::Serialize(data.ReadProcessingTimeEcho, s);
            s << data.TotalEchoBlocksCount;
			Utils::StatElementForInt::Serialize(data.WriteProcessingTime, s);
			Utils::StatElementForInt::Serialize(data.SocketPollingTime, s);
			Utils::StatElementForInt::Serialize(data.SocketDataIn, s);
        }

    };
    
} // namespace TdmMng

#endif
