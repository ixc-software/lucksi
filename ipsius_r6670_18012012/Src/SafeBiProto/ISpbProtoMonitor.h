
#ifndef __ISPBPROTOMONITOR__
#define __ISPBPROTOMONITOR__

#include "Utils/IBasicInterface.h"
#include "Utils/MemWriterStream.h"

namespace SBProto
{
    class SbpPackInfo;
    
    // Using for monitoring class SafeBiProto
    class ISpbProtoMonitor : public Utils::IBasicInterface
    {
    public:
        virtual void OnSpbPacketIn(const SbpPackInfo &data) = 0;
        virtual void OnSpbPacketOut(const Utils::MemWriterDynStream &header, const Utils::MemWriterDynStream &body) = 0;
        virtual void OnRawDataRecv(const void *pData, size_t size) = 0;
    };

} // namespace SBProto

#endif


