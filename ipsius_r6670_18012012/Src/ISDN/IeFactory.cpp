#include "stdafx.h"
#include "IeFactory.h"
#include "IeTypes.h"


namespace ISDN
{
    class IeFactory::IMetaInterface
        : //public BaseSpecAllocated,
        Utils::IBasicInterface
    {
    public:

        virtual ICommonIe* CreateInstance(const RawData&, IsdnInfra&) = 0;
        virtual const IeId& GetIeId() = 0;

    }; 

    // --------------------------------------------------------------

    template<class T>
    class IeFactory::MetaType : public IMetaInterface
    {
        const IeId m_id;

    public:

        MetaType() : m_id( T::GetId() )
        {
        }

        ICommonIe* CreateInstance(const RawData& raw, IsdnInfra& infra)  // override
        {
            return new(infra) T(raw, infra);
        }           

        const IeId& GetIeId()
        {
            return m_id;
        }

    };

    // --------------------------------------------------------------

    template<class T>
    void IeFactory::RegisterIe()
    {        
        const IeId ieId = T::GetId();        

        // эту нельзя регистрировать
        BOOST_STATIC_ASSERT( ! (T::CIeID      == ISDN::IeFactory::CSpecId_IeUnknown && 
                                T::CIeCodeSet == ISDN::IeFactory::CSpecCodSet) );
        
        m_IeCreators.Add( new MetaType<T>() );
    }

    // --------------------------------------------------------------

    IeFactory::IeFactory(IsdnInfra& infra)
        : m_infra(infra)
    {
        RegIes(*this); 
    }
    
    IeFactory::~IeFactory()
    {        
    }

    ICommonIe* IeFactory::CreateIeObj(IeId id, const RawData& raw)
    {
        for(int i = 0; i < m_IeCreators.Size(); ++i)
        {
            if (m_IeCreators[i]->GetIeId() == id)
            {
                return m_IeCreators[i]->CreateInstance(raw, m_infra);
            }
        }

        return new(m_infra) UnknownIeType(id, raw, m_infra);
    }

    void IeFactory::RegIes(IeFactory& factory)
    {
        ISDN::RegIes(factory);
    }

}

