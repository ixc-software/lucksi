
#include "stdafx.h"
#include "MemReaderStream.h"


namespace Utils
{
    
    byte MemReaderStream::ReadByte()    // can throw
    {
        CheckSize(sizeof(byte));

        byte res = *(m_pStartAddr + m_pos);
        ++m_pos;

        return res;
    }

    // ---------------------------------------------------

    // length == -1 -- read all, 
    void MemReaderStream::Read(void *pDest, size_t length) // can throw
    {
        CheckSize(length);
        
        memcpy(pDest, (m_pStartAddr + m_pos), length);
        m_pos += length;
    }

    // ---------------------------------------------------

    void MemReaderStream::CheckSize(size_t size)
    {
        if ((m_pos + size) > m_size) ESS_THROW(Utils::NotEnoughData);
    }
    
} // namespace Utils
