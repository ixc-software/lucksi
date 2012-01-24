
#ifndef __IBIRWSTREAM__
#define __IBIRWSTREAM__

// IBiRWStream.h

#include "Utils/IBasicInterface.h"
#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"

namespace Utils
{
    using Platform::byte;
    
    // Base class for all reader stream's exceptions
    ESS_TYPEDEF(BRStreamException);
    ESS_TYPEDEF_FULL(NotEnoughData, BRStreamException);

    // ------------------------------------------------
    
    // Base interface for all streams using in BinaryReader<>
    class IBiReaderStream : public Utils::IBasicInterface
    {
    public:
        virtual byte ReadByte() = 0;    // can throw
        virtual void Read(void *pDest, size_t length) = 0; // can throw
    };

    // ------------------------------------------------

    // Base class for all writer stream's exceptions
    ESS_TYPEDEF(BWStreamException);
    ESS_TYPEDEF_FULL(NotEnoughMemory, BWStreamException);

    // -----------------------------------------------
    
    // Base interface for all streams using in BinaryWriter<>
    class IBiWriterStream : public Utils::IBasicInterface
    {
    public:
        virtual void Write(byte b) = 0; // can throw
        virtual void Write(const void *pSrc, size_t length) = 0; // can throw
    };
    
} // namespace Utils


#endif
