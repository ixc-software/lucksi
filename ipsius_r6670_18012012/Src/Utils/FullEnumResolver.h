#pragma once

#include "Platform/Platform.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/QObjFactory.h"
#include "Utils/IntToString.h"

namespace Utils
{

    class FullEnumResolver
    {

    public:

        class EnumResolver
        {
            typedef std::pair<int, std::string> Pair;

            std::string m_name;
            std::vector<Pair> m_values;

        public:

            ESS_TYPEDEF(FromStringErr);

            EnumResolver()  // std::map requirement
            {
            }

            EnumResolver(const std::string &className, const QMetaEnum &e)
            {
                m_name = Platform::MakeEnumTypeid(className, e.name());

                m_values.reserve( e.keyCount() );
                for(int i = 0; i < e.keyCount(); ++i)
                {
                    m_values.push_back( Pair(e.value(i), e.key(i)) );
                }
            }

            const std::string& EnumName() const
            {
                return m_name;
            }

            const char* EnumResolve(int val) const
            {
                for(int i = 0; i < m_values.size(); ++i)
                {
                    const Pair &p = m_values.at(i);
                    if (p.first == val) return p.second.c_str();
                }

                return 0;
            }

            QList<std::string> AllValues() const
            {
                QList<std::string> res;
                
                for(int i = 0; i < m_values.size(); ++i)
                {
                    res.push_back( m_values.at(i).second );    
                }

                return res;
            }

            int FromString(const std::string &name, bool *pErr = 0) const
            {
                if (pErr != 0) *pErr = false;

                for(int i = 0; i < m_values.size(); ++i)
                {
                    if ( m_values.at(i).second == name ) 
                    {
                        return m_values.at(i).first;
                    }
                }

                if (pErr != 0) 
                {
                    *pErr = true;
                }
                else
                {
                    std::string msg = m_name + " error in FromString for " + name;
                    ESS_THROW_MSG(FromStringErr, msg);
                }

                return -1;
            }

        };

    private:

        typedef std::map<std::string, EnumResolver> Map;

        Map m_map;

        void HandleItem(const QMetaObject &mo)
        {
            for(int i = 0; i < mo.enumeratorCount(); ++i)
            {
                QMetaEnum me = mo.enumerator(i);
                EnumResolver e(mo.className(), me);
                if (m_map.find(e.EnumName()) != m_map.end())
                {
                    ESS_HALT("Dublicate enum " + e.EnumName());
                }

                m_map[ e.EnumName() ] = e;
            }
        }

        const EnumResolver* Find(const char *pTypeName) const
        {
            ESS_ASSERT(pTypeName != 0);
            Map::const_iterator i = m_map.find(pTypeName);
            if (i == m_map.end()) return 0;            
            return &i->second;
        }


    public:

        FullEnumResolver()
        {
            Utils::QObjFactory::Instance().Process( 
                boost::bind(&FullEnumResolver::HandleItem, this, _1) 
                );

            int totalEnums = m_map.size();
        }

        template<class TEnum>
        const char* EnumResolve(TEnum val) const
        {   
            const EnumResolver *pE = GetResolver<TEnum>();
            return (pE != 0) ? pE->EnumResolve(val) : 0;
        }

        template<class TEnum>
        const EnumResolver* GetResolver() const
        {
            BOOST_STATIC_ASSERT( boost::is_enum<TEnum>::value );
            return Find( typeid(TEnum).name() );
        }

        static const FullEnumResolver& Instance();

    };

    // ------------------------------------------------------

    template<class TEnum>
    std::string EnumResolve(TEnum val, bool *pError = 0)
    {
        const char *p = FullEnumResolver::Instance().EnumResolve(val);
        if (pError != 0) 
        {
            *pError = (p == 0);
        }

        return (p != 0) ? p : Utils::IntToString(val);
    }

    // ------------------------------------------------------

    ESS_TYPEDEF(NoEnumResolver);

    template<class TEnum>
    TEnum EnumFromString(const std::string &name, bool *pErr = 0)
    {
        if (pErr != 0) *pErr = false;

        const FullEnumResolver::EnumResolver *resolver = 
            FullEnumResolver::Instance().GetResolver<TEnum>();
        
        if (resolver == 0)
        {
            if (pErr != 0) *pErr = true;
            else 
            {
                ESS_THROW_MSG(NoEnumResolver, "Can't get resolver for " + name);
            }

            return TEnum(0);  // dummi
        }
        
        return TEnum( resolver->FromString(name, pErr) );
    }

    
}  // namespace Utils

