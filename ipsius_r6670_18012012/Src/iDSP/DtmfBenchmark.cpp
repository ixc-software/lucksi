#include "stdafx.h"
#include "DtmfBenchmark.h"

#include "DtmfDetector.h"
#include "DtmfGenerator.h"
#include "TdmMng/TdmCodecs.h"
#include "Utils/TimerTicks.h"


//#include "span_dtmf.h"

namespace 
{
    const int CCycles = 1000; 	
    const std::string CCycleSeq = "*1234567890ABCD#";
    const int CDataChank = 160; //160 == RtpPacketSize

	using namespace iDSP;

    class DtmfSpeedTest
    {
        std::vector<Platform::byte> m_data;          

        DtmfDetector m_detect;
        boost::scoped_ptr<iDSP::ICodec> m_codec;
        
        
        int IncPos(int& pos, int incSize) 
        {
        	//ESS_ASSERT(size <= m_data.size());
            pos += incSize;
            //ESS_ASSERT(m_pos <= m_data.size());
            if (pos != m_data.size()) return 0;
            pos = 0;
            return 1;
        }
    public:
        DtmfSpeedTest(bool useALaw) 
            : m_codec( TdmMng::TdmCodec::CreateCodec(useALaw) )            
        {
            // prepare data for benchmark

            DtmfGenerator gen;
            gen.Add(CCycleSeq);
            
            while(gen.RestSampleCount() != 0)
            {
                m_data.push_back( m_codec->Encode(gen.NextSample()) );
            }
        }
        

        int Run()
        {
            ESS_ASSERT(m_data.size() > CDataChank);

            std::vector<Platform::int16> chankBuff(CDataChank);                                          
            int cycleCount = 0; // dtmf sequence cycle counter
            int pos = 0; // position in m_data

            int prcSamles = 0;


            // set time
            Utils::TimerTicks timer;    
            while (cycleCount < CCycles)
            {
                int size = CDataChank;            
                if (size + pos > m_data.size()) size = m_data.size() - pos;

                m_codec->DecodeBlock(&m_data[pos], &chankBuff[0], size);
                cycleCount += IncPos(pos, size);            

                if (size < CDataChank) 
                {
                    int rest = CDataChank - size;
                    m_codec->DecodeBlock(&m_data[pos], &chankBuff[size], rest);
                    IncPos(pos, rest);                
                }                

                //ESS_ASSERT(m_chankBuff.size() == CDataChank);				
                m_detect.Process(chankBuff);
                prcSamles += chankBuff.size();
                m_detect.ReadDetected();	                
			}
			            
            // speed
            ESS_ASSERT(prcSamles == m_data.size() * CCycles);
            return m_data.size() * CCycles * 1000 / timer.Get();		                             
        }
    };

    void RunSpeedTest(bool useALaw)
    {
        DtmfSpeedTest test(useALaw);
        int speed = test.Run();
        std::cout << (useALaw ?  "ALaw " : "ULaw ") << " Speed: " << speed << " sample/sec." << std::endl;                
        std::cout << "Ch: " << speed / 8000 << std::endl;                
    }

    
} // namespace 

namespace iDSP
{
    void DtmfBenchmark()
    {   
        std::cout << "Start dtmf benchmark..." << std::endl;                
        RunSpeedTest(true);
        RunSpeedTest(false);
    }

} // namespace iDSP