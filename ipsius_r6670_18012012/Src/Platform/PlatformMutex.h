#ifndef __PLATFORMMUTEX__
#define __PLATFORMMUTEX__

namespace Platform
{
	
    // кросс-платформенная реализация мютекса
    class Mutex
    {
        QMutex m_mutex;

    public:
        
        void Lock()     { m_mutex.lock(); }
        void Unlock()   { m_mutex.unlock(); }
        
        bool TryLock()
        {
            return m_mutex.tryLock();
        }

        QMutex& getImpl()
        {
            return m_mutex;
        }
    };

}  // namespace Platform

#endif
