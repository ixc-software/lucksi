#include "stdafx.h"

#include "Utils/AtomicTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ILockable.h"
#include "Utils/ThreadNames.h"

#include "NetThreadMonitor.h"
#include "MemberSocketThread.h"

namespace
{
    Utils::ILockable InstanceLock;
	
    class SocketThreadImpl : public QThread
    {
    public:
		SocketThreadImpl() : m_monitor(0)
        {
            start(QThread::NormalPriority);
        }
        ~SocketThreadImpl()
        {
            exit();        
            wait();        
        }

		void StartNetThreadMon(int timeout)
		{
			if(m_monitor != 0) m_monitor->Start(timeout);
		}
		void StopNetThreadMon()
		{
			if(m_monitor != 0) m_monitor->Stop();
		}
		void Exit()
		{
			// גûחמג טח net thread
			m_monitor.reset();
            exit();        			
		}

    private:
        
		void run()  // override QThread::run
        {
            Utils::ThreadNamesLocker locker(QThread::currentThreadId(), "SocketThreadImpl");

			m_monitor.reset(new iNet::NetThreadMonitor());
            exec();
			m_monitor.reset();
        }

		boost::scoped_ptr<iNet::NetThreadMonitor> m_monitor;
    };


    class SocketThread : public boost::noncopyable
    {
        SocketThread()
		{
			m_timeout = 0;
		}

    public:
        static SocketThread &Instance()
        {
            static SocketThread G_SocketThread;
            return G_SocketThread;
        }

        // ---------------------------------------------

        ~SocketThread()
        {
            m_thread.reset();

            if (m_useCount.Get()) 
            {
                std::cerr << "Error in ~SocketThread(). Where are " << m_useCount.Get() 
                    << " allocated socks." << std::endl;
            }        
        }

        // ---------------------------------------------

        void TakeObject(QObject &newObject)
        {
            Utils::Locker locker(InstanceLock);

            if(!m_useCount) 
			{
				m_thread.reset(new SocketThreadImpl);
				if(m_timeout != 0) m_thread->StartNetThreadMon(m_timeout);
			}
            m_useCount.Inc();
            newObject.moveToThread(m_thread.get());
        }

        // ---------------------------------------------

        void FreeObject()
        {
            Utils::Locker locker(InstanceLock);
            ESS_ASSERT(m_useCount.Get());

            m_useCount.Dec();

            if(!m_useCount) m_thread->Exit();
        }

		// ---------------------------------------------

		void StartNetThreadMon(int timeout)
		{
            Utils::Locker locker(InstanceLock);
			m_timeout = timeout;
			m_thread->StartNetThreadMon(timeout);
		}

		// ---------------------------------------------

		void StopNetThreadMon()
		{
			Utils::Locker locker(InstanceLock);
			m_timeout = 0;
			m_thread->StopNetThreadMon();

		}
    private:
        Utils::AtomicInt m_useCount;
		int m_timeout;
        boost::scoped_ptr<SocketThreadImpl> m_thread;
    };
};

// ---------------------------------------------

namespace iNet
{
    MemberSocketThread::MemberSocketThread( QObject &newObject )
    {
        SocketThread::Instance().TakeObject(newObject);
    }

    MemberSocketThread::~MemberSocketThread()
    {
        SocketThread::Instance().FreeObject();
    }

	void MemberSocketThread::StartNetThreadMon(int timeout)
	{
		SocketThread::Instance().StartNetThreadMon(timeout);
	}

	void MemberSocketThread::StopNetThreadMon()
	{
		SocketThread::Instance().StopNetThreadMon();
	}
}  // namespace iSocket

