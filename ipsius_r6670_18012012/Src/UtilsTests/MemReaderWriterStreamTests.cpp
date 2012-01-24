
// MemReaderWriterStreamTests.cpp

#include "stdafx.h"
#include "MemReaderWriterStreamTests.h"
#include "Utils/MemReaderStream.h"
#include "Utils/MemWriterStream.h"

// -------------------------------------------------

namespace MemReaderStreamTests
{
    using namespace Utils;
    
    void ReadByteTest()
    {
        byte buff[2] = {3, 8};
        MemReaderStream reader(buff, sizeof(buff));

        TUT_ASSERT(reader.HasToRead() == sizeof(buff));
        TUT_ASSERT(reader.ReadByte() == 3);
        TUT_ASSERT(reader.ReadByte() == 8);
        TUT_ASSERT(reader.HasToRead() == 0);

        try
        {
            reader.ReadByte();
        }
        catch(Utils::NotEnoughData) { return; }
        TUT_ASSERT(0 && "ReadByteTest exception hasn't been thrown");
    }

    // ---------------------------------------------------
    
    void ReadTest()
    {
        const size_t CSize = 5;
        byte buff[CSize] = {0, 1, 2, 3, 4};
        MemReaderStream reader(buff, CSize);

        const size_t CReadSize = CSize - 2;
        byte dest[CReadSize];
        reader.Read(dest, CReadSize);

        for (size_t i = 0; i < CReadSize; ++i)
        {
            TUT_ASSERT(dest[i] == buff[i]);
        }
        
        try
        {
            byte dest2[10];
            reader.Read(dest2, sizeof(dest2));
        }
        catch(Utils::NotEnoughData) { return; }
        
        TUT_ASSERT(0 && "ReadTest exception hasn't been thrown");
    }
    
} // namespace MemReaderStreamTests

//  -------------------------------------------------

namespace MemWriterStreamTests
{
    using namespace Utils;
    
    void WriteByteTest()
    {
        byte staticBuff;
        MemWriterStream wr(&staticBuff, sizeof(staticBuff));

        byte b = 15;
        wr.Write(b);
        TUT_ASSERT(wr.BufferSize() == sizeof(staticBuff)); // do not resize
        TUT_ASSERT(*wr.GetBuffer() == b);
        TUT_ASSERT(staticBuff == b);

        // buffer full
        try
        {
            wr.Write(1);
        }
        catch(Utils::NotEnoughMemory &e) { return; }

        TUT_ASSERT(0 && "WriteByteTest exception hasn't been thrown");
    }

    // -------------------------------------------------
    
    void WriteTest()
    {
        const size_t staticBuffSize = 5;
        byte staticBuff[staticBuffSize];
        MemWriterStream wr(staticBuff, staticBuffSize);

        const size_t dataLen = staticBuffSize - 1;
        byte data[dataLen] = {8, 9, 10, 11};
        wr.Write(data, dataLen); 
        TUT_ASSERT(wr.BufferSize() == dataLen); // do not resize 

        for (size_t i = 0; i < (staticBuffSize - 1); ++i)
        {
            TUT_ASSERT(staticBuff[i] == data[i]);
        }

        // data size == 0
        wr.Write(staticBuff, 0);
        wr.Write(0, 0);

        // data's size > buffer's left size 
        try
        {
            wr.Write(data, dataLen);
        }
        catch(Utils::NotEnoughMemory &e) { return; }

        TUT_ASSERT(0 && "WriteTest exception hasn't been thrown");
    }

    // -------------------------------------------------

    void GetBufferTest()
    {
        byte data = 5;
        byte *pStaticBuff = 0;
        byte buff;
        {
            MemWriterStream wr(&buff, 1);
            TUT_ASSERT(wr.GetBuffer() != 0);

            wr.Write(data);
            pStaticBuff = wr.GetBuffer();
            TUT_ASSERT(*pStaticBuff == data);
        }
        TUT_ASSERT(*pStaticBuff == data); // buffer still remaining
    }
    
} // namespace MemWriterStreamTests

//  -------------------------------------------------

namespace MemWriterDynStreamTests
{
    using namespace Utils;
    
