
#ifndef __SBP_FRAMEHEADER__
#define __SBP_FRAMEHEADER__

// SbpFrameHeader.h

#include "SbpUtils.h"
#include "SbpGetTypeInfo.h"

namespace SBProto
{
    // Using for making packet header from incoming data
    class FrameHeaderParser
    {
        TypeInfoTable m_table;
        byte m_dataType;
        SBPUtils::DynamicLengthParser m_len;
        bool m_dataTypeAdded;

        byte m_headerLength;
        
    public:
        FrameHeaderParser() : m_dataType(0), m_dataTypeAdded(false), m_headerLength(0)
        {
        }

        // Returns true if complete
        bool Add(byte b, bool &dataTypeIsInvalid);
        void Reset();
        
        TypeInfoDesc getDataType() const;
        dword getLength() const;

        byte getHeaderLength() const;
    };

    // -----------------------------------------------------------------
    // Using to write header data to stream.
    // Convert dataLength to dynamic length
    // Parameter "dynDataSize" used only for dynamic data types
    template<class TWriterStream>
    void FrameHeaderWriter(Utils::BinaryWriter<TWriterStream> &writer, 
                           TypeInfo dataType, size_t dynDataSize = 0)
    {
        writer.WriteByte(static_cast<byte>(dataType.TypeID));

        // if static type we don't write data length
        if (!dataType.IsDynamic) return;
        
        SBPUtils::DynamicLengthWriter(writer, dynDataSize);
    }
    
} // namespace SBProto


#endif
