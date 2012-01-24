
#ifndef __SBPRECVPACK__
#define __SBPRECVPACK__

// SbpRecvPack.h

#include "Utils/ErrorsSubsystem.h"
#include "Utils/BinaryReader.h"
#include "Utils/MemWriterStream.h"
#include "Utils/MemReaderStream.h"

#include "SbpPacketHeader.h"
#include "SbpPackInfo.h"

namespace SBProto
{

    class ISbpRecvPack : public Utils::IBasicInterface
    {
    public:
        // returns true if packet complete 
        virtual bool AddToPacketBody(const void *pData, 
                                     size_t size, 
                                     size_t &addedSize) = 0; // can throw
    };

    // ----------------------------------------------------------------

    // Class-wrapper for received data packet.
    class SbpRecvPack:
        public ISbpRecvPack,
        boost::noncopyable
    {
        PacketHeader m_header;
        Utils::MemWriterDynStream m_body;
        Utils::BinaryWriter<Utils::MemWriterDynStream> m_bodyWriter;
        size_t m_needBodyDataSize;
        bool m_complete;

        boost::scoped_ptr<SbpPackInfo> m_packInfo;
        size_t m_currFrameIndex;

        bool IsComplete() const { return m_complete && m_packInfo != 0; }
        static void Error(const std::string &msg);
        
    // ISbpRecvPack impl 
    private:
        friend class SafeBiProto;
                
        // returns true if complete
        bool AddToPacketBody(const void *pData, size_t size, size_t &addedSize); // can throw
        
    public:
        ESS_TYPEDEF(BadCRC);
        ESS_TYPEDEF(BadDataType);
        
        SbpRecvPack(const PacketHeader &header);
        ~SbpRecvPack();
 
        const PacketHeader& Header() const;

        // frames count
        size_t Count() const;
        Frame& operator[](size_t index) const;

        // current frame
        Frame& Curr() const;

        // returns false if Eof
        bool Next();

        std::string ToString() const;

        SbpPackInfo& PackInfo() const;
    };

    
} // namespace SBProto

#endif
