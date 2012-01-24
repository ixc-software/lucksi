#ifndef __MSDATETIMEHELPERS__
#define __MSDATETIMEHELPERS__

// ���� ������������ ��� ������������� ������ � MsDateTime.cpp
// �� ����������� ��������� ������������� ��� ��������������� 
// � ���������� �������� �� ��������� ��� SyncPoint 

#include "Platform/Platform.h"
#include "Utils/UtilsDateTime.h"
#include "iVDK/CriticalRegion.h"

namespace UtilsDetails
{


    /*    	
    struct ThreadStrategy 
    {

        struct CriticalSection
        {
            CriticalSection(int &i) { iVDK::CriticalRegion::Enter(); }
            ~CriticalSection()      { iVDK::CriticalRegion::Leave(); }
        };

        typedef int                 Mutex;  // nothing
        typedef CriticalSection     Locker;

    }; */
    
	
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
            dt.date().Set(2008, 8, 8);
            dt.time().Set(0, 0, 0);
        }
    };
	
}  // namespace UtilsDetails

#endif

