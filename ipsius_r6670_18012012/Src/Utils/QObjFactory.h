#ifndef __QOBJFACTORY__
#define __QOBJFACTORY__

#include "Utils/IBasicInterface.h"
#include "Utils/ManagedList.h"

namespace Utils
{
    
    class IMetaObjectAccess : public Utils::IBasicInterface
    {
    public:
        virtual const QMetaObject& Meta() const = 0;
    };

    template<class T>
    class MetaObjectAccessImpl : public IMetaObjectAccess
    {
        const QMetaObject& Meta() const { return T::staticMetaObject; }
    };

    class QObjFactory 
    {
        Utils::ManagedList<IMetaObjectAccess> m_list;

    public:

        void Register(IMetaObjectAccess *pType)
        {
            m_list.Add(pType);
        }

        template<class TFn>
        void Process(TFn fn) const
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                fn( m_list[i]->Meta() );
            }
        }

		const QMetaObject* FindQMetaObject(const std::string &classType) const
		{
			for(int i = 0; i < m_list.Size(); ++i)
			{
				const QMetaObject *meta = &m_list[i]->Meta();

				if (classType.compare(meta->d.stringdata) == 0) return meta;
			}
			return 0;
		}

        static QObjFactory& Instance() 
        {
            static QObjFactory f;
            return f;
        }

    };

    template<class T>
    struct QObjFactoryRegister
    {
        QObjFactoryRegister() 
        { 
            QObjFactory::Instance().Register( new MetaObjectAccessImpl<T>() );
        }
    };
    
    
}  // namespace Utils

#endif