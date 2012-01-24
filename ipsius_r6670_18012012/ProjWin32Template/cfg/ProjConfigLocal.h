#ifndef __PROJCONFIGLOCAL__
#define __PROJCONFIGLOCAL__

namespace ProjConfig
{

    // print warning messages about config settings at application startup
    // implementation in Utils/CfgInfo.cpp
    class CfgInfo
    {
    public:

        const static bool CPrintConfigWarnings = true;
    };

    // ----------------------------------------------------------

    class CfgESS
    {
    public:

        enum 
        { 
            // использовать захват данных о стеке
            CUseStackTrace = true,              

            /* использовать stack trace для типов, унаследованных от ESS::Exception;
            отключение -- своего рода оптимизация, т.к. формирование stack trace 
            довольно накладная операция, и может снизить производительность кода, 
            в котором активно возникают исключения
            */
            CStackTraceInExceptions = true,   
        };  

    };

    // ----------------------------------------------------------

    // view settings descriptions in Utils/SafeRef.cpp
    class CfgSafeRef
    {
    public:

        const static bool CEnableDebug = false; // global enable/disable

        const static bool CCacheServerType = true;
        const static bool CRawNamesMode = true;

    };

    // ----------------------------------------------------------

    // Global new/delete can be enabled by define in ProjConfigGlobal.h
    // implementation in Utils/GlobalNewDelete.cpp
    class GlobalNewDelete
    {
    public:

        // enable store-list for all allocated blocks -- really SLOW mode
        const static bool CUseBlockList = false;
    };

    // ----------------------------------------------------------

    // Debug mode for Utils/MemoryPool.h
    class CfgMemoryPool
    {
    public:

        const static bool CEnableDebug      = true;
        const static bool CEnableThreadSafe = true;
    };
    
    // ----------------------------------------------------------

	// iCore::TimerThread watchdog param
	class CfgTimerThread
	{
	public:
		
		const static int CMaxMessageTimeMs = 0;
		const static int CSleepTimeMs = 5;		
	};    
    
	
}  // namespace ProjConfig

#endif

