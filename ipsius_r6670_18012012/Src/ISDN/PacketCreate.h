#ifndef PACKETCREATE_H
#define PACKETCREATE_H

#include "IIsdnAlloc.h"

namespace ISDN
{

    // формирователь пакетов второго уровня
    class PacketCreate : boost::noncopyable
    {
    public:

        PacketCreate(IIsdnAlloc *pAlloc):
          m_pAlloc(pAlloc) {}

          template<typename PacketType>
          PacketType* CreatePacket()
          {
              return new( m_pAlloc)PacketType(m_pAlloc);
          }

    private:

        IIsdnAlloc* m_pAlloc;

    };

} // namespace ISDN

#endif

