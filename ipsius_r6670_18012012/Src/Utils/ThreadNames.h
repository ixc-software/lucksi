#pragma once

#include "Platform/Platform.h"

namespace Utils
{
    
    class ThreadNames
    {
        typedef std::pair<Platform::Thread::ThreadID, std::string> Pair;
        typedef std::vector<Pair> List;

        List m_list;

        List::iterator Find(Platform::Thread::ThreadID id)
        {
            List::iterator i = m_list.begin();

            while(i != m_list.end())
            {
                if (i->first == id) break;
                ++i;
            }

            return i;
        }

    public:

        ThreadNames();

        void Register(Platform::Thread::ThreadID id, const std::string &name);
        void Unregister(Platform::Thread::ThreadID id);

        std::string Info();  // const 
        std::string ThreadName(Platform::Thread::ThreadID id);  // const 

        static ThreadNames& Instance();
    };

    // -------------------------------------------------

    class ThreadNamesLocker
    {
        Platform::Thread::ThreadID m_id;

    public:

        ThreadNamesLocker(Platform::Thread::ThreadID id, const std::string &name) : m_id(id)
        {
            ThreadNames::Instance().Register(id, name);
        }

        ~ThreadNamesLocker()
        {
            ThreadNames::Instance().Unregister(m_id);
        }
    };
    
    
}  // namespace Utils