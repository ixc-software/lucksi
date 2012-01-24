#include "stdafx.h"

#include "Platform/Platform.h"
#include "EchoCancel.h"
#include "Utils/Random.h"

#include "EchoTest.h"
#include "FirClass.h"

using Platform::int16;
using Platform::dword;

// -----------------------------------------

namespace
{
	using Platform::int16;
	using namespace iDSP;
	
    template<class TElem>
    void RndFill(std::vector<TElem> &v, int size, int rndSeed, int rndRange)
    {
        Utils::Random rnd(rndSeed);

        v.clear();
        v.reserve(size);

        for(int i = 0; i < size; ++i)
        {
            int val = rnd.Next(rndRange) - (rndRange / 2);
            v.push_back(val);
        }

    }

	// --------------------------------------------------
	
	void EchoTestBench(int taps)
	{
		using namespace std;
	
		const int CSamplesCount = 8000;
		const int CEchoShift = 20;
	
		vector<int16> tx, rx;
		
		RndFill(tx, CSamplesCount, 1, 8*1024);
		
		for(int i = 0; i < CSamplesCount; ++i)
		{
			rx.push_back( tx.at(i) / 4 );
		}
	
		iDSP::EchoCancel echo(taps);
		dword crc = 0;
		
	    int beginT = Platform::GetSystemTickCount();  // begin bench

	    for(int i = 0; i < tx.size(); ++i)
	    {
	        int res = echo.Process(tx[i], rx[i]);
	        crc += (dword)res;
	    }

	    int runTime = Platform::GetSystemTickCount() - beginT;  // end bench
	    int samplesPerSec = (tx.size() * 1000) / runTime;

	    // Samples per sec 173913; CRC fffca3fc
	    cout << "Samples per sec " << samplesPerSec 
	         << "; time (ms) " << runTime
	         << "; taps " << taps
	         << "; Ch " << (samplesPerSec / 8000) 
	    	 << "; CRC "<< hex << crc << endl;		
        cout << "Adopt called " << dec << echo.BgAdoptCalled() << endl;
	}	

    // --------------------------------------------------
	
	void FirTest()
    {
    	const int CTaps = 128;
    	const int CDataSize = 1024;
    	
    	// make coef
    	std::vector<int> coef;
        RndFill(coef, CTaps, 1, 1024);
    	    	    	
    	// make data-in
    	std::vector<int16> data;
        RndFill(data, CDataSize, 13, 8*1024);

        // data-out
        std::vector<int16> dataOutFir;
        dataOutFir.resize( data.size() );

        std::vector<int16> dataOutFirBf;
        dataOutFirBf.resize( data.size() );
    	        	    	
    	// process
        {
            FirClass fir(coef);
            fir.DoBlock(&data[0], &dataOutFir[0], data.size());

            BfFirClass firBf(coef);
            firBf.DoBlock(&data[0], &dataOutFirBf[0], data.size());
        }
    	
    	int errCount = 0;
    	
    	for(int i = 0; i < CDataSize; ++i)
        {
        	int res0 = dataOutFir.at(i);
        	int res1 = dataOutFirBf.at(i);
        	        	
        	int diff = std::abs(res0 - res1);
        	
        	if(diff > 1)
        	{
        		errCount++;
        		
        		if (errCount > 32)
        	    {
					std::cout << i << " " << res0 << " " << res1 << " " 
					          << (res0 - res1) << std::endl;        	    	
					
					errCount = 0;
        	    }
        		        		
        		ESS_ASSERT(diff <= 1);
        	}
        	
        }
        
        std::cout << "FIR OK" << std::endl;
    	
    }
    
  	// --------------------------------------------------
  	
  	template<class TFir, bool TUseBlock>
  	void FirBench(int taps, int dataSize, int loops)
  	{
        // make coef
        std::vector<int> coef;
        RndFill(coef, taps, 1, 1024);

        // make data-in
        std::vector<int16> data;
        RndFill(data, dataSize, 13, 8*1024);
        
        // data-out
        std::vector<int16> dataOut;
        dataOut.resize( data.size() );
        
        // begin bench
        int beginT = Platform::GetSystemTickCount();  

        // process
        TFir fir(coef);

        for(int i = 0; i < loops; ++i)
        {
        	if (TUseBlock)
        	{
        		fir.DoBlock(&data[0], &dataOut[0], data.size());
        	}
        	else
        	{
	            int count = dataSize;
	            int16 *pData = &data[0];

	            while(count--)
	            {
	                int res = fir.Do(*pData);
	                pData++;
	            }        		
        	}
        }

        // end of bench
        int runTime = Platform::GetSystemTickCount() - beginT;

        // result
        int firPerSec = ((dataSize * loops) * 1000) / runTime;

        std::cout << "FIR " << taps << " " << typeid(TFir).name();
        if (TUseBlock) std::cout << " block " << dataSize;
        std::cout << " K/sec " << (firPerSec/1024) << std::endl;  		
  	}

  	/*  	
  	void FirBenchmark()
  	{
        const int CTaps = 128;
        const int CDataSize = 1024;
        const int CLoops = 128;

        // FirBench<Utils::FirClass,   false>(CTaps, CDataSize, CLoops);
        // FirBench<Utils::BfFirClass, false>(CTaps, CDataSize, CLoops);
        
        FirBench<Utils::BfFirClass, true>(CTaps, CDataSize, CLoops);        
        FirBench<Utils::FirClass,   true>(CTaps, CDataSize, CLoops);  		
  	} */
  	
  	void FirBenchmark()
  	{
        const int CTaps = 128;
        const int CDataSize = 2; // 1024;
        const int CLoops = 128 * 512;

        // FirBench<Utils::FirClass,   false>(CTaps, CDataSize, CLoops);
        // FirBench<Utils::BfFirClass, false>(CTaps, CDataSize, CLoops);
        
        FirBench<BfFirClass, true>(CTaps, CDataSize, CLoops);        
        FirBench<FirClass,   true>(CTaps, CDataSize, CLoops);  		
  	}
  	

	
}  // namespace



// -----------------------------------------


/*

                 20 ch = no WB cache
	             27 ch = 432  FIR / 1641 (25%)
	             37 ch = 592  FIR        (36%)  adopt ~55%
	adopt off -- 83 ch = 1328 FIR        (80%)

	Freq: 600 MHz
	Samples per sec 222222; Ch 27; CRC b8
	Samples per sec 296296; Ch 37; CRC b8
	Adopt called 8000
	FIR OK
	FIR 128 Utils::BfFirClass block  K/sec 4413
	FIR 128 Utils::FirClass block  K/sec 1641
	
	----
	
	8.06.10
	
	600 MHz, WT cache: 
       Samples per sec 250000; time (ms) 32; taps 128; Ch 31; CRC b8
	   Samples per sec 307692; time (ms) 26; taps 64;  Ch 38; CRC ffffffd6	
	   Samples per sec 400000; time (ms) 20; taps 32;  Ch 50; CRC 47e
	   
	600 MHz, WB cache: 
	    Samples per sec 285714; time (ms) 28; taps 128; Ch 35; CRC b8	
	    Samples per sec 380952; time (ms) 21; taps 64;  Ch 47; CRC ffffffd6	
	   

*/


namespace iDSP
{

    void RunEchoTestBf()
    {
    	EchoTestBench(128);	
    	EchoTestBench(64);	
    	EchoTestBench(32);	
    	    
    	// FirTest();
    		
    	// FirBenchmark();
            		
    }
    
}  // namespace Echo