    void WriteByteTest()
    {
        size_t startSize = 1;
        MemWriterDynStream wr(startSize);
        TUT_ASSERT(wr.GetBuffer() != 0);
        
        byte b = 15;
        wr.Write(b);
        TUT_ASSERT(wr.BufferSize() == startSize);
        TUT_ASSERT(*wr.GetBuffer() == b);

        byte b2 = 16;
        wr.Write(b2);
        TUT_ASSERT(wr.BufferSize() == (startSize * 2)); // resized
        TUT_ASSERT(*(wr.GetBuffer() + 1) == b2);
    }

    // -------------------------------------------------
    
    void WriteTest()
    {
        const size_t dataLen = 4;
        MemWriterDynStream wr(dataLen);
        
        byte data[dataLen] = {1, 2, 3, 4};
        wr.Write(data, dataLen);
        TUT_ASSERT(wr.BufferSize() == dataLen);

        byte *pBuff = wr.GetBuffer();
        for (size_t i = 0; i < dataLen; ++i)
        {
            TUT_ASSERT(*pBuff++ = data[i]);
        }

        const size_t data2Len = 2;
        byte data2[data2Len] = {5, 6};
        wr.Write(data2, data2Len);
        TUT_ASSERT(wr.BufferSize() == (dataLen + data2Len)); // resized

        pBuff = wr.GetBuffer();
        for (size_t i = 0; i < data2Len; ++i)
        {
            TUT_ASSERT(*(pBuff + dataLen + i) = data[i]); // check write position changed
        }

        // data size == 0
        wr.Write(data, 0);
        wr.Write(0, 0);
    }

    // -------------------------------------------------

    void SetCapacityTest()
    {
        const size_t dataLen = 3;
        byte data[dataLen] = {1, 2, 3};
        
        // 1: create emtpy, then set capacity 
        {
            MemWriterDynStream wr;
            wr.setCapacity(dataLen);
            TUT_ASSERT(wr.BufferSize() == 0);
            TUT_ASSERT(wr.GetBuffer() != 0);
        }
        
        // 2: create with start capacity, write data, then set bigger capacity
        {
            MemWriterDynStream wr2(dataLen - 1);
            byte b = 5;
            wr2.Write(b);
            TUT_ASSERT(wr2.BufferSize() == sizeof(b));

            wr2.setCapacity(dataLen);
            TUT_ASSERT(wr2.BufferSize() == sizeof(b));
            TUT_ASSERT(*wr2.GetBuffer() == b);
        }

        // 3: create with start capacity, write data, 
        // then set capacity smaller then written data size
        {
            MemWriterDynStream wr3(dataLen);
            wr3.Write(data, dataLen);
            TUT_ASSERT(wr3.BufferSize() == dataLen);

            size_t newDataLen = dataLen - 1;
            wr3.setCapacity(newDataLen);
            TUT_ASSERT(wr3.BufferSize() == newDataLen);
            TUT_ASSERT(*wr3.GetBuffer() == data[0]);
            TUT_ASSERT(*(wr3.GetBuffer() + 1) == data[1]);
        }

        // 4: can set capacity after detach if can create new buffer after detach 
        {
            MemWriterDynStream wr4(0, true);
            wr4.Write(5);
            wr4.DetachBuffer();
            
            wr4.setCapacity(2);
        }
         
        // 5: can't set capacity after detach if can't create new buffer after detach 
        {
            MemWriterDynStream wr5;
            wr5.Write(5);
            wr5.DetachBuffer();
            // wr5.setCapacity(2);
        }
    }

    // -------------------------------------------------

