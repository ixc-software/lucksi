
#ifndef __CPUUSAGETHREAD__
#define __CPUUSAGETHREAD__

// CPUUsageThread.h

#include "Platform/PlatformTypes.h"
#include "Platform/PlatformThread.h"
#include "Platform/PlatformMutex.h"
#include "Utils/AtomicTypes.h"
#include "Utils/Random.h"

namespace Utils
{
    using Platform::dword;
    using Platform::byte;
    using Platform::ddword;

    /*
    
    CPUUsageThread working scheme
    _____________________________

      User         CPUUsageThread(CPUUsageThreadProfile)
        |                |  
        |                |                           
        |                |           Thread(Priority)
        |-- Calibrate -->|--- start --->|   
        |                |              |--------------------\
        |                |              |                     \ CalibrationPeriod
        |                |              |                     /
        |                |              |--------------------/
        |                |              | 
        |                |              | Loop(!stopMonitoring)
        |                |              |----------------------------------\
        |                |              |------------------------\          \
        |                |              |-----------\             \          \
        |                |              |  RunTasks  \ LoadQuantum \          \
        |                |              |            / Percent      \  Load    \
        |                |              |----------/  (of tasks)   /  Period    \ Measuring
        |                |              |                        /               \ Cycle
        |                |              |----------------------/                /
        |                |              |----------------------\               /
        |                |              |                       \  Sleep      /
        |                |              |                       / Period     /
        |                |              |----------------------/            /
        |                |              |----------------------------------/
        |--- destroy --->|--- stop ---->|
        |                | Monitoring   x
        |                x

        Notes:
        - MeasuringCycle - infinite cycle of LoadPeriods
        - LoadPeriod has 2 stages: Load and Sleep
        - On LoadPeriod we check timer after 
          running definite amount of tasks (LoadQuantumPercent)

        - all periods in msec
    */

    // Profile for CPUUsageThread
    struct CPUUsageThreadProfile
    {
        Platform::Thread::Priority ThreadPriority;
        dword CalibrationPeriodMs;
        dword MeasuringCycleMs; 
        byte SleepPeriodPercent;
        byte LoadQuantumPercent;
        dword OperationsPerTaskCount;
        dword MinTaskDurationMcs;
        dword MaxTaskDurationMcs;
        
        CPUUsageThreadProfile(); // Set default parameters values

        bool Validate() const;
        dword LoadPeriodMs() const;
        dword SleepPeriodMs() const;
        bool CheckTaskDuration(dword durationMicroSec) const;
    };

    // ------------------------------------------------------

    namespace CPUUsageThreadDetail
    {
        // Using for running one task per RunTask() call
        class TaskRunner
        {
            Utils::Random m_random;
            volatile Platform::ddword m_summ;
            dword m_operationsPerTaskCount;

        public:
            TaskRunner(dword operationsPerTaskCount);
            
            void RunTask();
        };
        
        
    } // namespace CPUUsageThread

    // ------------------------------------------------------

    class CPUUsageData
    {
        byte m_CPUUsagePercent;
        // to calculate average usage
        Platform::ddword m_summ;
        ddword m_counter;

    private:
        friend class CPUUsageThread;

        void Set(byte val);

    public:
        // start values == calibration data: 
        // m_CPUUsagePercent = 0, m_summ = 0, m_counter = 1
        CPUUsageData();

        int CPUUsagePercent() const;
        int CPUUsagePercentAverage() const;
        ddword MeasuringCounter() const;
    };

    // ------------------------------------------------------
    
    // Using for monitoring CPU usage.
    // To start monitoring need to call Calibrate()
    class CPUUsageThread:
        Platform::Thread
    {
        CPUUsageThreadProfile m_profile;

        // calibration data
        dword m_calibrTaskCount;
        dword m_calibrTaskDurMcs;
        bool m_calibrated;

        // using for exit from MeasuringCycle
        Utils::AtomicBool m_stopMonitoring;

        // measuring data
        Platform::Mutex m_mutex;
        CPUUsageData m_CPUUsageData;

        // using for running 1 task
        CPUUsageThreadDetail::TaskRunner m_taskRunner;
        // number of tasks running before timer check
        dword m_loadPercentInTasks;
        
        void run(); // override
        void RunCalibration();
        void SetCalibrationParams(dword taskCount, dword taskDurMcs);
        void Load();
        void CalculateCPUUsage(dword tasks);
        // static dword CalcPercentOfTime(dword time, byte percent);
        
    public:
        CPUUsageThread(const CPUUsageThreadProfile &profile);
        ~CPUUsageThread();

        void Calibrate();

        CPUUsageData GetCPUUsageData();

        dword TaskCountOnCalibration();
        dword TaskDurOnCalibrationMcs();
    };
    
    
} // namespace  Utils

#endif
