
#include "stdafx.h"
#include "CPUUsageThread.h"

#include "Platform/PlatformMutexLocker.h"
#include "Utils/TimerTicks.h"


namespace 
{
    using namespace Utils;

    dword CalcPercentOfTime(dword time, byte percent)
    {
        return (time * percent / 100);
    }
    
} // namespace

// -----------------------------------------------------------------

// Profile constants
namespace
{
    const Platform::Thread::Priority CDefaultPriority = Platform::Thread::LowestPriority;
    const dword CDefaultMeasuringCycleMs = 1000;
    const byte CDefaultSleepPeriodPercent = 10;
    const byte CDefaultLoadQuantumPercent = 10;
    const dword CDefaultCalibrationPeriodMs = 1000;
    const dword CDefaultOperationsPerTaskCount = 2*1000;
    const dword CDefaultMinTaskDurationMcs = 50;
    const dword CDefaultMaxTaskDurationMcs = 1000;
}

// -----------------------------------------------------------------

namespace Utils
{
    namespace CPUUsageThreadDetail
    {
        TaskRunner::TaskRunner(dword operationsPerTaskCount) :
            m_random(Utils::TimerTicks::Ticks()), m_summ(0), 
            m_operationsPerTaskCount(operationsPerTaskCount)
        {
        }

        // ----------------------------------------------------------------- 

        void TaskRunner::RunTask()
        {
            for (size_t i = 0; i < m_operationsPerTaskCount; ++i)
            {
                m_summ += m_random.Next(m_operationsPerTaskCount);
            }
        }
        
    } // namespace CPUUsageThread
    
    // -----------------------------------------------------------------
    // CPUUsageThreadProfile impl

    CPUUsageThreadProfile::CPUUsageThreadProfile()
    {
        ThreadPriority = CDefaultPriority;
        MeasuringCycleMs = CDefaultMeasuringCycleMs; 
        SleepPeriodPercent = CDefaultSleepPeriodPercent;
        LoadQuantumPercent = CDefaultLoadQuantumPercent;
        CalibrationPeriodMs = CDefaultCalibrationPeriodMs;
        OperationsPerTaskCount = CDefaultOperationsPerTaskCount;
        MinTaskDurationMcs = CDefaultMinTaskDurationMcs;
        MaxTaskDurationMcs = CDefaultMaxTaskDurationMcs;
    }

    // -----------------------------------------------------------------
    
    bool CPUUsageThreadProfile::Validate() const
    {
        return ((MeasuringCycleMs > 0) &&
                (SleepPeriodPercent <= 100) &&
                (LoadQuantumPercent <= 100) &&
                (CalibrationPeriodMs > 0) &&
                (OperationsPerTaskCount > 0) &&
                (MinTaskDurationMcs > 0) &&
                (MaxTaskDurationMcs > MinTaskDurationMcs));
    }

    // -----------------------------------------------------------------
    
    dword CPUUsageThreadProfile::LoadPeriodMs() const
    {
        return MeasuringCycleMs - SleepPeriodMs();
    }

    // -----------------------------------------------------------------
    
    dword CPUUsageThreadProfile::SleepPeriodMs() const
    {
        return MeasuringCycleMs * SleepPeriodPercent / 100;
    }
    
    // -----------------------------------------------------------------

    bool CPUUsageThreadProfile::CheckTaskDuration(dword durationMcs) const
    {
        return ((durationMcs >= MinTaskDurationMcs) 
                 && (durationMcs <= MaxTaskDurationMcs));
    }

    // -----------------------------------------------------------------
    // CPUUsageThread::CPUUsageData impl
    
    CPUUsageData::CPUUsageData() : 
        m_CPUUsagePercent(0), m_summ(0), m_counter(1)
    {
    }

    // -----------------------------------------------------------------

    void CPUUsageData::Set(byte val)
    {
        ESS_ASSERT(val <= 100);
        
        m_CPUUsagePercent = val;
        m_summ += val;
        ++m_counter;
    }

    // -----------------------------------------------------------------

    int CPUUsageData::CPUUsagePercent() const
    { 
        return m_CPUUsagePercent; 
    }

    // -----------------------------------------------------------------

    int CPUUsageData::CPUUsagePercentAverage() const 
    { 
        ESS_ASSERT(m_counter != 0);
        return (m_summ / m_counter); 
    } 

    // -----------------------------------------------------------------

    ddword CPUUsageData::MeasuringCounter() const
    {
        return m_counter;
    }
    
    // -----------------------------------------------------------------
    // CPUUsageThread impl

    CPUUsageThread::CPUUsageThread(const CPUUsageThreadProfile &profile) : 
        Platform::Thread("CPUUsageThread"),
        m_profile(profile), m_calibrTaskCount(0), m_calibrTaskDurMcs(0), m_calibrated(false), 
        m_stopMonitoring(false), m_taskRunner(m_profile.OperationsPerTaskCount), 
        m_loadPercentInTasks(0)
    {
        ESS_ASSERT(m_profile.Validate());
    }
    
    // -----------------------------------------------------------------
    
    CPUUsageThread::~CPUUsageThread()
    {
        m_stopMonitoring.Set(true);
        wait();
    }

    // -----------------------------------------------------------------

    void CPUUsageThread::run() // override
    {
        RunCalibration();

        while (!m_stopMonitoring)
        {
            Load();

            int sleepTime = m_profile.SleepPeriodMs();
            if (sleepTime > 0) Sleep(sleepTime);
        }
    }

