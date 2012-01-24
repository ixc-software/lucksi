#ifndef __E1APPLICATIONSTATS__
#define __E1APPLICATIONSTATS__

#include "Platform/Platform.h"

#include "Utils/StatisticElement.h"
#include "Utils/IntToString.h"

#include "BfDev/BfTimerCounter.h"

#include "EchoApp/EchoAppStats.h"
#include "SafeBiProto/ISerialazable.h"

// Warning! This file affected CmpProto version!

namespace E1App
{

    struct E1ApplicationStats : public SBProto::ISerialazable
    {

        // stats
        int CmpPackInSession, CmpPackInTotal;  // ������� �������� CMP ������� (������/�����)
        int StartTimeTicks, StartSessionTimeTicks;  // ����� ������ ������ / ... ������ mSec
        int TotalSessions;   // ����� ������ (�������� ���������� ������ BfInit)
        int CmpCommandsFatalExceptions;  // ���������� ������������ ���� �� ����� ��������� CMP �������
        Utils::StatElementForInt TimerEventTime;  // ����� ���������� ������� ������� (� ����� ����)
        Utils::StatElementForInt CmpCommandTime;  // ����� ���������� CMP �������� (� ����� ����)
        Utils::StatElementForInt TimerEventInterval;  // �������� ������ ������� ������� (ms) 

        // current
        Platform::dword CurrTicks;  // ������� ���� ������� 
        int CpuFreq;  // ������� ����������
        int SysBusFreq;  // ������� ��������� ���� (��� ��������� �����)
        int CpuUsage;  // cpu usage
        int HeapFreeBytes;  // ���� � ����
        std::string ActiveLinkInfo; // ���������� � ������� ������ (���� ����)
        int ActiveChCount;      // ���-�� �������� ������� 
        std::string BuildInfo;  // application build-info
        std::string MacHash;    // application protection info

        // echo 
        EchoApp::EchoAppStats Echo;

        E1ApplicationStats()
        {
            Clear();
        }

        void Clear();
        std::string ToString(const std::string &sep = "; ") const;

		template<class Data, class TStream>
			static void Serialize(Data &data, TStream &s)
        {
            s
                << data.CmpPackInSession
                << data.CmpPackInTotal
                << data.StartTimeTicks 
                << data.StartSessionTimeTicks 
                << data.TotalSessions
                << data.CmpCommandsFatalExceptions;
		
			Utils::StatElementForInt::Serialize(data.TimerEventTime, s);
			Utils::StatElementForInt::Serialize(data.CmpCommandTime, s);
            Utils::StatElementForInt::Serialize(data.TimerEventInterval, s);

            s
                << data.CpuFreq 
                << data.SysBusFreq
                << data.CpuUsage 
                << data.HeapFreeBytes
                << data.ActiveLinkInfo
                << data.ActiveChCount
                << data.BuildInfo
                << data.MacHash;

            EchoApp::EchoAppStats::Serialize(data.Echo, s);
        }

        static void Test();
		
    };    
    
    
}  // namespace E1App

#endif
