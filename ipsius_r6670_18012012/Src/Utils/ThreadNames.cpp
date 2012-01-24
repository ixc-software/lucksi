#include "stdafx.h"

#include "Utils/GlobalMutex.h"
#include "Utils/IntToString.h"

#include "Platform/IProcSnapshot.h"

#include "ThreadNames.h"

// --------------------------------------------

namespace
{
    Utils::ThreadNames GThreadNames;
}

// --------------------------------------------

namespace Utils
{
    using Platform::Thread;

    ThreadNames::ThreadNames()
    {
        m_list.push_back( Pair(Thread::GetCurrentThreadID(), "main") );
    }

    // ---------------------------------------------------------------

    void ThreadNames::Register( Platform::Thread::ThreadID id, const std::string &name )
    {
        Utils::GlobalMutexLocker locker;

        if ( Find(id) != m_list.end() )
        {
            ESS_HALT( "Dublicate thread ID " + Thread::ThreadIdToStr(id) );
        }

        m_list.push_back( Pair(id, name) );
    }

    // ---------------------------------------------------------------

    void ThreadNames::Unregister( Platform::Thread::ThreadID id )
    {
        Utils::GlobalMutexLocker locker;

        List::iterator i = Find(id);

        if ( i == m_list.end() )
        {
            ESS_HALT( "No thread ID " + Thread::ThreadIdToStr(id) );
        }

        m_list.erase(i);
    }

    // ---------------------------------------------------------------

    /*
    std::string ThreadNames::Info() const
    {
        boost::scoped_ptr<Platform::IProcSnapshot> m_snapshot( Platform::CreateProcSnapshot() );

        Utils::GlobalMutexLocker locker;

        std::ostringstream oss;

        for(int i = 0; i < m_list.size(); ++i)
        {
            const Pair &p = m_list.at(i);
            oss << std::setw(8) << Thread::ThreadIdToStr(p.first) << ": " << p.second;

            std::string prio = m_snapshot->ThreadPriority(p.first);
            if ( !prio.empty() ) 
            {
                oss << " [" << prio << "]";
            }

            oss << "\n";
        }

        return oss.str();
    } */

    std::string ThreadNames::Info() 
    {
        // create snapshot
        boost::scoped_ptr<Platform::IProcSnapshot> m_snapshot( Platform::CreateProcSnapshot() );
        
        if (m_snapshot == 0)  // emulate
        {
            class Emul : public Platform::IProcSnapshot
            {
                List m_list;

            // Platform::IProcSnapshot impl
            private:
                std::string ThreadPriority(Thread::ThreadID id) { return ""; }
                int ThreadCount() { return m_list.size(); }
                Thread::ThreadID ThreadAt(int index) { return m_list.at(index).first; }

            public:
                Emul(const List &list) : m_list(list) { }
            };

            m_snapshot.reset( new Emul(m_list) );
        }

        // make output from snapshot 
        std::ostringstream oss;

        for(int i = 0; i < m_snapshot->ThreadCount(); ++i)
        {
            Platform::Thread::ThreadID id = m_snapshot->ThreadAt(i);

            // id
            oss << std::setw(8) << Thread::ThreadIdToStr(id) << ": ";
            
            // name
            std::string name = ThreadName(id);
            if ( name.empty() ) name = "<unnamed>";
            oss << name;

            // prio
            std::string prio = m_snapshot->ThreadPriority(id);
            if ( !prio.empty() ) 
            {
                oss << " [" << prio << "]";
            }

            oss << "\n";           
        }

        return oss.str();
    }


    // ---------------------------------------------------------------

    ThreadNames& ThreadNames::Instance()
    {
        return GThreadNames;
    }

    // ---------------------------------------------------------------

    std::string ThreadNames::ThreadName( Platform::Thread::ThreadID id )
    {
        Utils::GlobalMutexLocker locker;

        List::iterator i = Find(id);
        if (i == m_list.end()) return "";

        return i->second;
    }

}  // namespace Utils

