#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"


namespace IpsiusService
{    

	class AppLauncher
	{
	public:
		typedef boost::function<int()> RunAppFn;     
		typedef boost::function<void()> RunInMainThreadFn;

		static int StartApp(int argc, char *argv[], const RunAppFn &fRun); // Init. main() thread context!

		static void AsyncCallInMainThread(const RunInMainThreadFn &fRun);
        static void SyncCallInMainThread(const RunInMainThreadFn &fRun); // Not main() thread context!

        static bool IsMainThreadContext();

        static void AssertIsMainThreadContext(bool isMain) 
        {
            ESS_ASSERT(!isMain ^ IsMainThreadContext()); 
        }
	};
     
} // namespace IpsiusService

#endif
