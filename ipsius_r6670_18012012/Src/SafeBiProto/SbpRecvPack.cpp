
#include "stdafx.h"
#include "SbpRecvPack.h"
#include "SbpFrameHeader.h"
#include "SbpCRC.h"
#include "SbpGetTypeInfo.h"

namespace SBProto
{
    // SbpRecvPack impl

    SbpRecvPack::SbpRecvPack(const PacketHeader &header) 
    : m_header(header), m_body(m_header.getDataLength()), m_bodyWriter(m_body),
        m_needBodyDataSize(m_header.getDataLength()), m_complete(false), 
        m_currFrameIndex(0)
    {
        if (m_header.getDataLength() != 0) return;

        m_complete = true;
        // if (m_header.getCRC() == 0) m_crcCheckOk = true;
    }

    // ----------------------------------------------------------

    SbpRecvPack::~SbpRecvPack()
    {
        /*
        if (!IsComplete()) return;
        if (EoF()) return;

        ESS_HALT("Not all data were read");
        */
    }

    // ----------------------------------------------------------
    
    bool SbpRecvPack::AddToPacketBody(const void *pData, size_t size, size_t &addedSize) // can throw
    {
        ESS_ASSERT(pData != 0);
        ESS_ASSERT(size != 0);
        ESS_ASSERT(!m_complete);

        addedSize = (m_needBodyDataSize > size)? size : m_needBodyDataSize;
        m_bodyWriter.WriteData(pData, addedSize);

        m_needBodyDataSize -= addedSize;
        if (m_needBodyDataSize != 0) return m_complete;

        // check CRC
        CRC realCRC(m_body.GetBuffer(), 0, m_body.BufferSize());
        if (m_header.getCRC() != realCRC.get()) ESS_THROW(BadCRC);

        // make packet info
        try
        {
            m_packInfo.reset(new SbpPackInfo(m_header, 
                                             m_body.GetBuffer(), 
                                             m_body.BufferSize()));
        }
        catch(SbpPackInfo::InvalidDataTypesInPacket &e) { ESS_THROW(BadDataType); }
        
        m_complete = true;
        
        return m_complete;
    }

    // ----------------------------------------------------------

    const PacketHeader& SbpRecvPack::Header() const
    {
        ESS_ASSERT(IsComplete());
        
        return m_header;
    }

    // ----------------------------------------------------------

    size_t SbpRecvPack::Count() const
    {
        ESS_ASSERT(IsComplete());
        
        return m_packInfo->Count();
    }

    // ----------------------------------------------------------
    
    Frame& SbpRecvPack::operator[](size_t index) const
    {
        ESS_ASSERT(IsComplete());

        return (*m_packInfo)[index];
    }

    // ----------------------------------------------------------

    Frame& SbpRecvPack::Curr() const
    {
        return this->operator[](m_currFrameIndex);
    }

    // ----------------------------------------------------------
    
    bool SbpRecvPack::Next()
    {
        ESS_ASSERT(IsComplete());
        
        if (m_currFrameIndex == (m_packInfo->Count() - 1)) return false;
        
        ++m_currFrameIndex;
        return true;
    }

    // ----------------------------------------------------------

    std::string SbpRecvPack::ToString() const
    {
        ESS_ASSERT(IsComplete());
        
        std::string res("Received packet begin:");
        res += "\n";
        res += m_packInfo->ConvertToString();
        res += "Received packet end.";
        res += "\n";
        
        return res;
    }

    // ----------------------------------------------------------
    
    SbpPackInfo& SbpRecvPack::PackInfo() const
    {
        ESS_ASSERT(IsComplete());
        
        return *m_packInfo;
    }
    
} // namespace SBProto
