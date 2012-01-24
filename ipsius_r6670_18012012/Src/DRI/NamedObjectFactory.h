#ifndef __NAMEDOBJECTFACTORY__
#define __NAMEDOBJECTFACTORY__

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"
#include "Domain/ObjectName.h"
#include "Domain/IDomain.h"
#include "Domain/NamedObject.h"
#include "DRI/driutils.h"
#include "DRI/INonCreatable.h"

namespace Domain
{
    class NamedObject;

}  // namespace Domain

namespace DRI
{

    // полиморфный интерфейс к типу, зарегестрированному в фабрике
    class INamedObjectType : public Utils::IBasicInterface
    {
    public:

        virtual Domain::NamedObject* 
            CreateInstance(Domain::IDomain *pDomain, QString fullName) const = 0;
        virtual bool AllowCreate() const = 0;

        virtual const char* TypeName() const = 0;
        virtual const QMetaObject& MetaObject() const = 0;
    };

    // --------------------------------------------------

    namespace NamedObjectTypeDetail
    {
        template<class T, bool blockCreate>
        struct Create
        {
            static Domain::NamedObject* 
                Do(Domain::IDomain *pDomain, QString fullName)
            {
                return new T(pDomain, Domain::ObjectName(fullName));
            }
        };

        template<class T>
        struct Create<T, true>
        {
            static Domain::NamedObject* 
                Do(Domain::IDomain *pDomain, QString fullName)
            {
                ESS_HALT("");
                return 0;
            }
        };
        
    } // NamedObjectTypeDetail

    // --------------------------------------------------
    
    // реализация INamedObjectType для типа T
    template<class T>
    class NamedObjectType : public INamedObjectType
    {

        // check T must be QObject (exactly -- NamedObject)
        enum { CCheckBase = boost::is_base_of<Domain::NamedObject, T>::value };      
        BOOST_STATIC_ASSERT(CCheckBase);

        // check for T is INonCreatable
        enum { CBlockCreate = boost::is_base_of<DRI::INonCreatable, T>::value };      
       

    // INamedObjectType impl
    private:

        Domain::NamedObject* 
            CreateInstance(Domain::IDomain *pDomain, QString fullName) const
        {
            return NamedObjectTypeDetail::Create<T, CBlockCreate>::Do(pDomain, fullName);
        }

        bool AllowCreate() const { return !CBlockCreate; }

        const char* TypeName() const
        {
            static std::string name( DriUtils::FormatedTypename<T>(true) );
            return name.c_str(); 
        }

        const QMetaObject& MetaObject() const
        {
            return T::staticMetaObject;
        }

    };

    // --------------------------------------------------

    // глобальная фабрика типов, унаследованных от NamedObject
    class NamedObjectFactory
    {
        Utils::ManagedList<INamedObjectType> m_list;

    public:

        NamedObjectFactory()
        {
        }

        void Register(INamedObjectType *pType)
        {
            if (Find(pType->TypeName()) != 0) 
            {
                ESS_HALT(std::string("Dublicate type register -- ") + pType->TypeName());
            }

            m_list.Add(pType);
        }

        INamedObjectType* Find(const std::string &name) const
        {
            for(size_t i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i]->TypeName() == name) return m_list[i];
            }

            return 0;
        }

        int Count() const { return m_list.Size(); }

        INamedObjectType* operator[](int indx)
        {
            return m_list[indx];
        }

        // singletone
        static NamedObjectFactory& Instance()
        {
            static NamedObjectFactory factory;
            return factory;
        }
    };

    // --------------------------------------------------

    /*
	template<class T, bool isInheritNamedObject>
		class NamedObjectTypeRegisterCheck
	{};

	template<class T>
	class NamedObjectTypeRegisterCheck <T, true>
	{
	public:
		NamedObjectTypeRegisterCheck()
		{
			NamedObjectFactory::Instance().Register( new NamedObjectType<T>() );
		}
	};

	template<class T>
	class NamedObjectTypeRegister :
		NamedObjectTypeRegisterCheck<T, boost::is_base_of<Domain::NamedObject, T>::value >
	{}; */

    // --------------------------------------------------
    
    namespace NamedObjectTypeRegisterDetail
    {
        template<class T, bool needRegister>
        struct Register
        {
            static void Do()
            {
                // nothing
            }
        };

        template<class T>
        struct Register<T, true>
        {
            static void Do()
            {
                NamedObjectFactory::Instance().Register( new NamedObjectType<T>() );
            }
        };
        
    } // NamedObjectTypeRegisterDetail

    // --------------------------------------------------
    
    template<class T>
    class NamedObjectTypeRegister
    {
    public:

        NamedObjectTypeRegister()
        {
            const bool CIsNamedObjBase = boost::is_base_of<Domain::NamedObject, T>::value;
            const bool CIsNamedObj     = boost::is_same<Domain::NamedObject, T>::value;

            const bool CNeedRegister = CIsNamedObjBase && !CIsNamedObj;

            NamedObjectTypeRegisterDetail::Register<T, CNeedRegister>::Do();
        }

    };


}  // namespace DRI

#endif

