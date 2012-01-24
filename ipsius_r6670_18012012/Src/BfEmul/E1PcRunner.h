#ifndef __E1PCRUNNER__
#define __E1PCRUNNER__

#include "Platform/Platform.h"
#include "BfDev/SysProperties.h"
#include "Utils/IntToString.h"

#include "E1AppConfig.h"
#include "E1PcEmul.h"
#include "E1AppRunner.h"

namespace E1App
{
    using Utils::ManagedList;

    
    class E1PcRunner : public IAppExit
    {
        struct Pair
        {
            int CmpPort0; 
            int CmpPort1;

            Pair()
            {
                CmpPort0 = -1;
                CmpPort1 = -1;
            }

            Pair(int cmpPort0, int cmpPort1)
            {
                ESS_ASSERT(cmpPort0 > 0);
                ESS_ASSERT(cmpPort1 > 0);
                ESS_ASSERT(cmpPort0 != cmpPort1);

                CmpPort0 = cmpPort0;
                CmpPort1 = cmpPort1;
            }
        };

        // ------------------------------------

        class AppInstance : public Platform::Thread
        {
            E1AppConfig m_cfg;

            void run()  // override
            {
				RunE1Application(m_cfg);
            }

        public:

            AppInstance(const E1AppConfig &cfg)
            {
                m_cfg = cfg;
                start(LowPriority);
            }
        };

        // ------------------------------------

        class ThreadList
        {
            ManagedList<AppInstance> m_threads;

        public:

            void Add(AppInstance *pThread)
            {
                ESS_ASSERT(pThread != 0);

                m_threads.Add(pThread);
            }

            void WaitAll()
            {
                while(true)
                {
                    Platform::ThreadSleep(1000);

                    {
                        int doneCount = 0;
                        for(int i = 0; i < m_threads.Size(); ++i)
                        {
                            if (m_threads[i]->isFinished()) doneCount++;
                        }

                        if (doneCount == m_threads.Size()) break;
                    }
                }
            }

        };

        // ------------------------------------

        E1AppConfig m_cfg;
        std::vector<Pair> m_pairs;
        bool m_exitReq;

        static std::string AddThread(ThreadList &list, E1AppConfig &cfg, int cmpPort)
        {
            std::string name;
            name += "App";
            name += Utils::IntToString(cmpPort);

            cfg.AppName = name;
            cfg.CmpProtoServicePort = cmpPort;

            list.Add( new AppInstance(cfg) );
            return name;
        }

    // IAppExit impl
    private:

        bool AppExitRequest()
        {
            const int CExitKey = 27; // 'Esc'

            if (!m_exitReq)
            {
                m_exitReq = (Platform::KeyWasPressed() == CExitKey);
            }

            return m_exitReq;
        }


    public:

        E1PcRunner(const E1AppConfig &cfg) 
        {
            m_cfg = cfg;
            m_exitReq = false;
        }

        void AddPair(int cmpPort0, int cmpPort1)
        {
            // TODO -- dublicates check

            m_pairs.push_back( Pair(cmpPort0, cmpPort1) );
        }

        void Run()
        {
            ESS_ASSERT(m_pairs.size() > 0);

            BfDev::SysProperties::InitFromProjectOptions();

            PcEmul::Init();

            E1AppConfig config = m_cfg;
			
			//config.SetAppExit(this);
            config.MonitorCpuUsage = false;
            config.NetworkConfig.Set("0.0.0.0", "192.168.0.35");  // ?
            config.ThreadRunnerSleepInterval = 200;

            E1App::Stack::Init(config.NetworkConfig);

            {
                ThreadList threads;

                for(int i = 0; i < m_pairs.size(); ++i)
                {
                    std::string name0 = AddThread(threads, config, m_pairs.at(i).CmpPort0);
                    std::string name1 = AddThread(threads, config, m_pairs.at(i).CmpPort1);

                    PcEmul::Instance().AddPair( BoardPair(name0, name1) );
                }

                threads.WaitAll();

            }

        }

        /*
        void Run()
        {
            BfDev::SysProperties::InitFromProjectOptions();

            E1App::PcEmul::Init( E1App::BoardPair("B1", "B2") );

            E1App::E1AppConfig config;
            config.MonitorCpuUsage = false;
            config.CmpProtoServicePort = 44045;
            config.NetworkConfig.Set("0.0.0.0", "192.168.0.35");  // ?

            E1App::Stack::Init(config.NetworkConfig);

            E1App::RunApplicationThread(config);
        } */


    };
    
    
}  // namespace E1App

#endif
