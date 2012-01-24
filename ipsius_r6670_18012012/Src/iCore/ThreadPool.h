#ifndef __THREADPOOL__

#define __THREADPOOL__

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/ILockable.h"
#include "MsgBase.h"
#include "MsgProcessor.h"

namespace iCore
{
    using Platform::dword;
    
    class ThreadPoolThread;
    class MsgObject;

    // ќбъект, позвол€ющий выполн€ть "задачи" на нескольких потоках
    class ThreadPool : boost::noncopyable
	{
        typedef ThreadPoolThread Thread;

        enum { CDefaultThreads = 3, };

        class TaskList : public std::queue<MsgBase*>, public Utils::ILockable {};
        typedef std::vector<Thread*> ThreadList;

        TaskList m_tasks;
        ThreadList m_threads;  // TODO -- replace with ManagedList<T>
        dword m_maxThreads;
        Platform::Thread::Priority m_priority;
        Utils::SafeRef<IMsgProcessorErrorHook> m_defaultHook;

        bool PlaceTaskToThread(MsgBase *pT);
        void TryPeekTask();

        friend class ThreadPoolThread;
        MsgBase* PeekTask();


    public:

        ThreadPool(Platform::Thread::Priority priority, dword maxThreads = CDefaultThreads) 
            : m_maxThreads(maxThreads), m_priority(priority)
        {
            // ...
        }

        ~ThreadPool();

        void Schedule(MsgBase *pTask);
        void RemoveTasksFor(MsgObject *pObject);

        void SetErrorHook(Utils::SafeRef<IMsgProcessorErrorHook> hook);

	};
	
} // namespace iCore


#endif

