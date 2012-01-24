#include "stdafx.h"
#include "BidirBuffer.h"
#include "ErrorsSubsystem.h"

namespace Utils
{
    BidirBuffer::BidirBuffer( IFixedMemBlockPool& alloc, int offset ) : m_alloc(alloc),
        m_pBlock(static_cast<byte*>( alloc.alloc(alloc.getBlockSize()) )),
        m_blockSize(m_alloc.getBlockSize())
    {
        SetDataOffset(offset);
    }

    // -----------------------------------------------------------------------------------------

    BidirBuffer::BidirBuffer( IFixedMemBlockPool& alloc, Offset specOffset ) : m_alloc(alloc),
        m_pBlock(static_cast<byte*>( alloc.alloc(alloc.getBlockSize()) )),
        m_blockSize(m_alloc.getBlockSize())
    {
        int offset = 0;

        if (specOffset == CCenter) offset = m_blockSize / 2;
        else
        if (specOffset == CBegin) offset = 0;
        else
        if (specOffset == CEnd) offset = m_blockSize;        
        else
            ESS_ASSERT("Invalid offset");

        SetDataOffset(offset);
    }

    // -----------------------------------------------------------------------------------------

    BidirBuffer::~BidirBuffer()
    {
        m_alloc.free(m_pBlock);
    }

    // -----------------------------------------------------------------------------------------

    void BidirBuffer::SetDataOffset( int offset, int size /*= 0*/ ) /* ~ Clear() */
    {
        ESS_ASSERT(offset >= 0 && size >= 0);
        ESS_ASSERT((offset + size) <= m_blockSize && "OutOfRange!");
        m_pData = m_pBlock + offset;
        m_dataSize = size;
    }

    // -----------------------------------------------------------------------------------------

    int BidirBuffer::GetDataOffset() const
    {
        return m_pData - m_pBlock;
    }

    //-----------------------------------------------------------------------------------------

    void BidirBuffer::Clear()
    {
        SetDataOffset(GetDataOffset());
    }

    // -----------------------------------------------------------------------------------------

    void BidirBuffer::PushBack(const void* pSrc, int size )
    {
        AddSpaceBack(size);
        memcpy(Back(size - 1), pSrc, size);
    }

    // -----------------------------------------------------------------------------------------

    void BidirBuffer::PushBack(BidirBuffer &src)
    {
        PushBack(src.Front(), src.Size());
    }

    // -----------------------------------------------------------------------------------------

    void BidirBuffer::PushFront(BidirBuffer &src)
    {
        PushFront(src.Front(), src.Size());
    }

    // -----------------------------------------------------------------------------------------

    void BidirBuffer::PushBack( byte val )
    {
        PushBack(&val, 1);
    }

    // -----------------------------------------------------------------------------------------

    void BidirBuffer::PushFront(const void* pSrc, int size )
    {
        if (size == 0) return; // ignore
        SetDataOffset(GetDataOffset() - size, m_dataSize + size);
        memcpy(Front(), pSrc, size);
    }

    // -----------------------------------------------------------------------------------------

    void BidirBuffer::PushFront( byte val )
    {
        PushFront(&val, 1);
    }

    // -----------------------------------------------------------------------------------------

    void BidirBuffer::AddSpaceBack( int distance ) /* size += distance */
    {
        SetDataOffset(GetDataOffset(), m_dataSize + distance);
    }

    // -----------------------------------------------------------------------------------------

    void BidirBuffer::AddSpaceFront( int distance ) /* size += distance */
    {
        SetDataOffset(GetDataOffset() - distance, m_dataSize + distance);
    }

    // -----------------------------------------------------------------------------------------

    byte* BidirBuffer::Front()
    {        
        ESS_ASSERT( Size() != 0 );
        return m_pData;
    }

    // -----------------------------------------------------------------------------------------

    const byte* BidirBuffer::Front() const
    {
        ESS_ASSERT( Size() != 0 );
        return m_pData;
    }

    // -----------------------------------------------------------------------------------------

    byte* BidirBuffer::Back( int offset /*= 0*/ ) /* offset bytes from end of data */
    {
        ESS_ASSERT( Size() != 0 );
        return &At(Size() - offset - 1);
    }

    // -----------------------------------------------------------------------------------------

    const byte* BidirBuffer::Back( int offset /*= 0*/ ) const /* offset bytes from end of data */
    {
        ESS_ASSERT( Size() != 0 );
        return &At(Size() - offset - 1);
    }

    // -----------------------------------------------------------------------------------------

    byte* BidirBuffer::BackWritePointer(/* out */ int &maxSize)
    {
        byte *pBack = m_pData + m_dataSize;
        maxSize = (m_pBlock + m_blockSize) - (pBack);
        return (maxSize > 0) ? pBack : 0;
    }

    // -----------------------------------------------------------------------------------------

    byte& BidirBuffer::At( int index )
    {
        ESS_ASSERT (index >= 0);
        ESS_ASSERT (index < Size());
        return *(m_pData + index);
    }

    // -----------------------------------------------------------------------------------------

    const byte& BidirBuffer::At( int index ) const
    {
        ESS_ASSERT (index >= 0);
        ESS_ASSERT (index < Size());
        return *(m_pData + index);
    }

    // -----------------------------------------------------------------------------------------

    byte& BidirBuffer::operator[]( int index )
    {
        return At(index);
    }

    // -----------------------------------------------------------------------------------------

    const byte& BidirBuffer::operator[]( int index ) const
    {
        return At(index);
    }

    // -----------------------------------------------------------------------------------------

    int BidirBuffer::getBlockSize() const
    {
        return m_blockSize;
    }

    // -----------------------------------------------------------------------------------------

    int BidirBuffer::Size() const
    {
        return m_dataSize;
    }

    // -----------------------------------------------------------------------------------------

    int BidirBuffer::AllocSizeof()
    {
        return sizeof(BidirBuffer) + Base::CExtraSize;
    }

    // -----------------------------------------------------------------------------------------

    byte BidirBuffer::PopBack()
    {
        byte result = *Back();
        SetDataOffset(GetDataOffset(), m_dataSize - 1);
        return result;
    }

    // -----------------------------------------------------------------------------------------

    byte BidirBuffer::PopFront()
    {
        byte result = *Front();
        SetDataOffset(GetDataOffset() + 1, m_dataSize - 1);
        return result;
    }

} // namespace Utils

