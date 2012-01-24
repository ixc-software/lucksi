#ifndef __GLOBALMUTEX__
#define __GLOBALMUTEX__

namespace Utils
{
    
    bool GlobalMutextLock();
    void GlobalMutextUnlock();
    
    class GlobalMutexLocker : boost::noncopyable
    {
        bool m_locked;

    public:
        GlobalMutexLocker()  
        { 
            m_locked = GlobalMutextLock(); 
        }

        ~GlobalMutexLocker() 
        { 
            if (m_locked) GlobalMutextUnlock(); 
        }
    };
    
}  // namespace Utils

#endif
