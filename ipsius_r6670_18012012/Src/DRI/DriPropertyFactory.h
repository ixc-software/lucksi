#ifndef __DRIPROPERTYFACTORY__
#define __DRIPROPERTYFACTORY__

#include "Utils/IBasicInterface.h"
#include "Utils/ManagedList.h"

namespace Domain
{
    class NamedObject;
}

namespace DRI
{
    using Domain::NamedObject;
    
    class IPropertyType : public Utils::IBasicInterface
    {

    protected:

        static bool IsType(const QMetaProperty &prop, QString name)
        {
            return (prop.typeName() == name.toStdString());
        }

    public:
        virtual bool Handle(const QMetaProperty &prop) const = 0;
        virtual QVariant Set(const QMetaProperty &prop, const QString &val) const = 0;  // can throw
        virtual QString Get(const QMetaProperty &prop, const QVariant &val) const = 0;  // can throw
    };

    // ----------------------------------------
    
    class DriPropertyFactory : boost::noncopyable
    {
        Utils::ManagedList<IPropertyType> m_list;

        IPropertyType* Find(const QMetaProperty &prop) const
        {
            for(size_t i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i]->Handle(prop)) return m_list[i];
            }

            ESS_HALT("");

            return 0;
        }

        void Add(IPropertyType *p)
        {
            ESS_ASSERT(p != 0);

            m_list.Add(p);
        }

    public:

        DriPropertyFactory();

        void Set(const QString &propName, NamedObject *pObj, const QString &val) const;  // can throw
        QString Get(const QString &propName, const NamedObject *pObj) const;  // can throw
        
    };
    
}  // namespace DRI

#endif
