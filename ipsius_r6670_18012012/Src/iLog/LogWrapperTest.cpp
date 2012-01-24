#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/ExeName.h"
#include "Utils/StringUtils.h"
#include "Utils/TimerTicks.h"
#include "iLog/LogWrapperLibrary.h"
#include "iLog/LogWrapperLoggers.h"
#include "LogWrapper.h"


// --------------------------------------

namespace
{
    using namespace iLogW;
    using Platform::dword;
    using iLogCommon::LogStringConvert;
    using std::string;
    using iLogCommon::LogString;

    LogString ToLogString(const std::string &s)
    {
        return iLogCommon::LogStringConvert::To(s);
    }
    

    void SimpleTest(bool useCout)
    {
        LogStore store;

        {
            LogOutput output;
            
            boost::shared_ptr<DefaultFormater> formater( new DefaultFormater() );
            LogCoutStream coutStream(formater);

            // or just -- "LogCoutStream coutStream;"
                        
            if (useCout)
            {
                output.Register(&coutStream);
            }

            LogSessionProfile profile(true, &output);

            LogSession session(store.getSessionInterface(), ToLogString("Name"), profile);
            LogRecordTag tagSpecial = session.RegisterRecordKind( ToLogString("special") );

            if (session.LogActive())
            {
                LoggerTextStream wr(session);
                wr << "x = " << 15;
            }

            if (session.LogActive())
            {
                LoggerTextStream wr(session, tagSpecial);
                wr << "y = " << 16;
            }

            if (session.LogActive())
            {
                session << tagSpecial << "z = " << 17 << EndRecord;
            }

        }

        // save
        std::string fileName = Utils::ExeName::GetExeDir();
        fileName += "WrapperLogTest.txt";

        LogFileStream fs(fileName);
        store.Save(fs);
    }

    // ----------------------------------------------------

    void SimpleTest2(bool useCout)
    {
        using namespace std;

        LogStore store;

        {
            LogSessionProfile profile;
            LogSession session(store.getSessionInterface(), ToLogString("Name"), profile);
            LogRecordTag tagSpecial = session.RegisterRecordKind( ToLogString("special") );

            if (session.LogActive())
            {
                LoggerTextStream wr(session);
                wr << "x = " << 15;
            }

        }

        {
            LogSessionProfile profile;
            LogSession session(store.getSessionInterface(), ToLogString("Name"), profile);

            if (session.LogActive())
            {
                LoggerTextStream wr(session);
                wr << "x = " << 15;
            }

        }

        iLog::LogStat stat = store.GetStat(true);
        if (useCout) cout << stat.ToString() << endl;        
    }

    // ----------------------------------------------------

    class SessionBenchmark
    {
        // enum { CDefaultRecordsSet = 32 * 1024, };
        enum { CDefaultAddRecordsQuantum = 512 };

        bool m_timeCapture;
        bool m_delayedSync;
        bool m_directAddMode;
        int  m_timeToRun;

        int m_totalRecords;
        dword m_timeResult;

        void ClearResult()
        {
            m_totalRecords = 0;
            m_timeResult = 0;
        }

        void DirectAddRecords(LogSession &session, LogRecordTag tagSpecial, int count)
        {
            LogString record = ToLogString("Benchmark...");

            for(int i = 0; i < count; ++i)
            {
                session.Add(record, tagSpecial);
            }
        }

        void SlowAddRecords(LogSession &session, LogRecordTag tagSpecial, int count)
        {
                      
            for(int i = 0; i < count; ++i)
            {
                if (session.LogActive())
                {
                    LoggerTextStream wr(session, tagSpecial);
                    wr.out() << "loop at " << i;
                }
            }
        }


    public:
        SessionBenchmark(bool timeCapture, bool delayedSync, bool directAddMode, int  timeToRun) : 
          m_timeCapture(timeCapture), 
          m_delayedSync(delayedSync),
          m_directAddMode(directAddMode),
          m_timeToRun(timeToRun)
        {
            ClearResult();
        }

        void Run()
        {
            LogStore store; 

            LogOutput output;
            int syncCounter = (m_delayedSync) ? -1 : 1;
            SessionSyncStrategy sync(syncCounter);
            LogSessionProfile profile(m_timeCapture, &output, &sync);

            LogSession session(store.getSessionInterface(), ToLogString("Name"), profile);

            LogRecordTag tagSpecial = session.RegisterRecordKind( ToLogString("special") );

            // loop
            ClearResult();

            Utils::TimerTicks timer;

            while (timer.Get() < m_timeToRun)
            {
                if (m_directAddMode) DirectAddRecords(session, tagSpecial, CDefaultAddRecordsQuantum);
                               else  SlowAddRecords(session, tagSpecial, CDefaultAddRecordsQuantum);

                m_totalRecords += CDefaultAddRecordsQuantum;
            }

            m_timeResult = timer.Get();           
        }

