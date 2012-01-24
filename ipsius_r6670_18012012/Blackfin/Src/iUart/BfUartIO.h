#ifndef _BF_UART_IO_H_
#define _BF_UART_IO_H_

#include "stdafx.h"
#include "iLog/iLogSessionCreator.h"
#include "IUartIO.h"
#include "BfUartBuffers.h"
#include "IBfUart.h"
#include "UartStatistics.h"

namespace BfDev {	class AdiDevice; }
namespace iUart
{
	using Platform::byte;
	struct BfUartProfile;
	class ILogForUart;

    class BfUartIO : boost::noncopyable,
        public IUartIO
    {
    public:	
        BfUartIO(BfDev::AdiDevice &device, 
			const BfUartProfile &profile,
			ILogForUart *log = 0);

		void SafeStartReading();        
		void BufferProcessed(UartBufferItem *buffer);
		void ErrorInInterrupt(Platform::dword code);
    // IBfUartIO
    private:
        int AvailableForRead() const;
        int AvailableForWrite() const;

        void ResetInboundData();
        void ResetOutboundData();

        int ReadImpl(void *data, int size, bool throwInd);
        int WriteImpl(const void *data, int size);
    private:
        void StartReading();
        void StartWriting();
        void SafeStartWriting();
        
        void CompleteInboundBuffer();
        void CompleteOutboundBuffer();
    	u32 InboundProcessedElementCount() const;
        
        const UartStatistics &Statistic() const;
        UartStatistics &Statistic();        
        
    private:
		mutable BfDev::AdiDevice &m_device;
		
		UartStatistics m_stat;
	
		UartBuffersStorage m_storage;

		UartBuffersList m_inboundBuffers;
		UartBuffersList m_outboundBuffers;

		int m_sizeInboundBuffer;
		int m_readUserPos;
	
		class ProcessedData : boost::noncopyable
		{
		public:
			ProcessedData();
			void Clear();
			void BufferProcessed(UartBufferItem *);
			void SetData(UartBufferItem *item);

			ADI_DEV_1D_BUFFER &AdiBuffer() { return  m_adiBuffer; }
            const UartBufferItem *CurrentItem() const
            {
                return m_currentItem;
            }
        private:
			UartBufferItem *m_currentItem;
			ADI_DEV_1D_BUFFER m_adiBuffer;
		};
		bool m_badAllocOnRead;

		enum ErrorType
		{
			READ_IS_OK = 0,
			NOT_FREE_BLOCK,
			EVENT_BREAK_INTERRUPT,
			EVENT_FRAMING_ERROR,
			EVENT_PARITY_ERROR,
			EVENT_OVERRUN_ERROR
		};
		ErrorType m_readError;
		static std::string ResolveError(ErrorType readError);
		ProcessedData m_readData;
		ProcessedData m_writeData;
		ILogForUart *m_log;
    };
	
} // namespace BfDev

#endif

