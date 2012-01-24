#ifndef __MSDATETIMEHELPERS__
#define __MSDATETIMEHELPERS__

// ���� ������������ ��� ������������� ������ � MsDateTime.cpp
// �� ����������� ��������� ������������� ��� ��������������� 
// � ���������� �������� �� ��������� ��� SyncPoint 

#include "Platform/Platform.h"
#include "Utils/UtilsDateTime.h"

namespace UtilsDetails
{
	
    class ThreadStrategy 
    {
    public:

        typedef Platform::Mutex       Mutex;
        typedef Platform::MutexLocker Locker;

    };

    struct DefaultValue
    {
        static void Set(Utils::DateTime &dt)
        {
            QDateTime qdt;
            qdt = QDateTime::currentDateTime();

            dt.date().Set(qdt.date().year(), qdt.date().month(), qdt.date().day());
            dt.time().Set(qdt.time().hour(), qdt.time().minute(), qdt.time().second());
        }
    };
	
}  // namespace UtilsDetails

#endif