    void DetachTest()
    {
        // 1: can't write after detach
        byte b = 5;
        {
            MemWriterDynStream wr;
            // write
            wr.Write(b);
            TUT_ASSERT(wr.BufferSize() == (sizeof(b)));
            TUT_ASSERT(*wr.GetBuffer() == b);
            // detach
            boost::shared_ptr<ManagedMemBlock> checkBuff(wr.DetachBuffer()); 
            TUT_ASSERT(*checkBuff->getData() == b);
            TUT_ASSERT(checkBuff->Size() == sizeof(b));
            TUT_ASSERT(wr.BufferSize() == 0);
            TUT_ASSERT(wr.GetBuffer() == 0);
            
            // can't detach again
            // boost::shared_ptr<ManagedMemBlock> checkBuff2(wr.DetachBuffer()); 
            // cant't write again
            // wr.Write(b);
            
        }
        
        // 2: can write after detach
        {
            MemWriterDynStream wr2(0, true);
            // write
            wr2.Write(b);
            // detach
            boost::shared_ptr<ManagedMemBlock> checkBuff = wr2.DetachBuffer(); 
            wr2.setCapacity(sizeof(b));
            // can write again
            wr2.Write(b);
            TUT_ASSERT(wr2.BufferSize() == sizeof(b));
            TUT_ASSERT(*wr2.GetBuffer() == b);
        }

        // 3: can't detach empty buffer
        {
            MemWriterDynStream wr3;
            // boost::shared_ptr<ManagedMemBlock> checkBuff = wr3.DetachBuffer();
        }

        // 4: can't detach empty buffer: 
        // buffer is concidered as empty even if start capacity != 0
        {
            MemWriterDynStream wr4(1);
            // boost::shared_ptr<ManagedMemBlock> checkBuff = wr4.DetachBuffer();
        }

        /*
        // 5: can't call CreateNewBufferAfterDetach() before detach
        {
            MemWriterDynStream wr5;
            // wr5.CreateNewBufferAfterDetach(1);
        }
        */
    }
    
    // --------------------------------------------------

    void ManagedMemBlockTest()
    {
        byte data = 2;
        const size_t size = 1;

        // 1: use outer buffer as inner
        {
            byte buff[size];
            buff[0] = data;
            
            {
                ManagedMemBlock block(buff, size);
                TUT_ASSERT(block.Size() == size);
                TUT_ASSERT(*block.getData() == data);
                
            } 
        } 

        // 2: copy outer buffer

        class BufferManager
        {
            byte *m_pBuff;
        public:
            BufferManager(size_t size, byte data) : m_pBuff(new byte[size])
            {
                *m_pBuff = data;
            }

            byte* getBuffer() { return m_pBuff; }
            ~BufferManager() { delete[] m_pBuff; }
        };
        
        {
            BufferManager buff2(size, data);

            {
                ManagedMemBlock block2(buff2.getBuffer(), size);
                TUT_ASSERT(block2.Size() == size);
                TUT_ASSERT(*block2.getData() == data);
                
            } // ~ManagedMemBlock: delete inner buffer

            TUT_ASSERT(*buff2.getBuffer() == data);

            // can't pass 'false' as 3rd parameter
            {
                // ManagedMemBlock block3(buff2.getBuffer(), size, false);
            }
        }
    }
    
    // --------------------------------------------------

    void GetBufferTest()
    {
        byte *pDynamicBuff = 0;
        byte b = 123;
        {
            MemWriterDynStream wr(sizeof(b));
            TUT_ASSERT(wr.GetBuffer() != 0); // buffer created

            wr.Write(b);
            pDynamicBuff = wr.GetBuffer();
            TUT_ASSERT(*pDynamicBuff == b);
        }
        TUT_ASSERT(*pDynamicBuff != b); // buffer destroyed
    }

    
} // namespace MemWriterDynStreamTests

//  -------------------------------------------------

namespace UtilsTests 
{
    void MemReaderWriterStreamTests()
    {
        MemReaderStreamTests::ReadByteTest();
        MemReaderStreamTests::ReadTest();

        MemWriterStreamTests::WriteByteTest();
        MemWriterStreamTests::WriteTest();
        MemWriterStreamTests::GetBufferTest();
        
        MemWriterDynStreamTests::WriteByteTest();
        MemWriterDynStreamTests::WriteTest();
        MemWriterDynStreamTests::GetBufferTest();
        MemWriterDynStreamTests::DetachTest();
        MemWriterDynStreamTests::SetCapacityTest();
        MemWriterDynStreamTests::ManagedMemBlockTest();
    }
    
} // namespace UtilsTests
