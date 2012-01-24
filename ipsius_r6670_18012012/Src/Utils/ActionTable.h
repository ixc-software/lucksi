
#ifndef __ACTIONTABLE__
#define __ACTIONTABLE__

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include <boost/bind.hpp>

namespace Utils
{
    namespace
    {
        template <class T> 
        bool IsEqual(T first, T second)
        {
            return (first == second);
        }

        // ------------------------------------------

        template<> 
        bool IsEqual<QString>(QString first, QString second)
        {
            bool res = (first.compare(second, Qt::CaseInsensitive) == 0);

            return res;
        }
      
    } // namespace 

    // ------------------------------------------

    // ActionTable record
    // TKey - method name or other identificator
    // TMethodOwner - method owner;
    template<class TKey, class TOwner>
    class ActionTableRecord
        : boost::noncopyable
    {
        typedef void(TOwner::*TFunc)();

        TKey m_key;
        TOwner *m_pOwner;
        TFunc m_func;

    public:
        ActionTableRecord(TKey key, TOwner *pOwner, TFunc func)
        : m_key(key), m_pOwner(pOwner), m_func(func)
        {
            ESS_ASSERT(m_pOwner != 0);
        }

        TKey getKey()
        {
            return m_key;
        }

        bool IsSameKey(TKey key)
        {
            // return (m_key == key);
            return IsEqual(m_key, key);
        }

        void RunFunc()
        {
            ESS_ASSERT(m_pOwner != 0);
            (m_pOwner->*m_func)();
        }
    };

    // ------------------------------------------

    // ActionTable for void method():
    // TKey - method name or other identificator
    // TMethodOwner - method owner;
    template<class TKey, class TMethodOwner>
    class ActionTable 
        : boost::noncopyable
    {
        // typedef std::vector<ActionTableRecord<TKey, TMethodOwner> > Table;
        typedef boost::ptr_vector<ActionTableRecord<TKey, TMethodOwner> > Table;
        Table m_table;
        TMethodOwner *m_pOwner;

        void AddRecord(TKey key, void (TMethodOwner::*func)())
        {
            for (size_t i = 0; i < m_table.size(); ++i)
            {
                if ( m_table.at(i).IsSameKey(key) ) ESS_THROW(DublicateKey);
            }
            
            // ActionTableRecord<TKey, TMethodOwner> value(key, m_pOwner, func);
            m_table.push_back( new ActionTableRecord<TKey, TMethodOwner>(key, m_pOwner, func) );
        }

    public:
        ESS_TYPEDEF(UnknownKey);
        ESS_TYPEDEF(DublicateKey);

        ActionTable()
        : m_pOwner(0)
        {}

        ActionTable(TMethodOwner *pOwner)
        : m_pOwner(pOwner)
        {}

        void Register( TKey key, TMethodOwner *pOwner, void (TMethodOwner::*func)() )
        {
            m_pOwner = pOwner;

            AddRecord(key, func); 
        }

        void Register( TKey key, void (TMethodOwner::*func)() )
        {
            // assert, that non-empty constructor was called 
            ESS_ASSERT(m_pOwner != 0);

            AddRecord(key, func);
        }

        void RunFunction(TKey key)
        {
            ESS_ASSERT( !m_table.empty() );

            for (size_t i = 0; i < m_table.size(); ++i)
            {
                if ( m_table.at(i).IsSameKey(key) )
                {
                    m_table.at(i).RunFunc();
                    return;
                }
            }

            ESS_THROW(UnknownKey);
        }

        int Size() const { return m_table.size(); }
    };

} // namespace Utils

#endif