        string Result(bool fullInfo)
        {
            std::ostringstream ss;
            ss << "Benchmark";

            if (fullInfo)
            {
                std::ostringstream info;

                if (m_timeCapture) info << " TimeCapture";
                              else info << " NoTimeCapture";

                if (m_delayedSync) info << " DelayedSync";
                              else info << " SyncOnEveryRec";

                if (m_directAddMode) info << " DirectAdd";
                                else info << " StreamAdd";

                info << " TimeToProcess = " << m_timeResult;
                
                ss << " (" << info.str() << ") ";
            }

            int recordsPerSec = (m_totalRecords * 1000) / m_timeResult;

            ss << recordsPerSec;

            return ss.str();
        }

    };

    LogString RunBenchmark(bool timeCapture, bool delayedSync, bool directMode, bool printIt, 
        int  timeToRun = 1000)
    {
        SessionBenchmark b(timeCapture, delayedSync, directMode, timeToRun);
        b.Run();

        string res = b.Result(true);
        if (printIt) std::cout << res << std::endl;

        return ToLogString(res);
    }

    /*
        На Athlon 4000+ результаты логирования в отдладке, для самого 
        быстрого режима -- ~33k записей в секунду.
        Захват даты работает на 10% медленнее. 
        Постоянная синхронизация с LogStore еще на 10% медленнее (~27k).

        В релизе самый медленный вариант дает 65-67k.

        ------------

          После адаптации под wstring скорость несколько упала, почему-то сильно
        просаживает захват даты.

        Debug

        Benchmark ( NoTimeCapture DelayedSync    TimeToProcess = 937)  34971
        Benchmark ( TimeCapture   DelayedSync    TimeToProcess = 1265) 25903
        Benchmark ( TimeCapture   SyncOnEveryRec TimeToProcess = 1438) 22787

        Release

        Benchmark ( NoTimeCapture DelayedSync    TimeToProcess = 469) 69867
        Benchmark ( TimeCapture   DelayedSync    TimeToProcess = 625) 52428
        Benchmark ( TimeCapture   SyncOnEveryRec TimeToProcess = 640) 51200


        ----------

        Blackfin (debug, 600 MHz, I-cache, no data cache)

        Benchmark ( NoTimeCapture DelayedSync DirectAdd    TimeToProcess = 1032) 9426
        Benchmark ( TimeCapture   DelayedSync DirectAdd    TimeToProcess = 1020) 7027
        Benchmark ( NoTimeCapture DelayedSync StreamAdd    TimeToProcess = 1175) 2614
        Benchmark ( TimeCapture   DelayedSync StreamAdd    TimeToProcess = 1117) 2291
        Benchmark ( TimeCapture   SyncOnEveryRec StreamAdd TimeToProcess = 1005) 2037


    */

    void RunBenchmarks(bool useCout)
    {
        std::string fileName = Utils::ExeName::GetExeDir();
        fileName += "LogBenchmark.txt";
        LogFileStream fs(fileName);

        // 1. quick direct mode
        fs.Write( RunBenchmark(false, true, true, useCout) );
        fs.Write( RunBenchmark(true,  true, true, useCout) );

        // 2. slow mode
        // a. delayed sync
        fs.Write( RunBenchmark(false, true, false, useCout) );
        fs.Write( RunBenchmark(true, true,  false, useCout) );

        // b. direct sync
        fs.Write( RunBenchmark(true, false, false, useCout) );
    }

    void RunBenchmarkTest()
    {
        RunBenchmark(true, true,  false, false, 100);
    }

}

// --------------------------------------

namespace iLog
{

    /*
    void iLogTestWrap()
    {
        Test();

        RunBenchmarks();
    } */

    void iLogWrapperSimpleTest(bool useCout)
    {
        SimpleTest(useCout);
        SimpleTest2(useCout);
    }

    void iLogWrapperBenchmark(bool useCout)
    {
        RunBenchmarks(useCout);
    }

    void iLogWrapperBenchmarkAsTest()
    {
        RunBenchmarkTest();
    }


}
