
#include "stdafx.h"

#include "CPUUsageThreadTest.h"

#include "Utils/CPUUsageThread.h"

#include "Utils/TimerTicks.h"

namespace
{
    using namespace Utils;

    const dword CCalibrationPeriodMs = 500;
    const dword CMeasuringPeriodMs = 200;
    const byte CSleepPeriodPercent = 10;
    const byte CLoadPercent = 20;

    const dword CTasksCalibrationPeriod = 500;
    const dword CTestDurationMs = 2100;
    const dword CTestLoadMs = 250;
    const dword CTestSleepMs = 250;

    // ----------------------------------------------------

    // Using for running task: 50% of time -- run, 50% of time - sleep
    // Average CPU usage has to be 50%
    class TaskRunningThread : Platform::Thread
    {
        dword m_loadPeriodMs;
        dword m_sleepPeriodMs;
        bool m_stop;

        // returns false if test finished
        bool Load(dword interval)
        {
            Utils::TimerTicks loadTimer;
            loadTimer.Set(interval);

            while (!loadTimer.TimeOut())
            {
                if (m_stop) return false;
            }

            return true;
        }
        
        void run() // override
        {
            while (true)
            {
                if (!Load(m_loadPeriodMs)) return;

                if (m_sleepPeriodMs == 0) continue;
                Sleep(m_sleepPeriodMs);
            }
        }
        
    public:
        TaskRunningThread(dword loadPeriodMs, dword sleepPeriodMs,
                          Platform::Thread::Priority priority = NormalPriority) : 
            m_loadPeriodMs(loadPeriodMs), m_sleepPeriodMs(sleepPeriodMs), m_stop(false)
        {
            start(priority);  
        }

        ~TaskRunningThread()
        {
            m_stop = true;
            wait();
        }
    };

    // ----------------------------------------------------

    CPUUsageThreadProfile MakeProfile()
    {
        CPUUsageThreadProfile res;
        res.CalibrationPeriodMs = CCalibrationPeriodMs;
        res.MeasuringCycleMs = CMeasuringPeriodMs;
        res.SleepPeriodPercent = CSleepPeriodPercent;
        res.LoadQuantumPercent = CLoadPercent;

        return res;
    }

    // ----------------------------------------------------

    dword CalcPercentOfTime(dword time, byte percent)
    {
        return (time * percent / 100);
    }
    
    // ----------------------------------------------------

    // returns CPU usage
    byte CalibrateTasks(dword period)
    {
        CPUUsageThread monitor(MakeProfile());
        monitor.Calibrate();

        {
            TaskRunningThread calibrateTasks(CTestLoadMs, CTestSleepMs);
            Platform::Thread::Sleep(CalcPercentOfTime(period, 110));
        }
        
        return monitor.GetCPUUsageData().CPUUsagePercentAverage();
    }
    
} // namespace

// ----------------------------------------------------

namespace UtilsTests
{
    using namespace Utils;
    
    void CPUUsageThreadTest(bool silentMode)
    {
        if (!silentMode) std::cout << "CPUUsageThread test begin ...\n";
        {
            dword tasksCPUUsage = CalibrateTasks(CTasksCalibrationPeriod);

            // run test
            CPUUsageThread monitor(MakeProfile());
            monitor.Calibrate();

            {
                TaskRunningThread tasks(CTestLoadMs, CTestSleepMs);
                Platform::Thread::Sleep(CalcPercentOfTime(CTestDurationMs, 110));
            }

            // check
            byte testAvgCPUUsage = monitor.GetCPUUsageData().CPUUsagePercentAverage();

            // CTestLoadMs / CTestSleepMs ratio 
            byte percentOfTaskCPUUsage = CTestLoadMs * 100 / (CTestLoadMs + CTestSleepMs);
            byte expectedCPUUsage = tasksCPUUsage * percentOfTaskCPUUsage / 100;
            // +/- 10% of 100% CPU usage
            byte delta = (expectedCPUUsage > 10)? 10 : expectedCPUUsage; 

            if (!silentMode)
            {
                std::cout << "Average -- " <<  (int)testAvgCPUUsage << "%" << std::endl
                            << "Expected -- " << (int)expectedCPUUsage << "%" << std::endl
                            << "Accepted range -- " << "+/-" << (int)delta << " CPU usage %" << std::endl;
            }

            if (CTestLoadMs > 0) TUT_ASSERT(testAvgCPUUsage != 0);

            TUT_ASSERT((testAvgCPUUsage >= (expectedCPUUsage - delta)) 
                        && (testAvgCPUUsage <= (expectedCPUUsage + delta))); 

            
        }
        
        if (!silentMode) std::cout << "CPUUsageThread test okay\n";
    }

    // ----------------------------------------------------
    
    void RunMonitoring(Platform::dword CPUCheckCount, Platform::dword sleepBetweenChecksMs)
    {
        CPUUsageThreadProfile profile;
        CPUUsageThread thread(profile);
        thread.Calibrate();
        std::cout << "Calibration:" << std::endl
            << "task count -- " << thread.TaskCountOnCalibration() << std::endl
            << "task duration (ms) -- " << thread .TaskDurOnCalibrationMcs() << std::endl;
        
        for (int i = 0; i < CPUCheckCount; ++i)
        {
            std::cout << "CPUUsage -- " 
                    << (int)thread.GetCPUUsageData().CPUUsagePercent() << std::endl;
            Platform::Thread::Sleep(sleepBetweenChecksMs);
        }

        std::cout << "CPUUsage average -- " 
                    << (int)thread.GetCPUUsageData().CPUUsagePercentAverage() << std::endl;
    }

    // ----------------------------------------------------

    void Win32CpuUsageTest()
    {
        using namespace std;
        
        // for 2x core
        // TaskRunningThread load100(100, 0, Platform::Thread::LowPriority);
        
        {
        	/*
        	using namespace Platform;
        	
        	cout << VDK::kPriority20 << endl;
        	cout << VDK::kPriority21 << endl;
        	
        	cout << Thread::TimeCriticalPriority << endl;
        	cout << Thread::LowPriority << endl;
        	cout << Thread::LowestPriority << endl; */
        }

        CPUUsageThreadProfile profile;

        profile.ThreadPriority = Platform::Thread::LowestPriority;
        profile.CalibrationPeriodMs = 100;
        profile.MeasuringCycleMs = 1000; 
        profile.SleepPeriodPercent = 1;
        profile.LoadQuantumPercent = 5;
        profile.OperationsPerTaskCount = 250;
        profile.MinTaskDurationMcs = 50;
        profile.MaxTaskDurationMcs = 1000;

        CPUUsageThread thread(profile);

        thread.Calibrate();

        cout << "Calibrated!" << endl;

        TaskRunningThread load(93, 7, Platform::Thread::LowPriority);

        while(true)
        {
            Platform::ThreadSleep(1000);

            CPUUsageData data;
            data = thread.GetCPUUsageData();

            cout << data.CPUUsagePercent() << "%" << endl;
        }
    }

    
} // namespace UtilsTests
