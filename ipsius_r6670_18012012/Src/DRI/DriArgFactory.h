#ifndef __DRIARGFACTORY__
#define __DRIARGFACTORY__

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"

namespace DRI
{

    class IQGenericArgument : public Utils::IBasicInterface
    {
    public:

        virtual QGenericArgument& GetGenericArgument() = 0;
    };

    // --------------------------------------------------

    class IArgFactoryType : public Utils::IBasicInterface
    {
    public:
        virtual const char* TypeName() const = 0;
        virtual IQGenericArgument* FromString(const QString &val) const = 0;
        virtual IQGenericArgument* DefaultValue() const = 0;
    };

    // --------------------------------------------------
    
    class DriArgFactory
    {
        Utils::ManagedList<IArgFactoryType> m_factory;

    public:

        DriArgFactory();

        void Add(IArgFactoryType *p)
        {
            ESS_ASSERT(p != 0);

            if (Find(p->TypeName()) != 0) ESS_HALT("Dublicate!");

            m_factory.Add(p);
        }

        /*
        IArgFactoryType* Find(const QString &typeName)
        {
            return Find(typeName.toStdString());
        } */

        IArgFactoryType* Find(const std::string &typeName)
        {
            for(size_t i = 0; i < m_factory.Size(); ++i)
            {
                if (m_factory[i]->TypeName() == typeName) return m_factory[i];
            }

            return 0;
        }

        static bool StringToBool(const QString &val, std::string &error);

    };    
    
    
}  // namespace DRI

#endif
