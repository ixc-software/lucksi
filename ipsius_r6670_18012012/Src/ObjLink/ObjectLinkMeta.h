#ifndef __OBJECTLINKMETA__
#define __OBJECTLINKMETA__

#include "stdafx.h"
#include "Utils/CRC32.h"
#include "Platform/Platform.h"
#include "ObjLinkInterfaces.h"

namespace ObjLink
{
	
    // мета-данные интерфейсов, порожденных от IObjectLinkInterface
    class IObjectLinkInterfaceMeta : public Utils::IBasicInterface
    {
    public:
        // вспомогательные методы
        virtual std::string TypeName() const = 0;
        virtual bool TypeAccepted(const IObjectLinkInterface *pObject) const = 0;
        virtual Utils::CRC32 TypeCRC() const = 0;

        // методы создания объектов
        virtual IObjectLinkInterface* CreateLocalInterfaceProxy(ILocalProxyHost &host) const = 0;
    };

    // -------------------------------------------------

    // статическая фабрика мета классов
    class ObjectLinkInterfacesFactory
    {
    public:

        template<class T>
        static std::string TypeToName(T *p = 0)
        {
            return Platform::FormatTypeidName(typeid(T).name());
        }

        static void RegisterMeta(IObjectLinkInterfaceMeta *pMeta);
        static const IObjectLinkInterfaceMeta* FindMeta(const std::string &name);
    };

    // -------------------------------------------------

    // класс для регистрации мета-типа
    template<class TIntf, class TLocalIntf>
    class RegisterObjectLinkMeta : public IObjectLinkInterfaceMeta
    {
        Utils::CRC32 m_typeCRC;

    // проверки типов 
    private:
        /*
        // TIntf -> IRemoteInterface
        enum { CCheck0 = boost::is_base_of<IRemoteInterface, TIntf>::value };
        BOOST_STATIC_ASSERT( CCheck0 );

        // TLocalIntf -> TIntf
        enum { CCheck1 = boost::is_base_of<TIntf, TLocalIntf>::value };
        BOOST_STATIC_ASSERT( CCheck1 );

        // TLocalIntf -> ProxyIntfLocal<TIntf>
        enum { CCheck2 = boost::is_base_of<ProxyIntfLocal<TIntf>, TLocalIntf>::value };
        BOOST_STATIC_ASSERT( CCheck2 );

        // TRemoteIntf -> TIntf
        enum { CCheck3 = boost::is_base_of<TIntf, TRemoteIntf>::value };
        BOOST_STATIC_ASSERT( CCheck3 );

        // TRemoteIntf -> ProxyIntfRemote
        enum { CCheck4 = boost::is_base_of<ProxyIntfRemote, TRemoteIntf>::value };
        BOOST_STATIC_ASSERT( CCheck4 );

        // TRemoteIntfSrv -> ProxyRemoteServer
        enum { CCheck5 = boost::is_base_of<ProxyRemoteServer, TRemoteIntfSrv>::value };
        BOOST_STATIC_ASSERT( CCheck5 ); */

    // IObjectLinkInterfaceMeta impl
    private:

        std::string TypeName() const
        {
            return ObjectLinkInterfacesFactory::TypeToName<TIntf>();
        }

        bool TypeAccepted(const IObjectLinkInterface *pObject) const
        {
            const TIntf *p = dynamic_cast<const TIntf*>(pObject);
            return (p != 0);
        }

        Utils::CRC32 TypeCRC() const
        {
            return m_typeCRC;
        }

        IObjectLinkInterface* CreateLocalInterfaceProxy(ILocalProxyHost &host) const
        {
            return new TLocalIntf(host);
        }

    public:

        RegisterObjectLinkMeta(Utils::CRC32 typeCRC) : m_typeCRC(typeCRC)
        {
            ObjectLinkInterfacesFactory::RegisterMeta(this);
        }

    };

	
	
}  // namespace ObjLink

#endif



