#ifndef __ECHOTEST__
#define __ECHOTEST__

#include "Platform/Platform.h"

namespace iDSP
{
    using std::vector;
    using boost::shared_ptr;

    struct EchoTestCfg
    {
        // echo 
        int TapsSize;
        int EchoShift, EchoPercent;

        // misc
        bool SaveResult;
        int  PartialQualityDiv;  // кол-во участков, на которые будет разбит выходной сигнал для раздельной оценки качества
        bool InputDataCRC;

        EchoTestCfg()
        {
            TapsSize = 128;
            EchoShift = 64;
            EchoPercent = 90;

            SaveResult = false;
            PartialQualityDiv = 16;
            InputDataCRC = false;
        }
    };

    // -----------------------------------------
    
    class EchoTest
    {
        // config for result
        EchoTestCfg m_config;

        // result
        shared_ptr< vector<int> > m_cleanRx;  
        Platform::dword m_resultCRC, m_inputCRC;
        double m_resultQuality;
        vector<double> m_partialQuality;

        // benchmark
        int m_totalSamples;
        int m_totalTimeMs;

        static double CaclQuality(const vector<int> &signal, int begin, int count);

        void SetQuality(const vector<int> &signal, int partialQualityDiv);

    public:

        EchoTest() : m_resultCRC(0), m_inputCRC(0), m_resultQuality(0), m_totalSamples(0), m_totalTimeMs(0) 
        {
        }

        void Run(const vector<int> &tx, const EchoTestCfg &cfg);

        std::string Result() const;
        Platform::dword CRC32() const { return m_resultCRC; }

        static void RunReferenceTest();
        static void Run(int taps, int echoDelay, int echoPercent);
        static void GeneratorTest();

    };
    
}  // namespace iDSP

#endif

