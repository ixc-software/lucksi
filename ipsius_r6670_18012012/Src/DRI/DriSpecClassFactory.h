
#ifndef __DRISPECCLASSFACTORY__
#define __DRISPECCLASSFACTORY__

// DriSpecClassFactory.h

namespace DRI
{
    // List of classes using inside NamedObject::MetaTypeInfo
    class DriSpecClassFactory
    {
        QStringList m_showWithInherited;
        
    public:
        // register className = metaObject()->className();
        void RegisterAlwaysShowWithInherited(const QString &className)
        {
            ESS_ASSERT(!m_showWithInherited.contains(className));
            m_showWithInherited.push_back(className);
        }

        bool IsAlwaysShowWithInherited(const QString &className)
        {
            return m_showWithInherited.contains(className);
        }
    };
    
} // namespace DRI

#endif
