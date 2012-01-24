#include "stdafx.h"
#include "LogCommonImpl.h"

// ----------------------------------------

namespace iLog
{
    // ѕринудительное инстанцирование классов
    
    template class LogRecordT<Timestamp, iLogCommon::LogString, iLogCommon::Convert>;
    template class LogStoreT<iLogCommon::LogRecord, iLogCommon::MultithreadLockStrategy>;
    template class LogSessionT<iLogCommon::LogRecord, Utils::ThreadContext>;
    
}

// ----------------------------------------

namespace iLogCommon
{

    // зацепка, чтобы проекты не комплировали без этого .cpp класса
    void Convert::Dummi() {}

}

