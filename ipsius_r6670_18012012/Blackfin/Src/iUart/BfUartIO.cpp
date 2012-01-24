#include "stdafx.h"

#include <drivers/adi_dev.h>
#include <drivers/uart/adi_uart.h>
#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "BfDev/InterruptLocker.h"
#include "BfDev/AdiDevice.h"
#include "BfDev/AdiDeviceCommand.h"
#include "UartReturnCode.h"
#include "BfUartProfile.h"
#include "BfUart.h"
#include "BfUartIO.h"

namespace iUart
{
	using namespace BfDev;

	BfUartIO::BfUartIO(AdiDevice &device, 
		const BfUartProfile &profile,
		ILogForUart *log) :
		m_device(device),
		m_storage(profile.m_countBuffers, profile.m_sizeBuffer),
		m_inboundBuffers(profile.m_countBuffers, false),
		m_outboundBuffers(profile.m_countBuffers, false),
		m_sizeInboundBuffer(0),
		m_readUserPos(0),
		m_readError(READ_IS_OK),
		m_log(log)
	{
	}

	//--------------------------------------------------------------
	
	void BfUartIO::BufferProcessed(UartBufferItem *buffer)
	{
		if(m_readData.CurrentItem() == buffer)
		{
			m_readData.BufferProcessed(buffer);
			Statistic().m_packReceived++;
			Statistic().m_byteReceived += buffer->ElementCount();
			m_sizeInboundBuffer += buffer->ElementCount();
			m_inboundBuffers.Push(buffer);

			StartReading();
			return;
		}

		if(m_writeData.CurrentItem() == buffer)
		{
		
			m_writeData.BufferProcessed(buffer);			
						
			Statistic().m_packSend++;
			Statistic().m_byteSend += buffer->ElementCount();

			m_storage.Free(buffer);
			
			StartWriting();
			return;
		}

		ESS_HALT("Unknown buffer");
	}

	//--------------------------------------------------------------

	void BfUartIO::ErrorInInterrupt(Platform::dword code)
	{
		switch(code)
		{
			case ADI_UART_EVENT_BREAK_INTERRUPT:
				m_readError = EVENT_BREAK_INTERRUPT;
				break; 
			case ADI_UART_EVENT_FRAMING_ERROR:
				m_readError = EVENT_FRAMING_ERROR;
				break; 
			case ADI_UART_EVENT_PARITY_ERROR:
				m_readError = EVENT_PARITY_ERROR;
				break; 
			case ADI_UART_EVENT_OVERRUN_ERROR:
				m_readError = EVENT_OVERRUN_ERROR;
				break; 
			default:
				ESS_HALT("Unknown event");
		}
	}

	//--------------------------------------------------------------
	
	std::string BfUartIO::ResolveError(ErrorType readError)
	{
		switch(readError)
		{
		case EVENT_BREAK_INTERRUPT:
			return "EVENT_BREAK_INTERRUPT";
		case EVENT_FRAMING_ERROR:
			return "EVENT_FRAMING_ERROR";
		case EVENT_PARITY_ERROR:
			return "EVENT_PARITY_ERROR";
		case EVENT_OVERRUN_ERROR:
			return "EVENT_OVERRUN_ERROR";
		case NOT_FREE_BLOCK:
			return "NOT_FREE_BLOCK";
		}
		return "";
	
	}
    
	//--------------------------------------------------------------

	int BfUartIO::AvailableForRead() const
	{
		InterruptLocker interruptLock;		
		return m_sizeInboundBuffer + InboundProcessedElementCount();
	}

    int BfUartIO::AvailableForWrite() const
    {
		InterruptLocker interruptLock;		
        return m_storage.CountFreeBuffers();
    }

    //--------------------------------------------------------------

    void BfUartIO::ResetInboundData()
    {
        InterruptLocker interruptLock;

        CompleteInboundBuffer();
        m_readData.Clear();

        m_readUserPos = 0;		
        while(m_inboundBuffers.Size())
        {
            m_storage.Free(m_inboundBuffers.Pop());
        }
    }

    //--------------------------------------------------------------

    void BfUartIO::ResetOutboundData()
    {
        InterruptLocker interruptLock;

        CompleteOutboundBuffer();
        m_writeData.Clear();

        while(m_outboundBuffers.Size())
        {
            m_storage.Free(m_outboundBuffers.Pop());
        }
    }

	//--------------------------------------------------------------

	int BfUartIO::ReadImpl(void *data, int size, bool throwInd)
	{
		if(m_readError != READ_IS_OK)
		{
			ErrorType temp = m_readError;
			m_readError = READ_IS_OK;
			if(temp == NOT_FREE_BLOCK) SafeStartReading();
			if(throwInd) ESS_THROW_MSG(UartIoReadError, ResolveError(temp));
			return 0;
		}		
		if(size == 0) return 0;				

		int sizeInboundBuffer = 0;
		int uartSize = 0;
		{
			InterruptLocker interruptLock;		
			sizeInboundBuffer = m_sizeInboundBuffer;
			uartSize = InboundProcessedElementCount();
		}
		
		if(sizeInboundBuffer + uartSize == 0) return 0;
		
		if(size > sizeInboundBuffer)
		{
			if(size > sizeInboundBuffer + uartSize)  size = sizeInboundBuffer + uartSize;
			CompleteInboundBuffer();
		}
		
		int result = 0;
		while(result < size) 
		{
			UartBufferItem *item = 0;
			{
				InterruptLocker interruptLock;		
				if(m_inboundBuffers.IsEmpty()) break;
				item = m_inboundBuffers.Head();
			}
			
			ESS_ASSERT(item != 0);
						
			int movedSize = 
				item->MoveToFromPos(m_readUserPos, static_cast<byte*>(data) + result, size - result);
				
			m_readUserPos += movedSize;
			result += movedSize;
			
			if(m_readUserPos == item->ElementCount()) 
			{
				InterruptLocker interruptLock;				
				m_storage.Free(m_inboundBuffers.Pop());
				m_readUserPos = 0;
			}
		}
		{
			InterruptLocker interruptLock;
			m_sizeInboundBuffer -= result;		
		}
		return result;
	}

