#ifndef __DATAPACKET__
#define __DATAPACKET__

#include "Utils/BidirBuffer.h"


#include "TdmHAL.h" // TdmWriteWrapper

namespace TdmMng
{    
    using Utils::BidirBuffer;    

    // ownership BidirBuffer
    class DataPacket : boost::noncopyable
    {        
        Utils::BidirBuffer *m_pBuff;

    public:
        
        DataPacket(); // Пустой пакет
        DataPacket(Utils::BidirBuffer* pBuff);        

        ~DataPacket();

        BidirBuffer& Buffer();

        bool Empty() const;

        // copying data from packet to block
        void WriteTo(Platform::byte timeSlot, ITdmWriteWrapper &block);

        void Clear();

        // отдает владение буфером
        Utils::BidirBuffer* DetachBuff();

        // забирает владение буфером
        void DetachFrom(DataPacket& other);
    };

		
}  // namespace TdmMng

#endif
