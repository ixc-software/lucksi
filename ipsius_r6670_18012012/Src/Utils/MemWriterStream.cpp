
#include "stdafx.h"
#include "MemWriterStream.h"
#include "MemReaderStream.h"
#include "BinaryReader.h"

namespace
{
    using namespace Utils;
    
    void CopyToBuffer(byte *pBuff, const void *pSrc, size_t srcLength)
    {
        ESS_ASSERT(pBuff != 0);
        
        if (pSrc == 0) ESS_ASSERT(srcLength == 0);
        else memcpy(pBuff, pSrc, srcLength);
    }
    
} // namespace


// ----------------------------------------------------------------------

namespace Utils
{
    // MemWriterStream impl

    MemWriterStream::MemWriterStream(void *pStartAddr, size_t capacity) 
    : m_pStartAddr(static_cast<byte*>(pStartAddr)), m_capacity(capacity), m_writePos(0)
    {
        // don't check pStartAddr != 0
    }

    // ----------------------------------------------------------------------

    void MemWriterStream::CheckIfCanWrite(size_t dataSize)
    {
        // ESS_ASSERT(dataSize > 0);
        if (dataSize == 0) return;
        
        size_t requestedCapacity = m_writePos + dataSize;

        CheckCapacity(requestedCapacity);

        ESS_ASSERT(m_pStartAddr != 0);
    }

    // ----------------------------------------------------------------------

    void MemWriterStream::CheckCapacity(size_t requestedCapacity) // can throw
    {
        if (requestedCapacity <= m_capacity) return;
        ESS_THROW(Utils::NotEnoughMemory);
    }

    // ----------------------------------------------------------------------
    
    void MemWriterStream::Write(byte b) // can throw
    {
        CheckIfCanWrite(sizeof(b));

        // write
        *(m_pStartAddr + m_writePos) = b;
        m_writePos += sizeof(b);
    }

    // ----------------------------------------------------------------------
    
    void MemWriterStream::Write(const void *pSrc, size_t length) // can throw
    {
        CheckIfCanWrite(length);

        CopyToBuffer((m_pStartAddr + m_writePos), pSrc, length);
        m_writePos += length;
    }

    // ----------------------------------------------------------------------

    void MemWriterStream::Write( const MemWriterStream &other )
    {
        Write(other.GetBuffer(), other.BufferSize());
    }

    // ----------------------------------------------------------------------

    IBinaryReader* MemWriterStream::CreateReader()
    {
        MemReaderStream *pStream = new MemReaderStream(GetBuffer(), BufferSize());
        return new BinaryReaderEx<MemReaderStream>(pStream);
    }

    // ----------------------------------------------------------------------
    // ManagedMemBlock impl

    ManagedMemBlock::ManagedMemBlock(size_t size) : 
        m_pData(new byte[size]), 
        m_size(size)
    {
        ESS_ASSERT(m_pData != 0);
        ESS_ASSERT(m_size != 0);
    }

    // ----------------------------------------------------------------------

    ManagedMemBlock::ManagedMemBlock(const void *pStartAddr, size_t size) : 
        m_pData(0), 
        m_size(size)
    {
        ESS_ASSERT(pStartAddr != 0);
        ESS_ASSERT(size != 0);

        m_pData = new byte[m_size];
        CopyToBuffer(m_pData, pStartAddr, size);
    }

    // ----------------------------------------------------------------------
    // free inner memory bloc
    ManagedMemBlock::~ManagedMemBlock()
    {
        delete[] m_pData;
    }

    // ----------------------------------------------------------------------
    
    void ManagedMemBlock::Resize(size_t size)
    {   
        if(size > m_size)
        {
            byte *oldData = m_pData;
            m_pData = new byte[size];
            CopyToBuffer(m_pData, oldData, m_size);
            delete[] oldData;
        }
        m_size = size;
    }

    // ----------------------------------------------------------------------
    // MemWriterDynStream impl

    MemWriterDynStream::MemWriterDynStream(size_t capacity, bool createNewBufferAfterDetach) : 
        MemWriterStream(0, 0),
        m_detached(false),
        m_createNewBufferAfterDetach(createNewBufferAfterDetach)
    {
        if (capacity > 0) CreateNewBuffer(capacity);
    }

    // ----------------------------------------------------------------------
    
    MemWriterDynStream::~MemWriterDynStream()
    {
        //delete[] m_pStartAddr;
    }

    // ----------------------------------------------------------------------
    
    void MemWriterDynStream::Validate()
    {
        if(!m_pStartAddr || !m_block)
            ESS_ASSERT(!m_pStartAddr && !m_block && !m_capacity && !m_writePos);
        else
            ESS_ASSERT(m_pStartAddr && m_block->getData() == m_pStartAddr &&
                m_capacity == m_block->Size() && m_writePos <= m_capacity);

        ESS_ASSERT(m_capacity >= m_writePos);

        if(m_detached)
            ESS_ASSERT(!m_pStartAddr && !m_block);
    }

    // ----------------------------------------------------------------------

    void MemWriterDynStream::CreateNewBuffer(size_t newCapacity)
    {
        ESS_ASSERT(newCapacity > 0);

        Validate();

        if (m_capacity == newCapacity) 
            return;
        
        m_writePos = (m_writePos < newCapacity) ? m_writePos : newCapacity;
        
        if(newCapacity < m_capacity)
        {
            m_block->Resize(newCapacity);
        }
        else
        {
            boost::shared_ptr<ManagedMemBlock> oldBlock(new ManagedMemBlock(newCapacity));
            m_block.swap(oldBlock);
            if (oldBlock)
                CopyToBuffer(m_block->getData(), oldBlock->getData(), m_writePos);
        }
        m_pStartAddr = m_block->getData();
        m_capacity = newCapacity;
        m_detached = false;
    }

    // ----------------------------------------------------------------------
    
    void MemWriterDynStream::CheckCapacity(size_t requestedCapacity) // override
    {
        if(m_detached && !m_createNewBufferAfterDetach)
        {
            ESS_HALT("Data can't be written after buffer has been detached");
        }
        // realocate data block if need
        if (requestedCapacity <= m_capacity) return;

        // create buffer bigger than previous in >= 2 times
        size_t newCapacity = m_capacity * 2;
        newCapacity = (newCapacity < requestedCapacity)?
                              requestedCapacity : newCapacity;
        CreateNewBuffer(newCapacity);
    }

    // ----------------------------------------------------------------------
    
    boost::shared_ptr<ManagedMemBlock> MemWriterDynStream::DetachBuffer()
    {
        ESS_ASSERT(!m_detached);
        ESS_ASSERT(m_writePos != 0);
        Validate();

        boost::shared_ptr<ManagedMemBlock> result;
        result.swap(m_block);

        result->Resize(m_writePos);

        MemWriterStream::Clear();
        m_detached = true;
        return result;
    }

    // ----------------------------------------------------------------------

    void MemWriterDynStream::setCapacity(size_t val)
    {
        if (m_detached) ESS_ASSERT(m_createNewBufferAfterDetach);

        if (val > 0) CreateNewBuffer(val);
    }

    // ----------------------------------------------------------------------
    
} // namespace Utils
