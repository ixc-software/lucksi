
#ifndef __UDPLOGDBSTATUS__
#define __UDPLOGDBSTATUS__


#include "iNet/SocketData.h"
#include "UlvTypes.h"

namespace Ulv
{
    class UdpLogDBStatus // copyable
    {
        ddword m_recvPacksCount;
        ddword m_recvBytes;
        ddword m_invalidPacksCount;

        ddword m_droppedPackCount;
        ddword m_dropTimes;

        dword m_packsInDBCount;
        dword m_filteredCount;

        ddword m_recTimes;
        ddword m_recFileSize;

        ddword m_refreshMsgCountAll;
        ddword m_refreshMsgCountProcessed;

        ddword m_renderImgCount;
        ddword m_renderImgAgvTimeMs;

    public:
        UdpLogDBStatus() :
            m_recvPacksCount(0), m_recvBytes(0), m_invalidPacksCount(0),
            m_droppedPackCount(0), m_dropTimes(0),
            m_packsInDBCount(0), m_filteredCount(0),
            m_recTimes(0), m_recFileSize(0),
            m_refreshMsgCountAll(0), m_refreshMsgCountProcessed(0),
            m_renderImgCount(0), m_renderImgAgvTimeMs(0)
        {
        }

        void ReceiveNewPack(int size, bool successfullyParsed)
        {
            ESS_ASSERT(size >= 0);

            m_recvBytes += size;
            ++m_recvPacksCount;
            if (!successfullyParsed) ++m_invalidPacksCount;
        }

        void FilteredCount(dword val)
        {
            ESS_ASSERT(val <= m_recvPacksCount);

            m_filteredCount = val;
        }

        void PacksInDBCount(dword val)
        {
            m_packsInDBCount = val;
        }

        void DroppedCount(ddword packs, ddword times)
        {
            m_droppedPackCount = packs;
            m_dropTimes = times;
        }

        void Recording(ddword times, ddword fileSize)
        {
            m_recTimes = times;
            m_recFileSize = fileSize;
        }

        void RefreshMsgCount(ddword val)
        {
            m_refreshMsgCountAll = val;
            ++m_refreshMsgCountProcessed;
        }

        void Rendering(ddword count, ddword avgTimeMs)
        {
            m_renderImgCount = count;
            m_renderImgAgvTimeMs = avgTimeMs;
        }

        ddword ReceivedPacksCount() const { return m_recvPacksCount; }
        ddword ReceivedInvalidPacksCount() const { return m_invalidPacksCount; }
        ddword ReceivedBytes() const { return m_recvBytes; }

        dword FilteredCount() const { return m_filteredCount; }
        dword PacksInDBCount() const { return m_packsInDBCount; }

        ddword DroppedPackCount() const { return m_droppedPackCount;}
        ddword DropTimes() const { return m_dropTimes; }

        ddword RecTimes() const { return m_recTimes; }
        ddword RecFileSize() const { return m_recFileSize; }

        ddword RefreshMsgCountAll() const { return m_refreshMsgCountAll; }
        ddword RefreshMsgCountProcessed() const { return m_refreshMsgCountProcessed; }

        ddword RenderingImgCount() { return m_renderImgCount; }
        ddword RenderingImgAgvTimeMs() { return m_renderImgAgvTimeMs; }

        void Clear()
        {
            // m_recvPacksCount = 0;
            // m_invalidPacksCount = 0;
            // m_recvBytes = 0;
            // m_droppedPackCount = 0;
            m_packsInDBCount = 0;
            m_filteredCount = 0;
        }        
    };
    
} // namespace Ulv

#endif
