#include "stdafx.h"
#include "AppLauncher.h"
#include "Utils/ErrorsSubsystem.h"

namespace IpsiusService
{
	int AppLauncher::StartApp(int argc, char *argv[], const RunAppFn &fRun)
    {
		QCoreApplication a(argc, argv);
        return fRun();
    }    

	void AppLauncher::AsyncCallInMainThread(const RunInMainThreadFn &fRun)
	{
        ESS_HALT("Unexpected");
	}

    void AppLauncher::SyncCallInMainThread(const RunInMainThreadFn &fRun)
    {
        ESS_HALT("Unexpected");
    }

    bool AppLauncher::IsMainThreadContext()
    {
        return false;
    }        

} // namespace IpsiusService
