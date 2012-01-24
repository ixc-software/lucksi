#include "stdafx.h"

#include "Platform/Platform.h"
#include "Utils/Random.h"
#include "Utils/RawCRC32.h"
#include "Utils/IntToString.h"

#include "LowBench.h"
#include "LowTransformT.h"

using namespace EchoApp;
namespace mpl = boost::mpl; 

using Platform::byte;
using Platform::int16;
using Platform::dword;
using Platform::ddword;

// ----------------------------------------------

namespace
{

    template<class TLow, class TIn>
    void FillDataIn(std::vector<TIn> &in, int seed, int blockSize)
    {
        in.clear();
        in.reserve(blockSize);

        Utils::Random rnd(seed);

        for(int i = 0; i < blockSize; ++i)
        {
            // compress vs expand
            int val = TLow::CExpand ? rnd.NextByte() : TLow::NextLinear(rnd); 
            in.push_back(val);
        }
    }

    void PrintResult(const std::string &desc, 
        dword t, ddword totalSamples, dword outCrc, bool blockConvert)
    {     
        if (t == 0) ++t;

        int speed = (totalSamples * 1000) / t;  
        std::cout << desc 
            << (blockConvert ? " block " : " single")
            << "; Out CRC " << Utils::IntToHexString(outCrc) 
            << "; Ksmpl/sec " << (speed / 1000)
            << "; time (ms) " << t;

        /*
        Platform::dword tableCRC;
        if (TLow::TableCRC(tableCRC))
        {
            std::cout << "; table " << Utils::IntToHexString(tableCRC);
        } */

        std::cout << std::endl;
    }


    template<class TLow>
    class LowBench
    {
        typedef typename TLow::TIn TIn;

    public:

        static void Run(int seed, int blockSize, int loopCount, bool blockConvert)
        {
            // make data
            std::vector<TIn> in;
            FillDataIn<TLow>(in, seed, blockSize);

            TLow::Init();  // dummi init inner table

            // data for output
            std::vector<TLow::TOut> out(blockSize, 0);

            // convert
            dword t = Platform::GetSystemTickCount();

            ESS_ASSERT((in.size() == out.size()) && (in.size() == blockSize));
            int count = loopCount;

            while(count--)
            {
                if (blockConvert)
                {
                    TLow::Process(&in[0], &out[0], blockSize);
                }
                else
                {
                    for(int i = 0; i < in.size(); ++i)
                    {
                        out[i] = TLow::Process(in[i]);
                    }
                }
            }

            t = Platform::GetSystemTickCount() - t;

            dword outCrc = Utils::UpdateCRC32(out);

            // result
            PrintResult(TLow::Name(), t, blockSize * loopCount, outCrc, blockConvert);
        }

        static void RunAll(int seed, int blockSize, int loopCount)
        {
            Run(seed, blockSize, loopCount, true);
            Run(seed, blockSize, loopCount, false);
        }


    };

    template<class TLow>
    void Run(int seed, int blockSize, int loopCount)
    {
        // expand
        LowBench< LowTemplateMath<true,  TLow> >::RunAll(seed, blockSize, loopCount);

        // expand-table
        LowBench< LowTemplateTable<true,  TLow> >::RunAll(seed, blockSize, loopCount);

        std::cout << std::endl;

        // compress
        LowBench< LowTemplateMath<false, TLow> >::RunAll(seed, blockSize, loopCount);

        // compress-table
        LowBench< LowTemplateTable<false, TLow> >::RunAll(seed, blockSize, loopCount);

        std::cout << std::endl;
    }


}  // namespace

// --------------------------------------------------------------------

namespace
{

    template<class TLowExp, class TLowComp>
    class EchoCodingBench
    {

        BOOST_STATIC_ASSERT(TLowExp::CExpand);
        BOOST_STATIC_ASSERT(!TLowComp::CExpand);

    public:

        static void Run(int seed, int blockSize, int loopCount, bool blockConvert)
        {
            TLowExp::Init();
            TLowComp::Init();

            // make data-in: RX + TX
            std::vector<byte> rxIn;
            FillDataIn<TLowExp>(rxIn, seed, blockSize);
            std::vector<int16> rxInExp(blockSize, 0);

            std::vector<byte> txIn;
            FillDataIn<TLowExp>(txIn, seed + 1, blockSize);
            std::vector<int16> txInExp(blockSize, 0);

            // data for output
            std::vector<int16> out(blockSize, 0);

            // check
            ESS_ASSERT((rxIn.size() == rxInExp.size()));
            ESS_ASSERT((txIn.size() == txInExp.size()));
            ESS_ASSERT((rxIn.size() == out.size()) && (out.size() == blockSize));

            // process
            dword t = Platform::GetSystemTickCount();

            int count = loopCount;
            while(count--)
            {
                if (blockConvert)
                {
                    TLowExp::Process(&rxIn[0], &rxInExp[0], blockSize);  // expand rx
                    TLowExp::Process(&txIn[0], &txInExp[0], blockSize);  // expand tx

                    for(int i = 0; i < out.size(); ++i)
                    {
                        out[i] = TLowComp::Process(rxInExp[i] + txInExp[i]);
                    }

                }
                else
                {
                    for(int i = 0; i < out.size(); ++i)
                    {
                        int16 rx = TLowExp::Process(rxIn[i]);
                        int16 tx = TLowExp::Process(txIn[i]);
                        out[i] = TLowComp::Process(rx + tx);
                    }
                }
            }

            t = Platform::GetSystemTickCount() - t;

            dword outCrc = Utils::UpdateCRC32(out);

            // result
            std::string desc = TLowExp::Name() + " -> " + TLowComp::Name() + ";";
            PrintResult(desc, t, blockSize * loopCount, outCrc, blockConvert);
        }

        static void RunAll(int seed, int blockSize, int loopCount)
        {
            Run(seed, blockSize, loopCount, true);
            Run(seed, blockSize, loopCount, false);
        }

    };

}   // namespace



// --------------------------------------------------------------------

namespace EchoApp
{
    
    void RunLowBenchmark()
    {
        const int CSeed = 1; 
        const int CBlockSize = 160;
        const int CLoopCount = 32 * 1024;

        Run<ALowBase>(CSeed, CBlockSize, CLoopCount);
        Run<ULowBase>(CSeed, CBlockSize, CLoopCount);

    }

    /*

        Result on real target:

        Freq: 600 MHz, cache
        A-low expand   math  -> A-low compress math ; block ; Out CRC 0x10e63f30; Ksmpl/sec 2601; time (ms) 2015
        A-low expand   math  -> A-low compress math ; single; Out CRC 0x10e63f30; Ksmpl/sec 2699; time (ms) 1942
        A-low expand   table -> A-low compress table; block ; Out CRC 0x10e63f30; Ksmpl/sec 9181; time (ms) 571
        A-low expand   table -> A-low compress table; single; Out CRC 0x10e63f30; Ksmpl/sec 4527; time (ms) 1158
        Done!

        best result: block + table = 9.2 Ms/sec
        need = 30 x 8k = 240 Ks/sec, load ~2.6%

    */

    void RunEchoCodingBench()
    {
        const int CSeed = 1; 
        const int CBlockSize = 160;
        const int CLoopCount = 32 * 1024;

        {
            typedef LowTemplateMath<true,  ALowBase> TExp;
            typedef LowTemplateMath<false,  ALowBase> TComp;

            EchoCodingBench<TExp, TComp>::RunAll(CSeed, CBlockSize, CLoopCount);
        }

        {
            typedef LowTemplateTable<true,  ALowBase> TExp;
            typedef LowTemplateTable<false,  ALowBase> TComp;

            EchoCodingBench<TExp, TComp>::RunAll(CSeed, CBlockSize, CLoopCount);
        }

    }
    
}  // namespace EchoApp

