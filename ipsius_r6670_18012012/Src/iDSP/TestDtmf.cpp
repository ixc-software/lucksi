#include "stdafx.h"
#include "TestDtmf.h"
#include "DtmfGenerator.h"
#include "DtmfDetector.h"

#include "span_dtmf.h"

void EvDigitDetected(void *user_data, const char *digits, int len)
{}
void SpanDtmfProbe()
{
    std::string dtmfCode = "*1234567890#ABCD";    
    iDSP::DtmfGeneratorProfile prof;
    prof.Level = 0;
    prof.DurationMs = 80; // 70 мало
    prof.PauseMs = 40; // 50 20-мало
    iDSP::DtmfGenerator gen;    
    gen.Add(dtmfCode);    

    // Init:
    digits_rx_callback_t callback = 0;//EvDigitDetected;     
    void* pUserData = 0;
    dtmf_rx_state_t state;
    dtmf_rx_state_t* pState = &state;
    dtmf_rx_init(pState, callback, pUserData);    

    // Adjust detector:
    bool rej350_440 = false;
    int fwTwist = 8;    //8 Db максимальная разница уровней частот в комбинации 
    int revTwist = 8;  //-4 Db ?
    int threshold = -26; //Db минимальный уровень сигнала
    dtmf_rx_parms(pState, rej350_440, fwTwist, revTwist, threshold);

    
    while (gen.RestSampleCount() != 0)
    {
        int16_t sample = gen.NextSample();
        dtmf_rx(pState, &sample, 1);    
    }
    

    const int CMaxSymbols = 100; 
    char dtmfCodeRx[CMaxSymbols];
    int size = dtmf_rx_get(pState, dtmfCodeRx, CMaxSymbols);
    TUT_ASSERT(size == dtmfCode.size());
    dtmfCodeRx[size] = '\0';
    TUT_ASSERT(dtmfCode == dtmfCodeRx);
    

    // delete pState;
    //dtmf_rx_free(pState);
    //pState = 0;
}


namespace 
{
    void Test(bool printCRC)
    {           
        std::string dtmfCode = "*1234567890#ABCD";          
        iDSP::DtmfGenerator gen;
        gen.Add(dtmfCode);
        iDSP::DtmfDetector detect;

        Platform::dword srcCrc = Utils::RawCRC32::InitialValue;
        Platform::dword recCrc = Utils::RawCRC32::InitialValue;        

        while (gen.RestSampleCount() != 0)
        {
            int sample = gen.NextSample();            
            if (printCRC) srcCrc = Utils::UpdateCRC32( &sample, sizeof(int), srcCrc );                        
            
            detect.Process( sample );            
            if (printCRC) recCrc = detect.StateCRC(recCrc);            
        }                
        
        if (printCRC) std::cout << "CRC generator/detector: " << srcCrc << "/" << recCrc << std::endl;
        
        TUT_ASSERT(detect.ReadDetected() == dtmfCode);        
        TUT_ASSERT(detect.ReadDetected().empty());        
    }

} // namespace 


namespace iDSP
{
    void TestDtmf(bool silenceMode)
    {
        if (!silenceMode) std::cout << "Begin test dtmf..." << std::endl;
        Test(!silenceMode);
        if (!silenceMode) std::cout << "Test dtmf complete." << std::endl;
    }    
} // namespace iDSP

