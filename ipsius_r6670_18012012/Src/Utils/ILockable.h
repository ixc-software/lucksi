
#ifndef __ILOCKABLE__
#define __ILOCKABLE__

#include "Platform/Platform.h"
#include "Utils/AtomicTypes.h"

namespace Utils
{

    //  "ѕримесь" дл€ классов, требующих синхронного доступа из нескольких потоков    
    class ILockable
    {
        Platform::Mutex m_mutex;
        AtomicBool m_locked;

        friend class Locker;

        void Lock()
        {
            m_mutex.Lock();
            m_locked.Set(true);
        }

        void Unlock()
        {
            m_locked.Set(false);
            m_mutex.Unlock();
        }

    public:

        bool IsLockedByLocker() { return m_locked; }
    };

    // RAII solution for ILockable
    class Locker
    {
        ILockable &m_obj;

    public:

        Locker(ILockable &obj) : m_obj(obj)
        {
            m_obj.Lock();
        }

        virtual ~Locker()
        {
            m_obj.Unlock();
        }
    };

}  // namespace Utils


#endif

