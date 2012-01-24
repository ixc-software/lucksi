
#ifndef __SBP_PACKETHEADER__
#define __SBP_PACKETHEADER__

// SbpPacketHeader.h

#include "SbpUtils.h"
#include "Utils/MemWriterStream.h"
#include "Utils/BinaryWriter.h"

namespace SBProto
{
    // SbpPacket header
    class PacketHeader
    {
    public:
        enum SbpPacketType
        {
            PTCommand = 0,
            PTResponce = 1,
            PTInfo = 2,
        };

        enum HeaderConsts
        {
            // [dataType + protoVersion][CRC][length] ---v
            // [byte][word][dword]
            CMaxHeaderSizeBytes = 7,
            CHeaderSizeExceptLengthBytes = 3,
        };

        ESS_TYPEDEF(InvalidPacketType);
        
    public:
        PacketHeader(byte packType, byte protoVersion, word crc, dword dataLen); // can throw

        SbpPacketType getPackType() const;
        byte getProtoVersion() const;
        word getCRC() const;
        dword getDataLength() const;

        // can be called for incorrect packet also
        std::string ToString() const;

        static void WriteHeaderToStream(Utils::MemWriterDynStream &stream,
                                        SbpPacketType type, byte protoVersion, word crc, 
                                        dword dataLen);

    private:
        SbpPacketType PackTypeFromByte(byte b); // can throw
        // byte PackTypeToByte(SbpPacketType type) { return static_cast<byte>(type); }
        
        SbpPacketType m_packType;
        // byte m_packType;
        byte m_protoVersion;
        word m_crc;
        dword m_dataLen;
    };

    // --------------------------------------------------
    
    // Using for creating packet header from incoming data
    class PacketHeaderParser
    {
        SBPUtils::DynamicLengthParser m_lengthParser;
        Utils::MemWriterDynStream m_stream;
        Utils::BinaryWriter<Utils::MemWriterDynStream> m_writer;

        bool IsComplete() const { return m_lengthParser.IsComplete(); }
        
    public:
        PacketHeaderParser();

        // Returns true if complete
        bool Add(byte b);
        void Reset();

        PacketHeader GetHeader() const; // can throw
        // size_t getHeadetSizeBytes() const;
    };

    // -------------------------------------------------

    /*
    // Using to write packet header data to stream
    template<class TWriterStream>
    void PacketHeaderWriter(Utils::BinaryWriter<TWriterStream> &writer,
                            const PacketHeader &header)
    {
        byte packTypeAndProtoVer = (header.getProtoVersion() << 2) 
                                    | (header.getPackType() & 0xff) ;
        writer.WriteByte(packTypeAndProtoVer);
        writer.WriteWord(header.getCRC());
        SBPUtils::DynamicLengthWriter(writer, header.getDataLength());
    }
    */

    
} // namespace SBProto

#endif
