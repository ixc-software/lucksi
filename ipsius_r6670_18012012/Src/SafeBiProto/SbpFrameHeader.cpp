
#include "stdafx.h"
#include "SbpFrameHeader.h"
#include "Utils/MemReaderStream.h"
#include "Utils/BinaryReader.h"
#include "SbpFrame.h"
#include "SbpGetTypeInfo.h"


namespace SBProto
{
    // FrameHeaderParser impl
    
    // Returns true if complete
    bool FrameHeaderParser::Add(byte b, bool &dataTypeIsInvalid)
    {
        ESS_ASSERT(!m_len.IsComplete());
        dataTypeIsInvalid = false;

        ++m_headerLength;

        if (!m_dataTypeAdded)
        {
            if (!IsValidTypeInfoDesc(b)) 
            {
                dataTypeIsInvalid = true;
                return m_len.IsComplete();
            }
            
            m_dataType = b;
            m_dataTypeAdded = true;

            TypeInfo info = m_table.Find(static_cast<TypeInfoDesc>(m_dataType));
            if (!info.IsDynamic) m_len.Add(info.TypeSize);
        }
        else m_len.Add(b);
        
        return m_len.IsComplete();
    }

    // -----------------------------------------------------------------

    void FrameHeaderParser::Reset()
    {
        m_dataType = 0;
        m_dataTypeAdded = false;
        m_len.Reset();
    }

    // -----------------------------------------------------------------
    
    TypeInfoDesc FrameHeaderParser::getDataType() const 
    {
        ESS_ASSERT(m_dataTypeAdded);
        return static_cast<TypeInfoDesc>(m_dataType);
    }

    // -----------------------------------------------------------------
    
    dword FrameHeaderParser::getLength() const
    {
        ESS_ASSERT(m_len.IsComplete());
        return m_len.getLength();
    }

    // -----------------------------------------------------------------
    
    byte FrameHeaderParser::getHeaderLength() const
    {
        ESS_ASSERT(m_len.IsComplete());
        return m_headerLength;
    }
    
} // namespace SBProto
