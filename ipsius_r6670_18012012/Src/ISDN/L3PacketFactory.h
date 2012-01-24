#ifndef L3PACKETFACTORY_H
#define L3PACKETFACTORY_H

#include "stdafx.h"
#include "L3MessageType.h"
#include "Platform/PlatformTypes.h"

namespace ISDN
{    
    //class IIsdnL3Internal;
    //class L3PacketFactory;
    //class IIsdnAlloc;
    class IsdnInfra;
    class IeFactory;    
    class L3Packet;
    using Platform::byte;

    // фабрика пакетов 3уровня
    class L3PacketFactory : boost::noncopyable
    {
        class IMetaInterface;

        template<class T>
        class MetaType;

        //typedef std::map<L3PacketMessgeType, IMetaInterface*> PacketMap;         
        typedef boost::ptr_map<L3MessgeType::Type, IMetaInterface> PacketMap;

        //fields
        PacketMap m_creators;
        IsdnInfra& m_infra;        
        IeFactory& m_ieFactory;

        static void RegPacketTypes(L3PacketFactory& factory);

    public:

        L3PacketFactory(IsdnInfra& infra, IeFactory& ieFactory);

        ~L3PacketFactory();

        template<class T>
        void RegPack();

        L3Packet* CreateL3Pack(QVector<byte> l2data);
        
    };

    /*
        Функция регистрации определяется в .cpp-файле.
        Это возможно тк инстанцирование этой фции происходит в h-файле заголовков типов для этой фабрики
        который включается cpp-файл фабрики.
    */

} // ISDN

#endif

