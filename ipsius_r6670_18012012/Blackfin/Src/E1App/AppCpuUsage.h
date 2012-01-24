#ifndef __APPCPUUSAGE__
#define __APPCPUUSAGE__

    #include "Utils/CPUUsageThread.h"
    
namespace E1App
{

    // Utils::CPUUsageThread wrapper
    class AppCpuUsage
    {
        boost::scoped_ptr<Utils::CPUUsageThread> m_cpu;

        static Utils::CPUUsageThreadProfile CpuUsageProfile()
        {
            Utils::CPUUsageThreadProfile profile;

            profile.ThreadPriority = Platform::Thread::LowestPriority;
            profile.CalibrationPeriodMs = 50;
            profile.MeasuringCycleMs = 1000; 
            profile.SleepPeriodPercent = 0; // 1%
            profile.LoadQuantumPercent = 5;
            profile.OperationsPerTaskCount = 500;
            profile.MinTaskDurationMcs = 50;
            profile.MaxTaskDurationMcs = 1000;

            return profile;
        }
    	
    public:
    	
    	AppCpuUsage(bool enable)
    	{
            if (enable)
            {
                m_cpu.reset( new Utils::CPUUsageThread( CpuUsageProfile() ) );
                m_cpu->Calibrate();
            }
    	}

        std::string CalibrationInfo()
        {
            if (m_cpu.get() == 0) return "(no cpu usage)";

            std::ostringstream ss;

            //float ratio = m_cpu->TaskCountOnCalibration();
            //ratio /= m_cpu->TaskDurOnCalibrationMcs();

            ss << "Calibration tasks " << m_cpu->TaskCountOnCalibration()
               << ", single task duration (mcs) " << m_cpu->TaskDurOnCalibrationMcs();              

            return ss.str();
        }
    
        int CpuLoad()  // in percent
        {
            if (m_cpu.get() == 0) return -1;

            Utils::CPUUsageData data;
            data = m_cpu->GetCPUUsageData();

            return data.CPUUsagePercent();
        }
    
    };
    
    
}  // namespace E1App
    

#endif
