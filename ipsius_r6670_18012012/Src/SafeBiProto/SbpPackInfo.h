
#ifndef __SBPPACKINFO__
#define __SBPPACKINFO__

// SbpPackInfo.h

#include "SbpPacketHeader.h"
#include "SbpFrame.h"
#include "SbpException.h"

#include "Utils/MemWriterStream.h"
#include "Utils/ManagedList.h"

namespace SBProto
{
    // Split get buffer to frames: 
    // DataPtr() in frame points to the given buffer
    class SbpPackInfo :
        boost::noncopyable
    {
        PacketHeader m_header;
        Utils::ManagedList<Frame> m_frames;

        static PacketHeader MakeHeader(const Utils::MemWriterDynStream &header);
        
    public:

        ESS_TYPEDEF(InvalidDataTypesInPacket);
        
        // SbpPackInfo(const byte *pBody, size_t bodySize);
        SbpPackInfo(const Utils::MemWriterDynStream &header, const Utils::MemWriterDynStream &body); // can throw
        SbpPackInfo(const PacketHeader &header, const byte *pBody, size_t bodySize); // can throw

        // frames count
        size_t Count() const;
        Frame& operator[](size_t index) const;

        const PacketHeader& Header() const { return m_header; }

        std::string ConvertToString() const;
    };
    
} // namespace SBProto

#endif
