#ifndef SUBSCENARIESLIST_H
#define SUBSCENARIESLIST_H

#include "stdafx.h"
#include "DssSubScenario.h"

namespace IsdnTest
{
    struct DelSubs
    {
        void operator()(DssSubScenario* p)  {delete p;}
    };

    // Хранение списка подсценариев, опрос завершения,
    // если modManaged == true, то отвечает за удаление подсценариев
    template<bool modManaged = true>
    class SubScenariesList : boost::noncopyable
    {
        typedef std::list<DssSubScenario*> List;
        typedef List::iterator Iter;
        List m_scnVector;

    public:

        void Add(DssSubScenario* pSubs)
        {
            m_scnVector.push_back(pSubs);            
        }

        bool AllComplete()
        {
            /*Iter i = m_scnVector.begin();
            Iter end = m_scnVector.end();

            for (; i != end; ++i )            
                if ( !(*i)->IsComplete() ) return false;            
            return true;*/

            int completed = std::count_if( m_scnVector.begin(), m_scnVector.end(),
                std::mem_fun<bool, DssSubScenario>(&DssSubScenario::IsComplete) );
            return completed == m_scnVector.size();
        }

        void DelIfComplete()
        {
            Iter end = std::partition(m_scnVector.begin(), m_scnVector.end(),
                std::mem_fun<bool, DssSubScenario>(&DssSubScenario::IsComplete));
            std::for_each(m_scnVector.begin(), end, DelSubs() );
            m_scnVector.erase(m_scnVector.begin(), end); 
        }

        ~SubScenariesList()
        {
            if (modManaged)
                std::for_each(m_scnVector.begin(), m_scnVector.end(), DelSubs() );
        }        
        
    };

} // IsdnTest

#endif

