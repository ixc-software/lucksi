#ifndef __CRC32__
#define __CRC32__

#include <boost/crc.hpp> 

// #include "stdafx.h" <-- don't include, 'couse this file used with MocExt

namespace Utils
{

    typedef boost::crc_32_type::value_type TCRC;

    class CRC32
    {
          
        TCRC m_crc;
    
    public:

        CRC32(TCRC crc) 
        {
            m_crc = crc;
        }

        CRC32(const QByteArray &qByteArray)
        {
            boost::crc_32_type crc;
            
            crc.process_bytes( qByteArray.constData(), qByteArray.size());
            m_crc = crc.checksum();                                                                

        }

        TCRC checksum() const 
        {
            return m_crc;
        }

        bool operator == (const CRC32 &other) const
        {
            return (m_crc == other.m_crc);
        }

        bool operator != (const CRC32 &other) const
        {
            return !(m_crc == other.m_crc);
        }

        static void TestAgainRaw();

    };


}  // namespace Utils


#endif

