#include "stdafx.h"
#include "ProjConfigLocal.h"

namespace
{
    using namespace ProjConfig;

    class CfgInfoPrint
    {
        static void Log(const std::string &msg)
        {
            std::cout << "WARNING!! " << msg << std::endl;
        }

    public:

        CfgInfoPrint()
        {
            if (!CfgInfo::CPrintConfigWarnings) return;

            // SafeRef
            if (CfgSafeRef::CEnableDebug)
            {
                Log("SafeRef debug enabled!");
            }

            // global new/delete
            #ifdef ENABLE_GLOBAL_NEW_DELETE
                Log("Global new/delete debug enabled!");
            #endif

            // boost::shared_ptr 
            if (!CfgSharedPtr::CEnableThreadSafety)
            {
                Log("Thread unsafe boost::shared_ptr!");
            }

            // log
            if (CfgLogWrapper::CDisableLogging)
            {
                Log("Logging disabled!");
            }

            // ESS
            #ifndef ENABLE_ESS_ASSERTIONS 
                Log("ESS_ASSERT disabled!!");
            #endif


        }
    };

    //-------------------------------------------

    // global instance
    CfgInfoPrint GCfgInfo;
}

//-------------------------------------------

namespace Utils
{
    void CfgInfoDummi()
    {
        // nothing, used for forced include this .cpp file
    }
}
