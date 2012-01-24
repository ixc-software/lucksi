#ifndef __LOGSTORESTRATEGY__
#define __LOGSTORESTRATEGY__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"

namespace iLog
{

    template<class TRecord>
    class ILogStoreAccess : public Utils::IBasicInterface
    {
    public:
        virtual std::list<TRecord*>& StoredRecords() = 0;
        virtual void StoreOnlyLast(int records) = 0;
    };

    
    template<class TRecord>
    class ILogStoreStrategy : public Utils::IBasicInterface
    {

    protected:
        typedef std::list<TRecord*> RecordsList;

    public:
        typedef ILogStoreAccess<TRecord> LogStoreAccess;

        virtual bool OnNewRecord(LogStoreAccess &logStore, TRecord &record) = 0;
        virtual void OnFinalize(LogStoreAccess &logStore) = 0;
    };

    /*
    template<class TRecord>
    class BasicLogStoreStrategy
    {        
    public:
        typedef ILogStoreAccess<TRecord> LogStoreAccess;
        typedef std::list<TRecord*> RecordsList;

        BasicLogStoreStrategy() : m_pStoreAccess(0), m_pList(0) {}

        virtual ~BasicLogStoreStrategy() {}

        void SetStoreAccess(LogStoreAccess &owner)
        {
            ESS_ASSERT(m_pStoreAccess == 0);

            m_pStoreAccess = &owner;
            m_pList = &(m_pStoreAccess->StoredRecords());
        }

        bool OnNewRecord(TRecord &record)
        {
            ESS_ASSERT(m_pStoreAccess != 0);
            return ProcessNewRecord(record);
        }

        void OnFinalize()
        {
            ESS_ASSERT(m_pStoreAccess != 0);
            ProcessFinalize();
        }

    protected:

        RecordsList& StoredRecords()
        {
            ESS_ASSERT(m_pList != 0);
            return *m_pList;
        }

        LogStoreAccess& StoreAccess()
        {
            ESS_ASSERT(m_pStoreAccess != 0);
            return *m_pStoreAccess;
        }

    private:

        virtual bool ProcessNewRecord(TRecord &record) = 0;
        virtual void ProcessFinalize() = 0;

        LogStoreAccess *m_pStoreAccess;
        RecordsList *m_pList;

    }; */


}  // namespace iLog


#endif

