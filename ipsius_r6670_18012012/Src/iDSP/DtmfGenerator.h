#ifndef DTMFGENERATOR_H
#define DTMFGENERATOR_H

#include "g711gen.h"
#include "Utils/ManagedList.h"

namespace iDSP
{
    struct DtmfGeneratorProfile 
    {
        int Level;        
        int DurationMs;
        int PauseMs; // используется только при серии

        DtmfGeneratorProfile()
        {
            DurationMs = 80;
            Level = 0;
            PauseMs = 40;
        }

        bool IsValid() const 
        {
            // todo Q24     
            return 
                DurationMs > 70 &&
                PauseMs > 25 &&
                Level <= 10;
        }

    };    
    
    class DtmfGenerator : boost::noncopyable
    {
        class Gen;
        const DtmfGeneratorProfile m_prof;        
        Utils::ManagedList<Gen> m_genList;        
    public:
        DtmfGenerator( const DtmfGeneratorProfile& prof = DtmfGeneratorProfile() );
        ~DtmfGenerator();
                
        void Add(const std::string& str);

        int NextSample();

        template<class TCont>
            void Fill(TCont &c, int count)
        {
            ESS_ASSERT(count <= RestSampleCount());
            while(count--)
            {
                c.push_back( NextSample() );
            }
        }
        
        int RestSampleCount() const; 
        bool Empty() const; // no more samples

        void Reset();        
    };



    // impl use SegmentGenerator ?

} // namespace iDsp

#endif
