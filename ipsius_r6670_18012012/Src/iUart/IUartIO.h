#ifndef _I_UART_IO_H_
#define _I_UART_IO_H_

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"

namespace iUart
{
    using Platform::byte;

    // all operations is unblocked
    class IUartIO : public Utils::IBasicInterface
    {
    public: 

        ESS_TYPEDEF(UartIoWriteError);
        ESS_TYPEDEF(UartIoReadError);

        // READ NVI
        bool Read(byte &data, bool throwInd = true)
        {
            return ReadImpl(&data, 1, throwInd) == 1;
        }

        int Read(void *data, int size, bool throwInd = true)
        {
            ESS_ASSERT(size > 0);
            return ReadImpl(data, size, throwInd);
        }

        // WRITE NVI
        int Write(byte data, bool throwInd = true)
        {
            return Write(&data, 1, throwInd);
        }

        int Write(const std::string &data, bool throwInd = true) 
        {
            return Write(reinterpret_cast<const byte*>(&data[0]), data.size(), throwInd);
        }

        int Write(const std::vector<byte> &data, bool throwInd = true) 
        {
            return Write(&data[0], data.size(), throwInd);
        }

        int Write(const std::vector<byte> &data, int size, bool throwInd = true)
        {
		    ESS_ASSERT(data.size() <= size);

            return Write(&data[0], size, throwInd);
        }

        int Write(const void *data, int size, bool throwInd = true)
        {
            ESS_ASSERT(size > 0);

            int result = WriteImpl(data, size);

            if (throwInd && result != size)
            {
                ESS_THROW(UartIoWriteError);
            }

            return result;
        }

        // OTHER OPS
        virtual int AvailableForRead() const = 0;
        virtual int AvailableForWrite() const = 0;

        virtual void ResetInboundData() = 0;
        virtual void ResetOutboundData() = 0;

    private:

        virtual int WriteImpl(const void *data, int size) = 0;
        virtual int ReadImpl(void *data, int size, bool throwInd) = 0;

    };
};
#endif
