#ifndef __AOZEXEC__
#define __AOZEXEC__

namespace DrvAoz
{
    
    struct AozExecConfig
    {
        bool CpuUsage;

        AozExecConfig()
        {
            CpuUsage = false;
        }
    };

    void RunAoz(const AozExecConfig &config);
    
}  // namespace DrvAoz

#endif
