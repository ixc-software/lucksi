#include "stdafx.h"
#include "AppLauncher.h"
#include "AppLauncherImpl.h"

// ------------------------------------------------------------------------------------

namespace IpsiusService
{
    IpsiusService::AppLauncherImpl* GRunnerRef = 0;

    int AppLauncherImpl::RunApp(int argc, char *argv[])
    {
        QApplication a(argc, argv);
        a.setQuitOnLastWindowClosed(false);

        using Platform::Thread;
        Thread::CurrentThreadPriority(Thread::LowestPriority);

        QTimer::singleShot(0, this, SLOT(OnRunDomain()));                 

        ESS_ASSERT(a.exec() == 0); // return if domain thread closed

        return m_runer->getExitCode();
    }
};

// ------------------------------------------------------------------------------------

namespace IpsiusService
{    

	int AppLauncher::StartApp(int argc, char *argv[], const RunAppFn &fRun)
    {
		ESS_ASSERT(GRunnerRef == 0);
        
		AppLauncherImpl runner(fRun);
		GRunnerRef = &runner;
        int res = runner.RunApp(argc, argv);
        GRunnerRef = 0;
        return res;
    }    

    // --------------------------------------------------------------------------------------

    void AppLauncher::AsyncCallInMainThread(const RunInMainThreadFn &fRun)
	{
		ESS_ASSERT(GRunnerRef != 0 && "Run domain first throught AppLauncher::StartApp");
		GRunnerRef->AsyncRunGui(fRun);
    }

    // ------------------------------------------------------------------------------------

    bool AppLauncher::IsMainThreadContext()
    {
        if (GRunnerRef == 0) return false;
        return GRunnerRef->IsGuiThreadContext();
    }

    // ------------------------------------------------------------------------------------

    void AppLauncher::SyncCallInMainThread( const RunInMainThreadFn &fRun )
    {
        ESS_ASSERT(GRunnerRef != 0 && "Run domain first throught AppLauncher::StartApp");
        GRunnerRef->SyncRunGui(fRun);
    }
} // namespace IpsiusService
