#ifndef __PLATFORMTHREAD__
#define __PLATFORMTHREAD__

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"

#include "Platform/PlatformTypes.h"

namespace Platform
{

    namespace detail
    {
        class IThreadBody : public Utils::IBasicInterface
        {
        public:
            virtual void ThreadBody() = 0;
        };

        class ThreadImpl : public QThread
        {
            IThreadBody &m_body;

            void run()
            {
                m_body.ThreadBody();
            }

        public:

            ThreadImpl(IThreadBody &body) : m_body(body)
            {
            }

            ~ThreadImpl()
            {
                QThread::wait();
            }

            static void Sleep(dword interval)
            {
                QThread::usleep(interval * 1000UL);
            }
        };

    }  // namespace detail
	
    class Thread : 
        public detail::IThreadBody, 
        boost::noncopyable
	{
    public:

        Thread(const std::string &name) : m_name( MakeThreadName(name) ), m_impl(*this)
        {
        }

        enum Priority 
        { 
            IdlePriority            = QThread::IdlePriority, 
            LowestPriority          = QThread::LowestPriority, 
            LowPriority             = QThread::LowPriority,
            NormalPriority          = QThread::NormalPriority,
            HighPriority            = QThread::HighPriority,
            HighestPriority         = QThread::HighestPriority,
            TimeCriticalPriority    = QThread::TimeCriticalPriority,

            InheritPriority         = QThread::InheritPriority,
        };

        typedef Qt::HANDLE ThreadID;

        static ThreadID GetCurrentThreadID()
        {
            return QThread::currentThreadId();
        }

        static std::string GetCurrentThreadName();

        static std::string ThreadIdToStr(ThreadID id);

        static void Sleep(int msInterval)
        {
            detail::ThreadImpl::Sleep(msInterval);
        }

        static void CurrentThreadPriority(Priority priority)
        {
            QThread *pThread = QThread::currentThread();
            ESS_ASSERT(pThread != 0);
            pThread->setPriority( ToQtPriority(priority) );
        }

        bool isRunning() const
        {
            return m_impl.isRunning();
        }

        bool isFinished() const
        {
            return m_impl.isFinished();
        }

        void wait()
        {
            m_impl.wait();
        }

        void setPriority(Priority priority)
        {
            m_impl.setPriority( ToQtPriority(priority) );
        }

    protected:

        void start(Priority priority = InheritPriority)
        {
            m_impl.start( ToQtPriority(priority) );
        }

    // IThreadBody impl
    private:

        void ThreadBody();


    private:

        virtual void run() = 0;

        static QThread::Priority ToQtPriority(Priority prio)
        {
            return static_cast<QThread::Priority>(prio);
        }

        static std::string MakeThreadName(const std::string &name);

        const std::string m_name;
        detail::ThreadImpl m_impl;
    };
	
	
}  // namespace Platform

#endif

    
