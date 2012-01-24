#ifndef DTMFDETECTOR_H
#define DTMFDETECTOR_H

#include "Utils/IBasicInterface.h"
#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/RawCRC32.h"

namespace iDSP
{
    struct DtmfDetectorProfile
    {
        //bool Rej350_440;
        int FwTwist;    //Db, максимальная разница уровней частот в комбинации 
        int RevTwist;   //Db ?
        int Threshold;  //Db, порог обработки сигнала 

        DtmfDetectorProfile()
        {
            //Rej350_440 = false;
            FwTwist = 8;    
            RevTwist = 8;  
            Threshold = -42;//-26; 
        }        

        bool IsValid() const
        {
            // todo
            return true;
        }
    };

   
    class DtmfDetector : boost::noncopyable
    {              
        class DtmfState;
        boost::scoped_ptr<DtmfState> m_stateImpl;
        const DtmfDetectorProfile m_prof;                
        std::string m_detected;

        std::string m_cash;

    private:
        friend void EvDigitDetected(void*, const char*, int);
        void OnDetect(const char* digit, int len);

    public:        
        DtmfDetector(const DtmfDetectorProfile& prof = DtmfDetectorProfile());
        ~DtmfDetector();

        static const std::string& PossibleSymbols();
        
        void Process(Platform::int16 sample);
        void Process(const std::vector<Platform::int16>& data);
        void Process(const Platform::int16* data, int size);
        
        const std::string& ReadDetected(); // return all detected and clear

        void Reset();

        Platform::dword StateCRC(Platform::dword crc = Utils::RawCRC32::InitialValue) const;
    };
} // namespace iDSP

#endif
