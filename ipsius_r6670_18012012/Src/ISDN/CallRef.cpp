#include "stdafx.h"
#include "CallRef.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IntToString.h"
#include "DssWarning.h"

namespace ISDN
{

    CallRef::CallRef(int value, dword maxVal): m_value(value), m_Flag(true), m_maxValue(maxVal)
    {
        ESS_ASSERT( m_value >= 0 && m_value <= maxVal);
    }

    //---------------------------------------------------------------------------

    // Extract CallRef from L2Packet
    CallRef::CallRef(QVector<byte> l2data, IIsdnAlloc& alloc) : m_value(0)
    {
        byte octet = l2data.at(1);
        if (!octet)
        {
            shared_ptr<DssWarning> pWarning(UnsupportedBroadcast::Create( alloc ));
            ESS_THROW_T(DssWarningExcept, pWarning);
        }
        int crv_len = octet & 0x0f;
        if (crv_len > 3)
        {
            shared_ptr<DssWarning> pWarning( PacketParseError::Create(alloc, l2data) );
            ESS_THROW_T(DssWarningExcept, pWarning);
        }

        octet = l2data.at(2); //crv[0]
        m_Flag = (octet & 0x80); 

        m_value = octet & 0x7f; // обнуляем флаг             
        m_maxValue = 0x7f; // for crv_len = 1 
        for(int i = 1; i < crv_len ; ++i)
        {   
            // байты ближние к началу пакета пишутся в старшие байта m_value
            m_value = m_value << 8;
            m_value |= l2data.at(2+i);    

            // set len format
            m_maxValue = (m_maxValue << 8) | 0xff;                 
        }        
    }

    //---------------------------------------------------------------------------

    bool CallRef::operator == (const CallRef& rhs) const
    {
        return ( m_value == rhs.m_value && m_Flag == rhs.m_Flag );
    }

    //---------------------------------------------------------------------------

    bool CallRef::operator != (const CallRef& rhs) const
    {
        return !operator==(rhs);
    }

    //---------------------------------------------------------------------------

    std::string CallRef::ToString(bool onlyValue) const
    {
		if(onlyValue) return Utils::IntToHexString(m_value | (m_Flag ? 0 : 0x80), true, 2);

		std::ostringstream out;
		out << Utils::IntToHexString(m_value | (m_Flag ? 0 : 0x80), true, 2);
        if (!onlyValue) out << (m_Flag ? " (assigned on this side)" : " (assigned on destination side)");
        return out.str();
    }

    //---------------------------------------------------------------------------

    void CallRef::AddBackCRef (std::vector<byte>& out) const
    {       
        byte flag; int bytecount = 0;
        out.push_back(0);
        //std::vector<byte>::iterator first = out.end() - 1;            
        flag = (m_Flag) ? 0 : 0x80; // инверсия флага при отправке
        if (m_maxValue > 0xffff)   // 3-byte crv
        {
            out.push_back( (m_value >> 16) & 0xff );
            ++bytecount;
        }
        if (m_maxValue > 0x7f)     // 2-byte crv            
        {
            out.push_back( (m_value >> 8) & 0xff );
            ++bytecount;
        }
        out.push_back((m_value) & 0xff);
        ++bytecount;

        *( out.end() - bytecount ) |= flag;          
        *( out.end() - bytecount - 1 ) = bytecount;
    };

    //---------------------------------------------------------------------------

    dword CallRef::GetValue() const 
    {
        return m_value;
    }

    //---------------------------------------------------------------------------

    void CallRef::IncrValue()
    {
        ++m_value;
    }

    //---------------------------------------------------------------------------

    void CallRef::ReplaceValue(dword value)
    {        
        ESS_ASSERT(value >= 0);
        m_value = value;
    }

    bool CallRef::IsGlobal() const
    {
        return m_value == 0;
    }
} // ISDN




