#ifndef _THREAD_TASK_RUNNER_
#define _THREAD_TASK_RUNNER_

#include "stdafx.h"
#include "Platform/Platform.h"
#include "IBasicInterface.h"
#include "ManagedList.h"
#include "IExecutor.h"


namespace Utils
{
    class ThreadTaskRunner : boost::noncopyable
    {

        template <class Aplication, class Config> 
            class Runner : public Platform::Thread
        {

            boost::scoped_ptr<Aplication> m_aplication;
            //Config m_cfg;
            const boost::shared_ptr<Config> m_cfg;

            void run()  // override
            {
                m_aplication.reset(new Aplication(m_cfg));
            }

        public:

            Runner(const boost::shared_ptr<Config> cfg, Platform::Thread::Priority priority) : 
              Platform::Thread("Runner"),
              m_cfg(cfg)
            {
                start(priority);
            }
        };

    public:
		ThreadTaskRunner(IExecutor *observer = 0) :
			m_threads(true, 0, true),
			m_observer(observer)
		{}

        // конструктор Aplication должен быть Aplication(shared_ptr<const Config> params)
        template <class Aplication, class Config>
        void Add(const boost::shared_ptr<Config> cfg, Platform::Thread::Priority priority = Platform::Thread::LowPriority)
        {
			Runner<Aplication, Config> *runner = 0;
			int i = m_threads.Find(runner);
			
			runner = new Runner<Aplication, Config>(cfg, priority);
			if (i != -1)	m_threads.Set(i, runner);
			else m_threads.Add(runner);
        }

		int Process()
		{
			if(m_observer) m_observer->Execute();

			int activeCount = 0;
			for(int i = 0; i < m_threads.Size(); ++i)
			{
				if (m_threads[i] && !m_threads[i]->isFinished()) 
				{
					++activeCount;
					continue;
				}
				m_threads.Set(i, 0);
			}
			return activeCount;
		}

        void WaitAll()
        {
			while(true)
			{
				Platform::ThreadSleep(1000);

				int activeCount = Process();

				if (activeCount == 0) break;
			}
        }
	private:
        Utils::ManagedList<Platform::Thread> m_threads;
		IExecutor *m_observer;
    };
};

#endif






