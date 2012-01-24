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
        int CmpPackInSession, CmpPackInTotal;  // счетчик входящих CMP пакетов (сессия/всего)
        int StartTimeTicks, StartSessionTimeTicks;  // время начала работы / ... сессии mSec
        int TotalSessions;   // всего сессий (получено корректный команд BfInit)
        int CmpCommandsFatalExceptions;  // исключений неизвестного типа во время выполения CMP команды
        Utils::StatElementForInt TimerEventTime;  // время выполнения события таймера (в тиках шины)
        Utils::StatElementForInt CmpCommandTime;  // время выполнения CMP комманды (в тиках шины)
        Utils::StatElementForInt TimerEventInterval;  // интервал вызова событий таймера (ms) 

        // current
        Platform::dword CurrTicks;  // текущие тики системы 
        int CpuFreq;  // частота процессора
        int SysBusFreq;  // частота системной шины (для пересчета тиков)
        int CpuUsage;  // cpu usage
        int HeapFreeBytes;  // байт в куче
        std::string ActiveLinkInfo; // информация о текущей сессии (если есть)
        int ActiveChCount;      // кол-во активных каналов 
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
