#include "stdafx.h"
#include "Utils/BaseSpecAllocated.h"
#include "IeFactory.h"
#include "L3Packet.h"
#include "IIsdnL3Internal.h"
#include "L3PacketFactory.h"
#include "L3PacketTypes.h"


namespace ISDN
{
    class L3PacketFactory::IMetaInterface
        : Utils::IBasicInterface
    {
    public:

        virtual L3Packet* CreateInstance(QVector<byte> l2data, IsdnInfra& infra, IeFactory& ieFactory) = 0;

    };

	// -------------------------------------------------------------------------------------

    template<class T>
    class L3PacketFactory::MetaType : public IMetaInterface
    {
    public:

        L3Packet* CreateInstance(QVector<byte> l2data, IsdnInfra& infra, IeFactory& ieFactory)  // override
        {
            return new( infra ) T(infra, ieFactory, l2data);
        }
    };

	// -------------------------------------------------------------------------------------

    L3PacketFactory::L3PacketFactory(IsdnInfra& infra, IeFactory& ieFactory) 
        : m_infra(infra), m_ieFactory(ieFactory)
    {        
        RegPacketTypes(*this);
	}

	// -------------------------------------------------------------------------------------

    // Ќеобходим дл€ переноса точки инстанцировани€ деструктора ptr_map в этот 
    // compile unit тк IMetaInterface определен тут.
    L3PacketFactory::~L3PacketFactory() {}

    template<class T>
    void L3PacketFactory::RegPack()
    {            
		L3MessgeType::Type mt = T::GetMsgType();
        bool res = m_creators.insert( mt, new MetaType<T> ).second;
        
        ESS_ASSERT(res && "Reregistration duplicate type");
    }

	// -------------------------------------------------------------------------------------

    L3Packet* L3PacketFactory::CreateL3Pack(QVector<byte> l2data)
    {
        //ESS_ASSERT(0 && "Prb");

        if (l2data.size() < 3) 
        {
            boost::shared_ptr<DssWarning> err(ShortPacket::Create(m_infra, l2data) );
            ESS_THROW_T(DssWarningExcept, err);
        }

		L3MessgeType::Type mt = L3Packet::ExtractMessageType(m_infra, l2data);
			
        PacketMap::iterator i = m_creators.find(mt);                
        
        if (i == m_creators.end())
        {
            boost::shared_ptr<DssWarning> err(PackWithUnknounMt::Create(m_infra, l2data) );
            ESS_THROW_T(DssWarningExcept, err);
        }

        return i->second->CreateInstance(l2data, m_infra, m_ieFactory);// == new

    }

	// -------------------------------------------------------------------------------------

    void L3PacketFactory::RegPacketTypes(L3PacketFactory& factory)
    {
        RegL3Packets(factory);
    }

} // ISDN

