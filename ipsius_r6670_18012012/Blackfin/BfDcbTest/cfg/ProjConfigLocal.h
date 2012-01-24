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

	
}  // namespace ProjConfig

#endif
