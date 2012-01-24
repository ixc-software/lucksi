
#ifndef __TYPEFACTORY__
#define __TYPEFACTORY__

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"

namespace Utils
{
    // base interface for all type cretaors
    template<class TType, class TTypeParams, class TTypeName>
    class ITypeCreator : public Utils::IBasicInterface
    {
    public:
        virtual TTypeName getTypeName() const = 0;
        virtual TType* Create(const TTypeParams &params) const = 0;
    };

    // ---------------------------------
    
    // Using for creating TType-type objects
    // TType -- basic type for factory
    // TTypeParams -- constructor param type
    // TTypeToCreate -- real type, must be super class for TType
    // TTypeName -- type for type name: QString or std::string
    template<class TType, class TTypeParams, class TTypeToCreate, class TTypeName>
    class TypeCreator : public ITypeCreator<TType, TTypeParams, TTypeName>
    {
        TTypeName m_typeName;

        enum { CCheck = boost::is_base_of<TType, TTypeToCreate>::value };
        BOOST_STATIC_ASSERT( CCheck );

    // ITypeCreator<> impl
    private:
        TTypeName getTypeName() const
        {
            return m_typeName;
        }

        TType* Create(const TTypeParams &params) const
        {
            return new TTypeToCreate(params);
        }

    public:

        TypeCreator(const TTypeName &typeName) : m_typeName(typeName)
        {}
    };     

    // ---------------------------------

    namespace
    {
        template <class T> 
        bool IsEqual(T first, T second)
        {
            return (first == second);
        }

        // ---------------------------------

        template<> 
        bool IsEqual<QString>(QString first, QString second)
        {
            bool res = (first.compare(second, Qt::CaseInsensitive) == 0);

            return res;
        }
      
    } // namespace 

    // ---------------------------------

    // Register of all types
    template<class TType, class TTypeParams, class TTypeName>    
    class TypeFactory
    {
        typedef ITypeCreator<TType, TTypeParams, TTypeName> Item;

        std::vector<Item*> m_creators;
        int m_typeCount;

        Item* Find(const TTypeName &typeName) const
        {
            for (size_t i = 0; i < m_creators.size(); ++i)
            {
                // if (m_creators.at(i)->getTypeName() == typeName) return m_creators.at(i);
                if ( IsEqual(m_creators.at(i)->getTypeName(), typeName) ) return m_creators.at(i);
            }

            return 0;
        }

        enum { CCheckTypeName1 = boost::is_same<TTypeName, QString>::value };
        enum { CCheckTypeName2 = boost::is_same<TTypeName, std::string>::value };
        BOOST_STATIC_ASSERT(CCheckTypeName1 || CCheckTypeName2);

        static std::string ToStdString(const QString &typeName)
        {
            return typeName.toStdString();
        }

        static std::string ToStdString(const std::string &typeName)
        {
            return typeName;
        }


    protected:
        template<class T>
        void Register(const TTypeName &typeName)
        {
            ++m_typeCount;

            // check that typeName is unique
            ESS_ASSERT(Find(typeName) == 0);

            Item *p = new TypeCreator<TType, TTypeParams, T, TTypeName>(typeName);
            m_creators.push_back(p);
        }

    public:

        ESS_TYPEDEF(UnknownTypeName);

        TypeFactory() : m_typeCount(0)
        {}

        virtual ~TypeFactory()
        {
            for (size_t i = 0; i < m_creators.size(); ++i)
            {
                delete m_creators.at(i);
            }

            m_creators.clear();
        }

        TType* Create(const TTypeName &typeName, const TTypeParams &params) const
        {
            Item *p = Find(typeName);
            if (p == 0)
            {
                ESS_THROW_MSG(UnknownTypeName, ToStdString(typeName));
            }

            return p->Create(params);
        }

        int getTypeCount() const { return m_typeCount; }

        TTypeName getTypeName(int index) const 
        {
            return m_creators.at(index)->getTypeName(); 
        }

    };

} // namespace Utils


#endif
