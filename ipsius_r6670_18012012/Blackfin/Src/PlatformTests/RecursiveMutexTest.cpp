#include "stdafx.h"

#include "iVDK/VdkThread.h"
#include "iVDK/VdkUtils.h"
#include "Platform/Platform.h"
#include "Utils/MsDateTime.h"

#include "RecursiveMutexTest.h"

// -------------------------------------------------

namespace
{
	using iVDK::detail::IThreadBody;

	
	class ControlThread : public IThreadBody
    {
        bool &m_completed;
        iVDK::Thread m_thread;

    // IThreadBody impl
    private:

        void ThreadBody()
        {
            iVDK::Sleep(3);
            ESS_ASSERT(m_completed);
        }

    public:

        ControlThread(bool &completed) : 
            m_completed(completed), m_thread(*this)
        {
            m_thread.Start(-1);
        }

        ~ControlThread()
        {
            m_thread.Wait();
        }
    };

    void RunRecursive(int depth)
    {
        if (depth == 0) return;

        Platform::Mutex mutex;

        {
            Platform::MutexLocker locker(mutex);
            depth--;
            RunRecursive(depth);
        }
    }

		
}  // namespace

// -------------------------------------------------

namespace PlatformTests
{
	
	void RunRecursiveMutexTest()
	{
        bool completed = false;

        ControlThread thread(completed);

        {
            int depth = 10;
            RunRecursive(depth);
        }

        {
            Utils::MsDateTime dt;
            dt.Capture();
        }

        completed = true;
	}
	
}  // namespace PlatformTests
