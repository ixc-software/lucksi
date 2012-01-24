#include "stdafx.h"
#include "Utils\ErrorsSubsystem.h"

#include "BfUartBuffers.h"

namespace iUart
{
    UartBufferItem::UartBufferItem(const UartBufferItem &src) :
        m_data(src.m_data.size())
    {
        Clear();	
	}

	//-------------------------------------------------

	UartBufferItem::UartBufferItem(int size) : m_data(size)
    {
        Clear();
    }

	//-------------------------------------------------
	
    void UartBufferItem::FillAdiBuffer(ADI_DEV_1D_BUFFER &adiBuffer) 
	{
        adiBuffer.Data              = &m_data[0];
        adiBuffer.ElementCount      = m_elementCount;
        adiBuffer.ElementWidth      = 1;
        adiBuffer.CallbackParameter = this;
        adiBuffer.ProcessedFlag     = FALSE;
        adiBuffer.ProcessedElementCount = 0;
        adiBuffer.pNext             = 0;
        adiBuffer.pAdditionalInfo   = 0;
	}        
	
	//-------------------------------------------------
    
    void UartBufferItem::UpdateFromAdiBuffer(const ADI_DEV_1D_BUFFER &adiBuffer)
    {
    	ESS_ASSERT(adiBuffer.Data == &m_data[0] &&
		adiBuffer.CallbackParameter == this && 	
		m_data.size() >= adiBuffer.ProcessedElementCount);
    	 	
    	m_elementCount = adiBuffer.ProcessedElementCount;
    }
        
	//-------------------------------------------------

	int UartBufferItem::MoveToFromPos(int sourcePos, byte *dst, int size)
	{
		if(!m_elementCount)	return 0;
		
        ESS_ASSERT(sourcePos < m_elementCount);
		int result = m_elementCount - sourcePos;

		if(size < result)	result = size;
		
		copy(m_data.begin() + sourcePos, m_data.begin() + sourcePos + result, dst);

		return result;
	}

	//-------------------------------------------------

	int UartBufferItem::WriteFromPos(const byte *src, int size, int dstPos)
	{
        int result = m_data.size() - dstPos;

        if (result == 0)    return 0;

		ESS_ASSERT(result > 0);

		if(size < result)  result = size;

		copy(src, src + result, m_data.begin() + dstPos);

		m_elementCount += result;
		return result;
	}

	//-------------------------------------------------

	UartBuffersStorage::UartBuffersStorage(int countBuffer, int sizeBuffer) : 
    	m_storage(countBuffer, sizeBuffer),
    	m_freeBuffers(countBuffer, false)
    {
        for (Storage::iterator i = m_storage.begin();
            i!=m_storage.end(); ++i)
        {
            m_freeBuffers.Push(&(*i));
        }
    }

	//-------------------------------------------------

    UartBufferItem *UartBuffersStorage::AllocForWriting()
    {
        UartBufferItem *item = Alloc();
        
        if(item) item->PreparationForWriting();
        
        return item;
    }

	//-------------------------------------------------

    UartBufferItem *UartBuffersStorage::AllocForReading()
    {
        UartBufferItem *item = Alloc();
        
        if(item) item->PreparationForReading();
        
        return item;
    }

	//-------------------------------------------------

	void UartBuffersStorage::Free(UartBufferItem *item)
    {
		ESS_ASSERT(item);

		item->Clear();
		
        m_freeBuffers.Push(item);
    }

};


