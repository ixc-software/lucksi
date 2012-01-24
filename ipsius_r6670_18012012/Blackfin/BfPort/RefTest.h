#ifndef __REFTEST__
#define __REFTEST__

#include "Utils/ErrorsSubsystem.h"

namespace RefTest
{
    const int CID = 0x1234;

    class DataStore
    {
        int m_id;
        int m_value;

    public:
        DataStore(int value) : m_id(CID), m_value(value) {}

        void Verify() { ESS_ASSERT(m_id == CID); }
    };

    class DataStoreUser
    {
        DataStore &m_store;

    public:
        DataStoreUser(DataStore &store) : m_store(store)
        {
            store.Verify();
        }

        void Verify()
        {
            m_store.Verify();
        }
    };

    class Storage
    {
        DataStore       m_store;
        DataStoreUser   m_user;

    public:
        Storage() : m_store(32), m_user(m_store)
        {
            // ...
        }

        static void Test()
        {
            Storage storage;
            storage.m_user.Verify();
        }
    };

	
}  // namespace RefTest

#endif

