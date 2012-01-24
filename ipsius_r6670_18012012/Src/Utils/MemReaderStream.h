
#ifndef __MEMREADERSTREAM__
#define __MEMREADERSTREAM__

// MemReaderStream.h

// for base exception type
#include "Utils/IBiRWStream.h" 

namespace Utils
{
    using Platform::byte;
    
    // Class has the same interface as IBiReaderStream
    // this is provided to make read operations faster for raw memory 
    class MemReaderStream : boost::noncopyable
    {
        const byte *m_pStartAddr;
        size_t m_size;
        size_t m_pos;

        void CheckSize(size_t size);
        
    public:
        MemReaderStream(const void *pStartAddr, size_t size)
            : m_pStartAddr(static_cast<const byte*>(pStartAddr)), m_size(size), m_pos(0)
        {
        }

        size_t HasToRead() const { return (m_size - m_pos); }
        
    // IBiReaderStream semantic
    public:
        byte ReadByte();    // can throw
        void Read(void *pDest, size_t length); // can throw
    };

    
} // namespace Utils

#endif
