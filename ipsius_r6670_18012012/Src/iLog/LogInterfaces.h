#ifndef __LOGINTERFACES__
#define  __LOGINTERFACES__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Utils/IVirtualDestroy.h"
#include "LogRecordT.h"

namespace iLog
{

    template<class TRecord>
    class ILogSession : public Utils::IBasicInterface
    {
        typedef typename TRecord::RecordData TData;

    public:

        virtual void LogToSession(const TData &data, 
                                  RecordKind<TData> recordKind = RecordKind<TData>()) = 0;

        virtual void DropSessionToStore() = 0;

        virtual RecordKind<TData> CreateRecordKind(const TData &data) = 0;
    };

    template<class TRecord>
    class ISessionToLogStore : public Utils::IBasicInterface
    {
        typedef typename TRecord::RecordData TData;

    public:

        virtual void AddRecord(TRecord *pRecord) = 0;
        virtual void AddRecords(std::list<TRecord*> &list) = 0;

        virtual LogSessionBody<TData>* CreateSession(const TData &sessionName) = 0;

        /*
        virtual typename TRecord::RecordTag 
            CreateNewTag(const typename TRecord::RecordData &tagDesc) = 0;

        virtual void RegisterForDelete(Utils::IVirtualDestroy *pObject) = 0; */
    };


}  // namespace iLog

#endif

