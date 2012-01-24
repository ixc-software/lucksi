#ifndef __PLATFORMTHREAD__
#define __PLATFORMTHREAD__

#include "iVDK/VdkThread.h"

namespace Platform
{
	
	class Thread : public iVDK::detail::IThreadBody, boost::noncopyable
	{

    public:

        enum Priority 
        { 
            TimeCriticalPriority = VDK::kPriority20,
            HighestPriority		 = TimeCriticalPriority - 1,  // VDK::kPriority21, ... (decrement increase kPriorityXX)
            HighPriority         = TimeCriticalPriority - 2,
            NormalPriority		 = TimeCriticalPriority - 3,
            LowPriority			 = TimeCriticalPriority - 4,
            LowestPriority		 = TimeCriticalPriority - 5,
            IdlePriority		 = TimeCriticalPriority - 6,

            // special
            InheritPriority = -1,
        };
        
        BOOST_STATIC_ASSERT(NormalPriority == VDK::kPriority23);

        typedef int ThreadID;

        static ThreadID GetCurrentThreadID()
        {
            return iVDK::Thread::GetCurrentThreadID();
        }

        static void Sleep(int msTime)
        {
            iVDK::Thread::Sleep(msTime);
        }

        static void CurrentThreadPriority(Priority priority)
        {
            iVDK::Thread::CurrentThreadPriority( ConvertPriority(priority) );
        }

        static std::string GetCurrentThreadName();
        static std::string ThreadIdToStr(ThreadID id);


        Thread(const std::string &name) : m_name( MakeThreadName(name) ), m_thread(*this)
        {
        }

        bool isRunning() const
        {
            return m_thread.IsRunning();
        }

        bool isFinished() const
        {
            return m_thread.IsFinished();
        }

        void wait()
        {
            m_thread.Wait();
        }

        void setPriority(Priority priority)
        {
            m_thread.SetPriority( ConvertPriority(priority) );
        }

    protected:

        void start(Priority priority = InheritPriority)
        {
            m_thread.Start( ConvertPriority(priority) );
        }

    // IThreadBody impl
    private:

        void ThreadBody();

    private:

        static std::string MakeThreadName(const std::string &name);

        static int ConvertPriority(Priority p)
        {
            return static_cast<int>(p);
        }

        virtual void run() = 0;

        const std::string m_name;
        iVDK::Thread m_thread;
    };
	
	
}  // namespace Platform

#endif

