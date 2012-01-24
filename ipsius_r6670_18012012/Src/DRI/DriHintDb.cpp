#include "stdafx.h"

#include "Domain/DomainClass.h"

#include "DriHintDb.h"

// -------------------------------------

namespace DRI
{
    using Utils::StringList;

    // db must be sorted - !
    // can be optimised for binary search 
    void FillFromSorted(const QString &s, const StringList &db, StringList &result)
    {
        for(int i = 0; i < db.size(); ++i)
        {
            const QString &curr = db.at(i);

            if (curr.size() <= s.size()) continue;

            if (curr.startsWith(s, Qt::CaseSensitive)) 
            {                
                result.push_back( curr.mid(s.size()) );
            }
        }
    }

    // -------------------------------------------------------------

    class DriHintDb::StaticMetaDb
    {
        StringList m_list;

    public:

        StaticMetaDb(Domain::IDomain &domain)
        {
            // cache all static meta info
            domain.getDomain().ObjectRoot()->GetAllMetaIdentificators(m_list);
            m_list.sort();
        }

        void Fill(const QString &s, StringList &sl) const
        {
            FillFromSorted(s, m_list, sl);
        }
    };

    // -------------------------------------------------------------

    class DriHintDb::RuntimeNamesDb
    {
        StringList m_list;

    public:

        RuntimeNamesDb()
        {
            // ...
        }

        void Update(Domain::IDomain &domain)
        {
            m_list.clear();

            domain.getDomain().ObjectRoot()->GetAllShortNames(m_list);

            m_list.sort();
        }

        void Fill(const QString &s, StringList &sl) const
        {
            FillFromSorted(s, m_list, sl);
        }

    };

    // -------------------------------------------------------------

    DriHintDb::DriHintDb( Domain::IDomain &domain ) : m_domain(domain)
    {
        m_runtimeUpdateRequested = true;
    }

    // -------------------------------------------------------------

    DriHintDb::~DriHintDb()
    {
        // nothing
    }

    // -------------------------------------------------------------

    DriHintResult* DriHintDb::GetHint( QString input)
    {
        // lazy init of static db
        if (m_staticDb == 0) m_staticDb.reset( new StaticMetaDb(m_domain) );

        // lazy init and update runtime db
        if (m_runtimeDb == 0)
        {
            m_runtimeDb.reset( new RuntimeNamesDb() ); 
            m_runtimeUpdateRequested = true;
        }

        if (m_runtimeUpdateRequested)
        {
            m_runtimeDb->Update(m_domain);
            m_runtimeUpdateRequested = false;
        }

        // find
        StringList sl;
        m_staticDb->Fill(input, sl);
        m_runtimeDb->Fill(input, sl);

        if (sl.isEmpty()) return 0;

        return new DriHintResult(sl);
    }

    
}  // namespace DRI

