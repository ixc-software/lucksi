#ifndef __DRIHINTDB__
#define __DRIHINTDB__

#include "Utils/StringList.h"
#include "Domain/IDomain.h"

namespace DRI
{
    using boost::scoped_ptr;

    // Hint result
    class DriHintResult : boost::noncopyable
    {
        Utils::StringList m_list;
        int m_currIndex;

    public:

        DriHintResult(const Utils::StringList &data)
        {
            for(int i = 0; i < data.size(); ++i) m_list.push_front( data.at(i) );

            m_list.sort();

            m_currIndex = 0;
        }

        QString Curr() const
        {
            if (m_currIndex >= m_list.size()) return "";

            return m_list.at(m_currIndex);
        }

        void Next()
        {
            if (m_currIndex >= m_list.size()) return;
            
            m_currIndex++;
        }

    };

    // -----------------------------------------------
    
    // This class contain all meta and runtime information for Telnet TAB key hints
    // Static meta info -- all types, method, props names. Get it once, lazy.
    // Runtime info -- with object names in tree. Lazy updated after ObjCreate/ObjDelete
    class DriHintDb : boost::noncopyable
    {

        class StaticMetaDb;
        class RuntimeNamesDb;

        Domain::IDomain &m_domain;

        scoped_ptr<StaticMetaDb> m_staticDb;

        bool m_runtimeUpdateRequested;
        scoped_ptr<RuntimeNamesDb> m_runtimeDb;

    public:

        DriHintDb(Domain::IDomain &domain);

        ~DriHintDb();

        // result can be 0 if no hint data
        // result must be free by extenal code
        DriHintResult* GetHint(QString input);

        void DoStaticInit()
        {
            scoped_ptr<DriHintResult> p( GetHint("do_static_init") );            
        }

        void ObjectTreeChanged()
        {
            m_runtimeUpdateRequested = true;
        }

    };
    
}  // namespace DRI


#endif
