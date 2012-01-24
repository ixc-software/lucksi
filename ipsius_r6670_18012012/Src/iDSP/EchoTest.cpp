#include "stdafx.h"

#include "Platform/Platform.h"
#include "iDSP/g711gen.h"
#include "Utils/RawCRC32.h"

#include "EchoCancel.h"
#include "EchoTest.h"

// -------------------------------------------------

namespace
{
    using namespace iDSP;

    struct RunCfg
    {
        vector<int> TxData;
        EchoTestCfg Config;

        RunCfg()
        {
            // make input
            iDSP::LineMultiFreqGenerator gen;
            gen.Add(433, 0);
            gen.Add(1213, 0);

            const int CSamplesPerSec = 8000;
            gen.Fill(TxData, 10 * CSamplesPerSec);

            // config
            Config.TapsSize = 128;
            Config.EchoShift = 64;
            Config.EchoPercent = 90;
            Config.SaveResult = false;
            Config.PartialQualityDiv = 0;
            Config.InputDataCRC = false;
        }
    };

    void RunGeneratorTest(int freq, int power)
    {
        using namespace std;
        const int CSize = 1024;

        iDSP::LineFreqGenerator gen(freq, power);
        cout << gen.ToString(8) << endl;

        std::vector<int> data;
        for (int i = 0; i < CSize; ++i) data.push_back( gen.Next() );

        Platform::dword crc = Utils::UpdateCRC32(data);
        cout << hex << crc << dec << endl << endl;
    }

}  // namespace

// -------------------------------------------------

namespace iDSP
{
    
    void EchoTest::RunReferenceTest()
    {
        RunCfg cfg;

        // data CRC
        Platform::dword txCRC = Utils::UpdateCRC32(cfg.TxData);
        TUT_ASSERT(txCRC == 0x09feab31);

        // run
        EchoTest test;
        test.Run(cfg.TxData, cfg.Config);

        // assert CRC vs magic number
        TUT_ASSERT(test.CRC32() == 0x5303ae75);
    }

    // ----------------------------------------------------------

    void EchoTest::Run( int taps, int echoDelay, int echoPercent)
    {
        RunCfg cfg;

        cfg.Config.TapsSize = taps;
        cfg.Config.EchoShift = echoDelay;
        cfg.Config.EchoPercent = echoPercent;
        cfg.Config.SaveResult = true;
        cfg.Config.PartialQualityDiv = 16;
        cfg.Config.InputDataCRC = true;

        // run
        EchoTest test;
        test.Run(cfg.TxData, cfg.Config);

        // print
        std::cout << test.Result() << std::endl;
    }

    // ------------------------------------------------------------

    /*
    void EchoTest::GeneratorTest()
    {
        using namespace std;
        const int CTotalData = 32;

        RunCfg cfg;

        cout << "Gx data: ";
        for(int i = 0; i < CTotalData; ++i)
        {
            cout << cfg.TxData.at(i) << " ";
        }
        cout << endl;

        cout << "Gx data: ";
        int size = cfg.TxData.size();
        for(int i = size - CTotalData; i < size; ++i)
        {
            cout << cfg.TxData.at(i) << " ";
        }
        cout << endl;

        // CRC32
        Platform::dword crc = Utils::UpdateCRC32(&cfg.TxData[0], cfg.TxData.size());
        cout << "CRC " << hex << crc << dec << endl;

        cout << endl;
    } */

    void EchoTest::GeneratorTest()
    {
        RunGeneratorTest(433, 0);
        RunGeneratorTest(1213, 0);
        std::cout << std::endl;
    }

    // ------------------------------------------------------------

    void EchoTest::Run(const vector<int> &dataTx, const EchoTestCfg &cfg)
    {
        m_config = cfg;

        std::vector<int> cleanRx;
        cleanRx.reserve( dataTx.size() );

        Platform::dword inputCRC = 0xffffffff;

        // echo loop
        EchoCancel echo( cfg.TapsSize);
        int beginT = Platform::GetSystemTickCount();

        for(int i = 0; i < dataTx.size(); ++i)
        {
            int tx = dataTx.at(i);

            int rx = 0;
            int rxIndx = i - cfg.EchoShift;
            if ((rxIndx) >= 0 && (rxIndx < dataTx.size()))
            {
                rx = (dataTx.at(rxIndx) * cfg.EchoPercent) / 100;
            }

            if (cfg.InputDataCRC)
            {
                inputCRC = Utils::UpdateCRC32(&rx, sizeof(rx), inputCRC);
                inputCRC = Utils::UpdateCRC32(&tx, sizeof(tx), inputCRC);
            }

            int res = echo.Process(tx, rx);

            cleanRx.push_back( res );
        }

        int runTime = Platform::GetSystemTickCount() - beginT;

        // fix benchmark fields
        m_totalSamples = dataTx.size();
        m_totalTimeMs = runTime;

        // CRC
        m_resultCRC = Utils::UpdateCRC32(cleanRx);
        m_inputCRC = inputCRC;

        // Quality
        SetQuality(cleanRx, cfg.PartialQualityDiv);

        // fix result
        m_cleanRx.reset();

        if (cfg.SaveResult) 
        {
            m_cleanRx.reset( new vector<int>() );
            m_cleanRx->swap( cleanRx );
        }
    }

    // ----------------------------------------------------------

    double EchoTest::CaclQuality( const vector<int> &signal, int begin, int count )
    {
        double res = 0;

        for(int i = begin; i < begin + count; ++i)
        {
            double sample = signal.at(i);
            res += (sample * sample);
        }

        return std::sqrt(res / signal.size()); // / 1000;
    }

    // ----------------------------------------------------------

    void EchoTest::SetQuality(const vector<int> &signal, int partialQualityDiv)
    {
        // clear
        m_resultQuality = -1;
        m_partialQuality.clear();

        if (partialQualityDiv <= 0) return;
        
        // partial
        double sum = 0;
        int partCount = signal.size() / partialQualityDiv;

        for(int i = 0; i < partialQualityDiv; ++i)
        {
            double val = CaclQuality(signal, i * partCount, partCount);
            sum += val;
            m_partialQuality.push_back(val);
        }

        m_resultQuality = sum;
    }

    // ----------------------------------------------------------

    std::string EchoTest::Result() const
    {
        using namespace std;
        ostringstream ss;

        ss << "Echo test results" << endl;

        // info
        ss << "Taps " << m_config.TapsSize << "; echo delay " << m_config.EchoShift
            << "; echo amp " << m_config.EchoPercent << "%" << endl;

        // benchmark
        int samplesPerSec = (m_totalSamples * 1000) / m_totalTimeMs;
        ss << "Samples/sec " << samplesPerSec << "; channels " << (samplesPerSec / 8000) << endl;

        // quality and CRC
        ss << fixed << setprecision(1);
        ss << "Avg quality " << m_resultQuality << "; Result CRC " << hex << m_resultCRC 
            << "; input CRC " << m_inputCRC << dec << endl;

        ss << "Partial: { ";
        for(int i = 0; i < m_partialQuality.size(); ++i)
        {
            double val = m_partialQuality.at(i);
            ss << val << " ";
        }
        ss << "}" << endl;

        return ss.str();
    }

}  // namespace iDSP


