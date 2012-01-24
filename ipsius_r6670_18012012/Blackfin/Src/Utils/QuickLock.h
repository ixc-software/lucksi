// #include "Src/Utils/QuickLock.h"

#ifndef __QUICKLOCK__
#define __QUICKLOCK__

namespace Utils
{

	// not implemented yet
    class QuickLock
    {

    public:

        QuickLock()
        {
        }

        void Lock()
        {
        }

        void Unlock()
        {
        }

        static int GetConflictsCount()
        {
            return 0;
        }

    };

    // ------------------------------------------

    // RAII for QuickLock
    class QuickLocker
    {
        QuickLock &m_lock;

    public:

        QuickLocker(QuickLock &lock) : m_lock(lock)
        {
            m_lock.Lock();
        }

        ~QuickLocker()
        {
            m_lock.Unlock();
        }

    };
	
}  // namespace Utils


#endif
