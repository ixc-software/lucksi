#ifndef __BINARYWRITEBUFF__
#define __BINARYWRITEBUFF__

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"

#include "IBiRWStream.h"

namespace Utils
{
	
    class BinaryWriteBuff : public IBiWriterStream, boost::noncopyable
    {
        int m_capacity;
        int m_size;
        Platform::byte *m_pBuff;

    // IBiWriterStream impl
    public:

        void Write(byte b)
        {
            Write(&b, sizeof(b));
        }

        void Write(const void *pSrc, size_t length)
        {
            if (FreeBytes() < length) ESS_THROW(NotEnoughMemory);

            std::memcpy(End(), pSrc, length);
            AddToSize(length);
        }


    public:

        BinaryWriteBuff(int capacity) : m_capacity(capacity)
        {
            m_pBuff = new Platform::byte[m_capacity];
            Clear();
        }

        ~BinaryWriteBuff()
        {
            delete[] m_pBuff;
        }

        Platform::byte ReadByte(int indx) const
        {
            ESS_ASSERT(indx < m_size);
            return m_pBuff[indx];
        }

        int Size() const
        {
            return m_size;
        }

        int Capacity() const
        {
            return m_capacity;
        }

        int FreeBytes() const
        {
            return m_capacity - m_size;
        }

        bool Full() const
        {
            return (FreeBytes() == 0);
        }

        bool Empty() const
        {
            return (Size() == 0);
        }

        void Clear()
        {
            m_size = 0;
        }

        const void* Begin(int offsetBytes = 0) const 
        {
            ESS_ASSERT(Size() > offsetBytes);  // at least one byte beyond requested position
            return m_pBuff + offsetBytes;
        }

        void* End()
        {
            return &m_pBuff[m_size];
        }

        void AddToSize(int bytes)
        {
            if (bytes == 0) return;

            m_size += bytes;
            ESS_ASSERT(m_size <= m_capacity);
        }

    };

		
}  // namespace Utils

#endif
