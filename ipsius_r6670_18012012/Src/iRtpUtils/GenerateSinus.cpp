#include "stdafx.h"
#include "iDSP/g711codec.h"
#include "GenerateSinus.h"


namespace iRtpUtils
{   
    using namespace Platform;

    bool UseALow(const iMedia::Codec& codec)
    {
        if (codec.getName() == "PCMU") return false;        
        if (codec.getName() == "PCMA") return true;        
        ESS_UNIMPLEMENTED; // todo support other codecs
        return false; // suppress warning
    }

    GenerateSinus::GenerateSinus(int amplitude, 
        int frec) :
        m_amplitude(amplitude),
        m_frec(frec)
    {}
    
    const std::string GenerateSinus::Name("Generator");

    //-----------------------------------------------------------------------
    // IDataSrcForRtpTest
    
    void GenerateSinus::Setup(const iMedia::Codec &codec)
    {
        ESS_ASSERT(!m_sinus.get());
        m_sinus.reset(new SinusArray(m_amplitude, m_frec, codec));
    }

    //-----------------------------------------------------------------------

    QByteArray GenerateSinus::Read(int size)
    {        
        return (size) ? m_sinus->Read(size) : QByteArray();
    }

    //-----------------------------------------------------------------------
    // GenerateSinus::SinusArray

    GenerateSinus::SinusArray::SinusArray(int amplitude, int frec, const iMedia::Codec& codec) : 
        m_pos(0)
    {
        ESS_ASSERT(amplitude >= 0 && amplitude <= 100);

        const word CFullSwing = ~word(0);
        const word CAMax = CFullSwing / 2;
        float absoleteAmplitude = (float)amplitude / 100 * CAMax;

        const double CPi = 3.141592;

        //convention Hz to rad/sec, eval period and writing step
        double w = frec * 2 * CPi;
        float period = 1/(float)frec; 
        float step = 1/(float)(codec.getRate());

        for (float time = 0; time < period; time += step)
        {
            float arg = w * time;            
            Platform::int16 src = (Platform::int16)( absoleteAmplitude * sin(arg)); 
        
            if (UseALow(codec))
                m_array.push_back( iDSP::linear2ulaw(src) );
            else
                m_array.push_back( iDSP::linear2alaw(src) );                           
        }         
    }

    //-----------------------------------------------------------------------

    QByteArray GenerateSinus::SinusArray::Read( int size )
    {
        ESS_ASSERT(size > 0);
        int mySize = m_array.size();
        QByteArray rez;

        if (size + m_pos <= mySize)
        {
            rez = m_array.mid(m_pos, size);
            m_pos += size;
            return rez;
        }

        //write head
        int headSize = mySize - m_pos;
        rez.append( m_array.mid(m_pos, headSize) );

        //write cycles if exist
        int cycles = (size - headSize) / mySize;
        for (int n = 0; n < cycles; ++n)
        {
            rez.append(m_array);
        }

        //write tail if exist
        int tailSize = size - headSize - cycles * mySize;
        rez.append( m_array.mid(0, tailSize) );

        //member pos
        m_pos = tailSize;

        return rez;
    }

    void GenerateSinus_db::Setup( const iMedia::Codec &codec )
    {
        m_gen.reset( new iDSP::FreqGenerator(m_freq, m_dBm0, UseALow(codec)) );
    }

    QByteArray GenerateSinus_db::Read( int size )
    {
        m_cash.clear();
        m_cash.resize(size);        
        m_gen->Fill(reinterpret_cast<Platform::byte*>( m_cash.data()), size);
        return m_cash;
    }
}; //iRtpUtils
