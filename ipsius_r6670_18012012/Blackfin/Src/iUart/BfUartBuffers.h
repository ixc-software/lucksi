#ifndef _BF_UART_BUFFERS_H_
#define _BF_UART_BUFFERS_H_

#include "stdafx.h"
#include "drivers/adi_dev.h"
#include "Platform/PlatformTypes.h"
#include "Utils/FixedFifo.h"


namespace iUart
{
	using Platform::byte;
	class UartBuffersStorage;

	// еденица буферизации Uart
    class UartBufferItem : boost::noncopyable
    {
        typedef std::vector<Platform::byte> Buffer;
        Buffer m_data;
        int m_elementCount;
    public:
        UartBufferItem(const UartBufferItem &);
        UartBufferItem(int size);
		
        bool IsFull() const 
        {
        	return ElementCount() == MaxElementCount();
        }
	    int ElementCount() const
	    {
	        return m_elementCount;
	    }
	    int MaxElementCount() const
	    {
	        return m_data.size();
	    }
        void FillAdiBuffer(ADI_DEV_1D_BUFFER &adiBuffer);
        void UpdateFromAdiBuffer(const ADI_DEV_1D_BUFFER &adiBuffer);        
        
		int MoveToFromPos(int sourcePos, byte *dst, int size);

	    int WriteFromPos(const byte *src, int size, int dstPos);

        const Buffer &Data() const {	return m_data; }
	    void PreparationForReading()
	    {
			m_elementCount = m_data.size();
	    }
	    void PreparationForWriting()
	    {
			m_elementCount = 0;
	    }
		void Clear()
		{
			m_elementCount = 0;
		}
    };

	//-------------------------------------------------

  	typedef Utils::FixedFifo<UartBufferItem*> UartBuffersList;

	//-------------------------------------------------
	// Storage uart buffers

    class UartBuffersStorage : boost::noncopyable
    {
    public:
        UartBuffersStorage(int countBuffer, int sizeBuffer);
        UartBufferItem *AllocForWriting();
        UartBufferItem *AllocForReading();
        void Free(UartBufferItem *item);
        int CountFreeBuffers() const
        {
            return m_freeBuffers.Size();
        }
    private:
		UartBufferItem *Alloc()
	    {
	        return (m_freeBuffers.IsEmpty()) ? 0 : m_freeBuffers.Pop();
	    }
        typedef std::vector<UartBufferItem> Storage;
        Storage m_storage;
        UartBuffersList m_freeBuffers;
    };

};

#endif
