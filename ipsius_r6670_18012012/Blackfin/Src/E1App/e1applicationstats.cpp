#include "stdafx.h"
#include "Utils/UtilsDateTime.h"
#include "E1ApplicationStats.h"

namespace 
{

	std::string MSecToUptime(int msec)
	{
		int sec = msec / 1000;
		msec %= 1000;
		int min = sec / 60;
		sec %= 60;
		int hour = min / 60;
		min %= 60;
		int day = hour / 24;
		hour %= 24;
		
		std::ostringstream out;
		if(day  != 0) out << day  << " d  ";
		out << Utils::Time(hour, min, sec, msec). ToString();
		return out.str();
	}

    std::string TicksToMcs(int ticks, int freq)
    {
        int val =  BfDev::BfTimerCounter::TimerCounterToMcs(ticks, freq);
        return Utils::IntToString(val);
    }

    std::string TicksStatToStr(const Utils::StatElementForInt &e, int freq)
    {
        if (e.Empty() || freq < 1) return "N/A";

        std::ostringstream out;

        out << "{ " << "avg " << TicksToMcs(e.Average(), freq) << ", "
                              << "max " << TicksToMcs(e.Max(), freq) 
                              << ", count " << e.Count() << "}";

        return out.str();
    }

    std::string StatToStr(const Utils::StatElementForInt &e)
    {
        if (e.Empty()) return "N/A";

        std::ostringstream out;

        out << "{ " << "avg "     << e.Average() << ", "
                    << "max "     << e.Max()
                    << ", count " << e.Count() << "}";

        return out.str();
    }

};

namespace E1App
{

	void E1ApplicationStats::Clear()
    {
        CmpPackInSession = 0;
        CmpPackInTotal = 0;
        StartTimeTicks = 0;
        StartSessionTimeTicks = 0;
        TotalSessions = 0;
        CmpCommandsFatalExceptions = 0;
        TimerEventTime.Clear();
        CmpCommandTime.Clear();
        TimerEventInterval.Clear();
        CpuFreq = 0;
        SysBusFreq = 0;
        CpuUsage = 0;
        HeapFreeBytes = 0;
        ActiveLinkInfo.clear();
        ActiveChCount = 0;
        Echo.Clear();
    }

	// ---------------------------------------------------------------------------------

	std::string E1ApplicationStats::ToString(const std::string &sep) const
    {
		std::ostringstream out;

		out << "StartTime: " << MSecToUptime(StartTimeTicks) << sep
			<< "StartTimeTicks = " << StartTimeTicks << sep
			<< "StartSessionTime: " << MSecToUptime(StartSessionTimeTicks) << sep
			<< "StartSessionTimeTicks = " << StartSessionTimeTicks << sep
			<< "CmpPackInSession = " << CmpPackInSession << sep
			<< "CmpPackInTotal = " << CmpPackInTotal << sep
			<< "TotalSessions = " << TotalSessions << sep
			<< "CmpCommandsFatalExceptions = " << CmpCommandsFatalExceptions << sep
			<< "TimerEventTimeMcs = " << TicksStatToStr(TimerEventTime, SysBusFreq) << sep
			<< "CmpCommandTimeMcs = " << TicksStatToStr(CmpCommandTime, SysBusFreq) << sep
            << "TimerEventIntervalMs = " << StatToStr(TimerEventInterval) << sep
			<< "CurrTicks = " << CurrTicks << sep
			<< "CpuFreq = " << CpuFreq << sep
            << "SysBusFreq = " << SysBusFreq << sep
			<< "CpuUsage = " << CpuUsage << sep
			<< "HeapFreeBytes = " << HeapFreeBytes << sep
			<< "ActiveLinkInfo = " << ActiveLinkInfo << sep
			<< "ActiveChCount = " << ActiveChCount << sep
            << "Build = " << BuildInfo << sep
            << "MAC-Hash = " << MacHash << sep
            << "Echo = (" << Echo.ToString() << ")";

        return out.str();
    }

	// ---------------------------------------------------------------------------------

    void E1ApplicationStats::Test()
    {
        using namespace std;

        BfDev::SysProperties::InitFromProjectOptions();

        E1ApplicationStats stats;

        stats.CpuFreq = BfDev::SysProperties::Instance().getFrequencyCpu();
        stats.SysBusFreq = BfDev::SysProperties::Instance().getFrequencySys();

        BfDev::BfTimerCounter t(0);

        for(int i = 0; i < 4; ++i)
        {
            t.Reset();

            Platform::ThreadWaitUS((1 + i) * 10);

            stats.CmpCommandTime.Add( t.GetCounter() );
            stats.TimerEventTime.Add( t.GetCounter() * 2 );
        }

        cout << stats.ToString("\n") << endl;
    }

  
    
}  // namespace E1App


