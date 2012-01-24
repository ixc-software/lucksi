
#ifndef __MEMWRITERSTREAM__
#define __MEMWRITERSTREAM__

// MemWriterStream.h

// for base exception type
#include "Utils/IBiRWStream.h"
#include "Utils/IBinaryReader.h"

namespace Utils
{
    // Using for writing data to given memory segment.
    class MemWriterStream : public Utils::IBiWriterStream, boost::noncopyable
    {
        void CheckIfCanWrite(size_t dataSize);

    protected:
        byte *m_pStartAddr;
        size_t m_capacity;
        size_t m_writePos;

        // void setSize(size_t val) { m_writePos = val; }

        virtual void CheckCapacity(size_t requestedCapacity); // can throw

        void Clear()
        {
            m_pStartAddr = 0;
            m_capacity = 0;
            m_writePos = 0;
        }

    // Utils::IBiWriterStream impl
    public:

        void Write(byte b); 
        void Write(const void *pSrc, size_t length);

    public:
        MemWriterStream(void *pStartAddr, size_t capacity);

        virtual ~MemWriterStream() 
        {
            Clear();
        }

        void ClearBufferSize()
        {
            m_writePos = 0;
        }

        byte* GetBuffer() const { return m_pStartAddr; }
        size_t BufferSize() const { return m_writePos; }

        // result must be deleted by caller
        IBinaryReader* CreateReader();

        // methods can throw
        void Write(const MemWriterStream &other);
    };

    // ---------------------------------------------

    // Free inner memory block in destructor.
    class ManagedMemBlock : boost::noncopyable
    {
        byte* m_pData;
        size_t m_size;
        
    public:
        // save given memory block as inner
        //ManagedMemBlock(void *pStartAddr, size_t size);

        // copy data from given memory block to inner block,
        ManagedMemBlock(const void *pStartAddr, size_t size);

        // create inner memory block
        ManagedMemBlock(size_t size);

        // free inner memory bloc
        ~ManagedMemBlock();
        
        void Resize(size_t );
        byte* getData() const { return m_pData; }
        size_t Size() const { return m_size; }
    };
    
    // ---------------------------------------------

    // Using for writing data to inner buffer.
    // Write methods have the same semantic as IBiWriterStream's methods, 
    // this is provide to make write operations faster for raw memory 
    class MemWriterDynStream : public MemWriterStream
    {
        boost::shared_ptr<ManagedMemBlock> m_block;
        bool m_detached;
        bool m_createNewBufferAfterDetach;
        
        void CreateNewBuffer(size_t newCapacity);
        void Validate();
        void CheckCapacity(size_t requestedCapacity); // override
        
    public:
        MemWriterDynStream(size_t capacity = 0, bool createNewBufferAfterDetach = false);
        ~MemWriterDynStream();
        
        boost::shared_ptr<ManagedMemBlock> DetachBuffer();

        // if new capacity < old capacity, buffer will be chopped
        void setCapacity(size_t val);
    };

/*
    
    // Using for writing data to given memory segment.
    // Write methods have the same semantic as IBiWriterStream's methods, 
    // this is provided for making write operations faster for raw memory 
    class MemWriterStream : boost::noncopyable
    {
        byte *m_pBuffStart;
        size_t m_size;
        size_t m_pos;

        void CheckSize(size_t size);
        
    public:
        MemWriterStream(void *pStartAddr, size_t size);

        // IBiWriterStream semantic
        void Write(byte b); // can throw
        void Write(const void *pSrc, size_t length); // can throw

        byte* GetBuffer() const { return m_pBuffStart; }
        size_t BufferSize() const { return m_size; }
    };

    // -----------------------------------------------------

    // Noncopyable memory block(can be used in pair with boost::shared_ptr<>)
    class DynamicMemBlock : boost::noncopyable
    {
        byte* m_pData;
        size_t m_size;
        
    public:
        DynamicMemBlock(size_t capacity);
        // copy data from pCopyFrom to his own buffer
        DynamicMemBlock(const void *pCopyFrom, size_t size);
        ~DynamicMemBlock();

        byte* getData() { return m_pData; }
        size_t Size() const { return m_size; }
        void ExtendBlock(size_t addSize);
    };

    // -----------------------------------------------------
    
    // Using for writing data to own buffer.
    // Write methods have the same semantic as IBiWriterStream's methods, 
    // this is provided for making write operations faster for raw memory 
    class MemWriterDynStream : boost::noncopyable
    {
        boost::shared_ptr<DynamicMemBlock> m_buff;

        size_t m_writePos;

        bool m_createNewBufferAfterDetach;
        bool m_detached;

        void ResetBuffer();
        void BufferCheck(size_t size);

    public:
        MemWriterDynStream(size_t capacity = 0, bool createNewBufferAfterDetach = false);
        ~MemWriterDynStream();
        
        // IBiWriterStream semantic
        void Write(byte b); // can throw
        void Write(const void *pSrc, size_t length); // can throw
        
        boost::shared_ptr<DynamicMemBlock> DetachBuffer();
        // After buffer's been detached, this method ca nbe used to set new buffer capacity, 
        // if createNewBufferAfterDetach enabled.
        void setNewBufferCapacity(size_t size); 

        // Data will be unavailable after object destruction
        byte* GetBuffer() const;

        size_t BufferSize() const;
    };
    */
    // -----------------------------------------------------

    /*
    // Using for writing data to given memory segment or to class' own buffer.
    // Write methods have the same semantic as IBiWriterStream's methods, 
    // this is provide to make write operations faster for raw memory 
    class MemWriterStream
    {
        byte *m_pBuffStart;
        size_t m_size;
        size_t m_pos;

        bool m_useOwnBuffer;
        bool m_detached;

        void ExtendBuffer(size_t size);
        void CheckSize(size_t askedSize);

    public:
        // Wrapper for detached data
        // (thread-unsafe)
        class DetachedBuffer
        {
            byte* m_pData;
            size_t m_dataSize;
            bool m_needToBeFreed;
            int *m_pCounter;

            void DeletePtrIfNeed();

        public:
            DetachedBuffer(byte *pData, size_t dataSize, bool needToBeFreed);
            DetachedBuffer(const DetachedBuffer &other);
            ~DetachedBuffer();

            DetachedBuffer& operator=(const DetachedBuffer& other);

            byte* get() { return m_pData; }
            size_t Size() const { return m_dataSize; }
            int UseCount() const { return (m_pCounter != 0)? *m_pCounter : 0; }
        };

    public:
        // Write in the own buffer
        MemWriterStream();
        // Write in the given buffer
        MemWriterStream(void *pStartAddr, size_t size);
        // Clear own buffer if !detached
        ~MemWriterStream();

        // IBiWriterStream semantic
        void Write(byte b); // can throw
        void Write(void *pSrc, size_t length); // can throw

        // Write all buffer to pDest (even if buffer's size > writen data's size).
        // Data will be unavailable after object destruction if using our own buffer
        byte* GetBuffer() const;
        // void GetBuffer(void *pDest) const;

        // Data will be available after object destruction.
        // If using own buffer, buffer will be freed, size and write position set to begin.
        DetachedBuffer DetachBuffer();

        // Size of GetBuffer()
        size_t BufferSize() const { return m_size; }

        static void MemWriterStreamTest();
    };
    */

} // namespace Utils

#endif
