#ifndef __MSDATETIMEHELPERS__
#define __MSDATETIMEHELPERS__

// файл предназначен для использования только в MsDateTime.cpp
// он настраивает стратегию синхронизации для многопоточности 
// и генерирует значение по умолчанию для SyncPoint 

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