	//--------------------------------------------------------------

	int BfUartIO::WriteImpl(const void *data, int size )
	{

		int writedSize = 0;
				
		UartBufferItem *currentItem = 0;
		
	    while(writedSize < size)
	    {
	    	{
				InterruptLocker interruptLock;
	   			currentItem = m_storage.AllocForWriting();
	    	}
	    	
	    	if(currentItem == 0) break; // переполнения буфера, записываем то что есть 

		    int currentPos = 
		    	currentItem->WriteFromPos(static_cast<const byte*>(data) + writedSize, size - writedSize, 0);
		    
		    writedSize += currentPos;
		    
		    if(currentPos == currentItem->MaxElementCount())
		    {
				InterruptLocker interruptLock;		    	
				m_outboundBuffers.Push(currentItem);
				currentItem = 0;
		    }
	    }
	    
	    {
			InterruptLocker interruptLock;
	    	if(currentItem) m_outboundBuffers.Push(currentItem);
			StartWriting();
	    }
        return writedSize;
	}

	//--------------------------------------------------------------

	void BfUartIO::StartReading()
	{
		if(m_readError != READ_IS_OK) return;		

		UartBufferItem *item = m_storage.AllocForReading();
		if(item == 0) 
		{
			m_readError = NOT_FREE_BLOCK;
			return;
		}
		m_readData.SetData(item);
		m_device.Read(m_readData.AdiBuffer());			
	}

    //--------------------------------------------------------------

    void BfUartIO::SafeStartReading()
    { 
		InterruptLocker interruptLock;
        
        StartReading();
    }

	//--------------------------------------------------------------

	void BfUartIO::StartWriting()
	{
		if (m_writeData.CurrentItem() || m_outboundBuffers.IsEmpty()) return;

		m_writeData.SetData(m_outboundBuffers.Pop());
		m_device.Write(m_writeData.AdiBuffer());		
	}

    //--------------------------------------------------------------

    void BfUartIO::SafeStartWriting()
    {
		InterruptLocker interruptLock;
        
        StartWriting();
    }

    //--------------------------------------------------------------

    void BfUartIO::CompleteInboundBuffer()
	{
		AdiDeviceCommand command(ADI_UART_CMD_COMPLETE_INBOUND_BUFFER);
		m_device.RunCommand(command);
	}
	
    //--------------------------------------------------------------

    void BfUartIO::CompleteOutboundBuffer()
	{
		AdiDeviceCommand command(ADI_UART_CMD_COMPLETE_OUTBOUND_BUFFER);
		m_device.RunCommand(command);
	}
	
    //--------------------------------------------------------------

    u32 BfUartIO::InboundProcessedElementCount() const
	{
		u32 size = 0;		
		AdiDeviceCommand command(ADI_UART_CMD_GET_INBOUND_PROCESSED_ELEMENT_COUNT, &size);
        return (m_device.RunCommand(command).isSuccess()) ? size : 0;
	}
    
	//--------------------------------------------------------------

    const UartStatistics &BfUartIO::Statistic() const
    {
    	return m_stat;
    }

	//--------------------------------------------------------------

    UartStatistics &BfUartIO::Statistic()
    {
    	return m_stat;
    }

	//--------------------------------------------------------------

	BfUartIO::ProcessedData::ProcessedData()
	{
        Clear();
	};
	
	//--------------------------------------------------------------

	void BfUartIO::ProcessedData::Clear()
	{
		m_currentItem = 0;
        m_adiBuffer.Data              = 0;
        m_adiBuffer.ElementCount      = 0;
        m_adiBuffer.ElementWidth      = 1;
        m_adiBuffer.CallbackParameter = 0;
        m_adiBuffer.ProcessedFlag     = FALSE;
        m_adiBuffer.ProcessedElementCount = 0;
        m_adiBuffer.pNext             = 0;
        m_adiBuffer.pAdditionalInfo   = 0;
	};
	
	//--------------------------------------------------------------
	
	void BfUartIO::ProcessedData::BufferProcessed(UartBufferItem *buffer)
	{
		ESS_ASSERT(m_currentItem && m_currentItem == buffer);
				
		buffer->UpdateFromAdiBuffer(m_adiBuffer);
		Clear();	
	
	}
	
	//--------------------------------------------------------------
	
	void BfUartIO::ProcessedData::SetData(UartBufferItem *item)
	{
		ESS_ASSERT(!m_currentItem && item);
		
    	m_currentItem = item;
		
		m_currentItem->FillAdiBuffer(m_adiBuffer);		
	};
	

} // namespace BfDev

