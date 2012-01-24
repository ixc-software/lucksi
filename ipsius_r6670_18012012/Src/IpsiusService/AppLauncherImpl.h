#ifndef APPLAUNCHERIMPL_H
#define APPLAUNCHERIMPL_H

#include "AppLauncher.h"
#include "Utils/ErrorsSubsystem.h"
#include "AppLauncher.h"
#include "Utils/AtomicTypes.h"
#include "Utils/ThreadSyncEvent.h"
#include "Utils/ThreadNames.h"

namespace IpsiusService
{
	typedef AppLauncher::RunAppFn RunAppFn;     
	typedef AppLauncher::RunInMainThreadFn RunInMainThreadFn;
    

	// Run domain in new QThread
    class AppThreadLauncher : public QThread
    {
        Q_OBJECT;        

        RunAppFn m_fRun;        
        int m_res;

        void run() //override
        {               
            Utils::ThreadNamesLocker locker(QThread::currentThreadId(), "AppThreadLauncher");

            m_res = m_fRun();                                 
            quit(); // emit finished() when quit complete            
        }

    public:
        AppThreadLauncher(const RunAppFn& fRun): m_fRun(fRun), m_res(0)
        {            
            start();
        }        

        int getExitCode() 
        {                 
            ESS_ASSERT(isFinished());
            return m_res;
        }
    };

    // ------------------------------------------------------------------------------------

    class AppLauncherImpl : public QObject
    {
        Q_OBJECT;            

        boost::scoped_ptr<AppThreadLauncher> m_runer;
        const RunAppFn& m_fRun;

        Utils::AtomicBool m_guiStarted;
        Platform::Thread::ThreadID m_guiThreadId;
		
    private slots:
        void OnRunDomain()
        {
            m_runer.reset(new AppThreadLauncher(m_fRun));
            // finished - QThread Signal
            connect(m_runer.get(), SIGNAL(finished()), SLOT(OnFinish()), Qt::QueuedConnection); 
        }

        void OnFinish()
        {
            qApp->exit(0);
        }

		void OnRunGui(RunInMainThreadFn fRunGui)
		{			
            m_guiThreadId = Platform::Thread::GetCurrentThreadID();
            m_guiStarted.Set(true);

			fRunGui();
		}

        static void RunAndUlock(RunInMainThreadFn fRunGui,  boost::shared_ptr<Utils::ThreadSyncEvent> lock)
        {
            fRunGui();
            lock->Ready();
        }

	signals:        
		void RunGuiSig(RunInMainThreadFn fRunGui);

    public:           
        int RunApp(int argc, char *argv[]);

		void AsyncRunGui(const RunInMainThreadFn& fRunGui)
		{			
			ESS_ASSERT(!fRunGui.empty());
			emit RunGuiSig(fRunGui);            
		}

        void SyncRunGui(const RunInMainThreadFn& fRunGui)
        {
            ESS_ASSERT(!IsGuiThreadContext());
            boost::shared_ptr<Utils::ThreadSyncEvent> lock( new Utils::ThreadSyncEvent() );            
            AsyncRunGui(boost::bind(&RunAndUlock, fRunGui, lock));
            lock->Wait();
        }

        bool IsGuiThreadContext()
        {
            if (!m_guiStarted.Get()) return false;
            return m_guiThreadId == Platform::Thread::GetCurrentThreadID();
        }

        AppLauncherImpl(const RunAppFn& fRun) : m_fRun(fRun), m_guiStarted(false)
        {
			ESS_ASSERT(!fRun.empty());            

 			qRegisterMetaType<RunInMainThreadFn>("RunInMainThreadFn");
 			ESS_ASSERT(
                 connect(this, SIGNAL(RunGuiSig(RunInMainThreadFn)), SLOT(OnRunGui(RunInMainThreadFn)), Qt::QueuedConnection)
                 );
		}

    };

} // namespace IpsiusService

#endif
