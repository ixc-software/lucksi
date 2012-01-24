#ifndef BIDIRBUFFER_H
#define BIDIRBUFFER_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "IFixedMemBlockPool.h"
#include "BaseSpecAllocated.h"

namespace Utils
{    

    using Platform::byte;

    /*

        [.............xxxxxxxxxxxxxxxxxxxxxxxxxxxxx.......]

        ^             ^                           ^
        |             |                           |
        m_pBlock      m_pData (front)             back

        <-  offset  -><-      m_dataSize         ->

        <-                m_blockSize                    ->

    */

    class BidirBuffer
        : public BaseSpecAllocated<IFixedMemBlockPool>,
        boost::noncopyable
    {
        typedef BaseSpecAllocated<IFixedMemBlockPool> Base;
        
        IFixedMemBlockPool& m_alloc;

        byte *const m_pBlock;
        const int m_blockSize;
        
        byte* m_pData;      // inside block
        int   m_dataSize;

    public:

        BidirBuffer(IFixedMemBlockPool& alloc, int offset);

        enum Offset {CCenter = -1, CBegin = -2, CEnd = -3};
        BidirBuffer(IFixedMemBlockPool& alloc, Offset specOffset);

        ~BidirBuffer();

        void SetDataOffset(int offset, int size = 0);
        int GetDataOffset() const;
        
        void Clear();

        // max capacity
        int getBlockSize() const;

        int Size() const;

        static int AllocSizeof();

        void PushBack(BidirBuffer &src);
        void PushFront(BidirBuffer &src);

        void PushBack(const void* pSrc, int size);
        void PushFront(const void* pSrc, int size);
        
        void PushBack(byte val);
        void PushFront(byte val);

        byte PopBack();
        byte PopFront();

        void  AddSpaceBack(int distance);   // size += distance
        void  AddSpaceFront(int distance);  // size += distance

        byte* Front();        
        const byte* Front() const;
        
        byte* Back(int offset = 0);               // offset bytes from end of data
        const byte* Back(int offset = 0) const;   // -//-

        // for direct write to buffer thru pointer, without size change
        // use AddSpaceBack latter
        // maxSize -- space beyond pointer 
        byte* BackWritePointer(/* out */ int &maxSize); 

        byte& At(int index);
        const byte& At(int index) const;

        byte& operator[](int index);
        const byte& operator[](int index) const;
                
    };


} // namespace Utils

#endif

