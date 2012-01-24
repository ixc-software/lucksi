#include "stdafx.h"
#include "DataPacket.h"

// ---------------------------------------------

namespace TdmMng
{	    
    DataPacket::DataPacket() : m_pBuff(0)
    {
    }

    DataPacket::DataPacket( Utils::BidirBuffer* pBuff ) : m_pBuff(pBuff)
    {

    }

    DataPacket::~DataPacket()
    {
        Clear();
    }

    BidirBuffer& DataPacket::Buffer()
    {
        ESS_ASSERT(!Empty());
        return *m_pBuff;
    }

    bool DataPacket::Empty() const
    {
        return (m_pBuff == 0);
    }

    void DataPacket::WriteTo( Platform::byte timeSlot, ITdmWriteWrapper &block )
    {
        ESS_ASSERT(!Empty() && "Forget DetachFrom()?");                      
        ESS_ASSERT (block.BlockSize() >= m_pBuff->Size()); // проверить справедливость утверждения!!!            

        // copy all data from buffer to block
        block.Write(timeSlot, 0, *m_pBuff, m_pBuff->Size());
    }

    void DataPacket::Clear()
    {
        delete m_pBuff;
        m_pBuff = 0;
    }

    Utils::BidirBuffer* DataPacket::DetachBuff()
    {
        ESS_ASSERT(!Empty());
        Utils::BidirBuffer* rez = m_pBuff;
        m_pBuff = 0;
        return rez;
    }

    void DataPacket::DetachFrom( DataPacket& other )
    {
        Clear();
        m_pBuff = other.DetachBuff();
    }
}  // namespace TdmMng

