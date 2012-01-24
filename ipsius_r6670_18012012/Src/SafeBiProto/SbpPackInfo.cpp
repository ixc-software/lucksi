
#include "stdafx.h"
#include "SbpPackInfo.h"

#include "Utils/MemReaderStream.h"
#include "Utils/BinaryReader.h"


namespace SBProto
{
    // SbpSendPackInfo impl
    /*
    SbpPackInfo::SbpPackInfo(const byte *pData, size_t size)
    {
        Frame::SplitDataToFrames(pData, size, m_frames);
    }
    */

    // ----------------------------------------------------------

    SbpPackInfo::SbpPackInfo(const Utils::MemWriterDynStream &header, 
                             const Utils::MemWriterDynStream &body)
    : m_header(MakeHeader(header))
    {
        bool parsingOk = Frame::SplitDataToFrames(body.GetBuffer(), 
                                                  body.BufferSize(), 
                                                  m_frames);
        
        if (!parsingOk) ESS_THROW(InvalidDataTypesInPacket);
    }

    // ----------------------------------------------------------

    SbpPackInfo::SbpPackInfo(const PacketHeader &header, const byte *pBody, size_t bodySize)
    : m_header(header)
    {
        bool parsingOk = Frame::SplitDataToFrames(pBody, bodySize, m_frames);

        if (!parsingOk) ESS_THROW(InvalidDataTypesInPacket);
    }

    // ----------------------------------------------------------

    PacketHeader SbpPackInfo::MakeHeader(const Utils::MemWriterDynStream &header)
    {
        Utils::MemReaderStream stream(header.GetBuffer(), header.BufferSize());
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);
        
        PacketHeaderParser parser;
        while (!parser.Add(reader.ReadByte()));

        ESS_ASSERT(stream.HasToRead() == 0);
        
        return parser.GetHeader();
    }
    
    // ----------------------------------------------------------

    size_t SbpPackInfo::Count() const
    {
        return m_frames.Size();
    }

    // ----------------------------------------------------------
        
    Frame& SbpPackInfo::operator[](size_t index) const
    {
        if (index >= Count()) ESS_THROW(BadFieldIndex);
        
        return *m_frames[index];
    }

    // ----------------------------------------------------------

    std::string SbpPackInfo::ConvertToString() const
    {
        std::string res(m_header.ToString());
        
        for (size_t i = 0; i < m_frames.Size(); ++i)
        {
            res += m_frames[i]->ConvertToString();
        }

        return res;
    }

    // ----------------------------------------------------------

} // namespace SBProto