    // -----------------------------------------------------------------

    // Durring calibraation main thread is sleeping
    void CPUUsageThread::RunCalibration()
    {
        dword runs = 0;
        // dword runStartTime = Utils::TimerTicks::Ticks();
        // while ((Utils::TimerTicks::Ticks() - runStartTime) < m_profile.CalibrationPeriodMs)
        Utils::TimerTicks timer;
        timer.Reset();
        while (timer.Get() < m_profile.CalibrationPeriodMs)
        {
            m_taskRunner.RunTask();
            ++runs;
        }
        // dword realRunTimeMs = Utils::TimerTicks::Ticks() - runStartTime;
        dword realRunTimeMs = timer.Get();
        ESS_ASSERT(runs != 0);
        ESS_ASSERT(realRunTimeMs != 0);

        // count one task duration
        dword taskDurationMcs = realRunTimeMs * 1000 / runs;
        // count how many task will be runned in one LoadPeriod if CPU usage == 0
        dword taskCount = m_profile.LoadPeriodMs() * runs / realRunTimeMs;
        SetCalibrationParams(taskCount, taskDurationMcs);
        
        m_calibrated = true;
    }
    
    // -----------------------------------------------------------------

    void CPUUsageThread::SetCalibrationParams(dword taskCount, dword taskDurMcs)
    {
        ESS_ASSERT(taskCount != 0);
        ESS_ASSERT(m_profile.CheckTaskDuration(taskDurMcs));
        m_calibrTaskCount = taskCount;
        m_calibrTaskDurMcs = taskDurMcs;

        /*
        std::cout << "taskDurationMcs -- " << taskDurMcs << std::endl
            << "taskCount -- " << taskCount << std::endl;
        */
        
        // calculate LoadPercent in tasks
        m_loadPercentInTasks = m_calibrTaskCount * m_profile.LoadQuantumPercent / 100;
    }
    
    // -----------------------------------------------------------------

    void CPUUsageThread::Load()
    {
        ESS_ASSERT(m_loadPercentInTasks != 0);
        
        dword realRuns = 0;
        dword startTime = Utils::TimerTicks::Ticks();
        while ((Utils::TimerTicks::Ticks() - startTime) < m_profile.LoadPeriodMs())
        {
            dword i = 0;
            while (i < m_loadPercentInTasks)
            {
                if (m_stopMonitoring) return;
                
                m_taskRunner.RunTask();
                ++i;
            }
            realRuns += i;
        }
        dword realRunTime = Utils::TimerTicks::Ticks() - startTime;
        dword periodRuns = realRuns * m_profile.LoadPeriodMs() / realRunTime;

        if (periodRuns == 0) periodRuns = 1;  // fix, sometimes it happend

        CalculateCPUUsage(periodRuns);
    }

    // -----------------------------------------------------------------

    void CPUUsageThread::CalculateCPUUsage(dword tasks)
    {
        ESS_ASSERT(tasks != 0);
        
        // recalibrate if need
        dword newTaskCount = (tasks > m_calibrTaskCount) ? tasks : m_calibrTaskCount;
        dword newTaskDurMcs = m_calibrTaskDurMcs * m_calibrTaskCount / newTaskCount;

        byte newCPUUsagePercent = (newTaskCount - tasks) * 100 / newTaskCount;
        ESS_ASSERT(newCPUUsagePercent <= 100);

        {
            Platform::MutexLocker lock(m_mutex);
            SetCalibrationParams(newTaskCount, newTaskDurMcs);
            m_CPUUsageData.Set(newCPUUsagePercent);
        }

        /*
        std::cout << "CPU usage " << (int)m_CPUUsageData.CPUUsagePercent() << "%" << std::endl
                    << "CPU usage average " << (int)m_CPUUsageData.CPUUsagePercentAverage() 
                    << "%" << std::endl;
        */
    } 

    // -----------------------------------------------------------------

    dword CPUUsageThread::TaskCountOnCalibration()
    {
        ESS_ASSERT(m_calibrated);

        dword res = 0;
        {
            // Using mutex in case of possibility of recalibration
            Platform::MutexLocker locker(m_mutex);
            res = m_calibrTaskCount;
        }
        return res;
    }

    // -----------------------------------------------------------------

    dword CPUUsageThread::TaskDurOnCalibrationMcs()
    {
        ESS_ASSERT(m_calibrated);
        
        dword res = 0;
        {
            Platform::MutexLocker locker(m_mutex);
            res = m_calibrTaskDurMcs;
        }
        return res;
    }
    
    // -----------------------------------------------------------------

    void CPUUsageThread::Calibrate()
    {
        ESS_ASSERT(!m_calibrated);
        
        start(m_profile.ThreadPriority);
        
        // wait calibration
        byte percent = 110;
        dword sleepTime = CalcPercentOfTime(m_profile.CalibrationPeriodMs, percent);
        Sleep(sleepTime);

        ESS_ASSERT(m_calibrated);
    }

    // -----------------------------------------------------------------

    CPUUsageData CPUUsageThread::GetCPUUsageData()
    {
        ESS_ASSERT(m_calibrated);
        CPUUsageData res;
        {
            Platform::MutexLocker locker(m_mutex);
            res = m_CPUUsageData;
        } // unlock
        
        return res;
    }
    
} // namespace Utils

