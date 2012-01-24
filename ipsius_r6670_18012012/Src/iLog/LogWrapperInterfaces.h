#ifndef __LOGWRAPPERINTERFACES__
#define __LOGWRAPPERINTERFACES__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "LogTimestamp.h"
#include "LogCommonImpl.h"

/*
    �� � ���� ������ �� �������� ���� ���� � ������ .h �����!
    �� ����� �� ����� ������� ��������� ���������� �� iLog!

*/

namespace iLogW
{
    using iLogCommon::LogString;

    // ------------------------------------------------------------------

    // ���������, ����������� �������� ������ � LogStore
    class ILogSessionToLogStore : public Utils::IBasicInterface
    {
    public:
        virtual iLogCommon::LogStore& getLogStore() = 0;
    };

    // ------------------------------------------------------------------

    // ������ � LogSession -- ����������� �������� ������ � ���
    class ILoggerToSession : public Utils::IBasicInterface
    {
    public:
        virtual void AddRecord(const LogString &data, iLog::RecordKind<LogString> recordKind) = 0;
    };

    // ------------------------------------------------------------------

    // �������������� LogRecord
    class ILogRecordFormater : public Utils::IBasicInterface
    {
    public:
        virtual void FormatRecord(iLogCommon::LogRecord &record, LogString &result) = 0;
    };

    // ------------------------------------------------------------------

    // ��������� ��� ����������� ������������� LogSession <-> LogStroe
    class ILogSessionSyncStrategy : public Utils::IBasicInterface
    {
    public:
        virtual bool DoSyncWithStore(iLogCommon::LogRecord &record) = 0;
    };

    // ------------------------------------------------------------------

    class ILogSelector : public Utils::IBasicInterface
    {
    public:
        virtual iLog::ILogSelector<iLogCommon::LogRecord>& getSelector() = 0;
    };

    // ------------------------------------------------------------------

    class ILogStoreStrategy : public Utils::IBasicInterface
    {
    public:
        typedef iLog::ILogStoreAccess<iLogCommon::LogRecord> LogStoreAccess;

        virtual bool OnNewRecord(LogStoreAccess &logStore, iLogCommon::LogRecord &record) = 0;
        virtual void OnFinalize(LogStoreAccess &logStore) = 0;        
    };

}  // namespace iLogW


#endif
