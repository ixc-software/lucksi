#include "stdafx.h"
#include "DtmfGenerator.h"

namespace 
{
    const int CTabSize = 4;
    const char* CDtmfPos[] = {"123A", "456B", "789C", "*0#D"};     
    const int CFHi[CTabSize] = {1209, 1336, 1477, 1633}; // col
    const int CFLow[CTabSize] = {697, 770, 852, 941}; // row

    struct DtmfPos
    {
        int Row;
        int Col;  

    public:

        DtmfPos(char s)
        {
            for (int r = 0; r < CTabSize; ++r)
            {            
                for (int c = 0; c < CTabSize; ++c)
                {
                    if (CDtmfPos[r][c] != s) continue;
                    Row = r;
                    Col = c;
                    return;
                }
            }
            ESS_HALT("Wrong character");
        }

        int FLow() const
        {
            return CFLow[Row];
        }
        int FHi() const
        {
            return CFHi[Col];
        }
    };
     
    
} // namespace 

namespace iDSP
{

    class DtmfGenerator::Gen
    {
        enum {CSampleRate = 8}; // (sample/msec)

        LineMultiFreqGenerator m_gen;
        int m_signalSamples;
        int m_silentSamples;
    public:
        Gen(const DtmfPos& pos, const DtmfGeneratorProfile& prof)
        {
            ESS_ASSERT(prof.IsValid());

            m_gen.Add(pos.FHi(), prof.Level);
            m_gen.Add(pos.FLow(), prof.Level);
            m_signalSamples = CSampleRate * prof.DurationMs;
            m_silentSamples = CSampleRate *prof.PauseMs;
        } 
        int RestSample() const
        {
            return m_signalSamples + m_silentSamples;
        }        
        bool Empty() const
        {
            return m_signalSamples == 0 && m_silentSamples == 0;
        }
        int Next()
        {            
            //ESS_ASSERT( !Empty() );
            if (m_signalSamples > 0)
            {
                --m_signalSamples;
                return m_gen.Next();
            }
            if (m_silentSamples > 0)
            {
                --m_silentSamples;
                return 0;
            }
            ESS_HALT("No samples");
            return 0;
        }
    };

    // ------------------------------------------------------------------------------------

    DtmfGenerator::DtmfGenerator( const DtmfGeneratorProfile& prof ) : m_prof(prof)
    {
    }    

    void DtmfGenerator::Add( const std::string& str )
    {        
        // todo to up reg
        ESS_ASSERT(std::string::npos == str.find_first_not_of("0123456789*#ABCD"));

        for (int i = 0; i < str.size(); ++i)
        {
            DtmfPos pos( str.at(i) );
            m_genList.Add( new Gen(pos, m_prof) );
        }
    }

    int DtmfGenerator::NextSample()
    {        
        ESS_ASSERT (!m_genList.IsEmpty());
        int rez = m_genList.Front()->Next();        
        if ( m_genList.Front()->Empty() ) m_genList.Delete(0);
        return rez;
    }

    int DtmfGenerator::RestSampleCount() const
    {
        int rest = 0;
        for (int i = 0; i < m_genList.Size(); ++i)
        {
            rest += m_genList[i]->RestSample();
        }   
        return rest;
    }    

    DtmfGenerator::~DtmfGenerator()
    {}    


    void DtmfGenerator::Reset()
    {
        m_genList.Clear();
    }

    bool DtmfGenerator::Empty() const
    {
        for (int i = 0; i < m_genList.Size(); ++i)
        {
            if(!m_genList[i]->Empty()) return false;
        } 
        return true;
    }
} // namespace iDSP



