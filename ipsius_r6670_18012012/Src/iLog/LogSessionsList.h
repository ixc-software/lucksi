#ifndef __LOGSESSIONSLIST__
#define __LOGSESSIONSLIST__

#include "LogSessionBody.h"
#include "LogSelector.h"

namespace iLog
{
	
    template<class TRecord>
    class LogSessionsList : boost::noncopyable
	{
        typedef typename TRecord::RecordData TData;
        typedef LogSessionBody<TData> SessionBody;
        typedef std::list<SessionBody*> List;
     
        ISessionBodyDestroy &m_destroy;
        List m_list;

    public:

        typedef List LogSessionsSet;

        LogSessionsList(ISessionBodyDestroy &destroy)
            : m_destroy(destroy)
        {
            // ...
        }

        ~LogSessionsList()
        {
            if (m_list.size() != 0)
            {
                std::ostringstream oss;

                oss << "Log sessions list not empty! ";
                
                typename List::iterator i = m_list.begin();

                while(i != m_list.end())
                {
                    SessionBody *pBody = *i;
                    oss << TRecord::DataToString( pBody->Name() ) << "; ";
                    ++i;
                }

                ESS_HALT(oss.str());
            }
        }

        // debug
        void DumpNames(std::vector<std::string> &dump)
        {
            dump.clear();
            if (m_list.size() == 0) return;

            dump.reserve(m_list.size());

            typename List::iterator i = m_list.begin();
            while(i != m_list.end())
            {
                SessionBody *pBody = *i;
                dump.push_back( TRecord::DataToString(pBody->Name()) );
                ++i;
            }
        }

        int Size() const
        {
            return m_list.size();
        }

        bool Exists(const TData &name) const 
        {
            typename List::const_iterator i = m_list.begin();

            while(i != m_list.end())
            {
                const SessionBody *pCurr = *i;

                if (pCurr->Name() == name) return true;
                ++i;
            }

            return false;
        }

        SessionBody* Register(const TData &name)
        {
            SessionBody *p = new SessionBody(m_destroy, name);

            m_list.push_back(p);

            return p;
        }

        void Unregister(Utils::IVirtualDestroy *p)
        {
            typename List::iterator i = m_list.begin();

            while(i != m_list.end())
            {
                SessionBody *pCurr = *i;

                if (pCurr == p)
                {
                    m_list.erase(i);
                    delete pCurr;
                    return;
                }

                ++i;
            }

            ESS_HALT("Not found!");
        }

        int MakeSelectionSet(ISessionFilter<TData> &selector, LogSessionsSet &result)
        {
            int count = 0;

            typename LogSessionsSet::iterator i = m_list.begin();

            while(i != m_list.end())
            {                
                const TData &name = (*i)->Name();
                if ( selector.SessionAccepted(name) ) 
                {
                    result.push_back(*i);
                    count++;
                }

                ++i;
            }

            return count;
        }

	};
	
}  // namespace iLog

#endif
