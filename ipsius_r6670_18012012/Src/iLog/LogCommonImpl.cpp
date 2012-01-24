#include "stdafx.h"
#include "LogCommonImpl.h"

// ----------------------------------------

namespace iLog
{
    // �������������� ��������������� �������
    
    template class LogRecordT<Timestamp, iLogCommon::LogString, iLogCommon::Convert>;
    template class LogStoreT<iLogCommon::LogRecord, iLogCommon::MultithreadLockStrategy>;
    template class LogSessionT<iLogCommon::LogRecord, Utils::ThreadContext>;
    
}

// ----------------------------------------

namespace iLogCommon
{

    // �������, ����� ������� �� ������������ ��� ����� .cpp ������
    void Convert::Dummi() {}

}

