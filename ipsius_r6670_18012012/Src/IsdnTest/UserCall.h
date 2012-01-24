#ifndef USERCALL_H
#define USERCALL_H

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

namespace IsdnTest
{
    using boost::shared_ptr;

    typedef int UserCallHandle;

    //class IUserCall
    
    class UserCall
    {
        UserCallHandle m_handle;

    public:

        UserCall(UserCallHandle handle)
            : m_handle(handle)
        {}

        UserCallHandle GetHandle()
        {
            return m_handle;
        }
    };

    class UserCallsOwner
    {
        typedef std::map<UserCallHandle, shared_ptr<UserCall> > Map;
        
        typedef Map::iterator Iterator;
        Map m_calls;

        UserCallHandle m_lastHandle;

        UserCallHandle GenHandle()
        {
            UserCallHandle max = !0;
            if (m_lastHandle == max)
                m_lastHandle = 1;
            else 
                ++m_lastHandle;
            return m_lastHandle;
        }

    public:

        UserCall* CreateOutCall()
        {
            UserCallHandle handle = GenHandle();
            UserCall* pNewCall = new UserCall( handle );            
            bool inserted = m_calls.insert( Map::value_type(handle, shared_ptr<UserCall>( pNewCall )) ).second;
            TUT_ASSERT( inserted );
        }

        void DeleteCall(UserCall* pCall)
        {            
            int count = m_calls.erase( pCall->GetHandle() );
            TUT_ASSERT( count == 1 );//ESS
        }

        ~UserCallsOwner()
        {
            TUT_ASSERT ( m_calls.empty() );
        }
    };
};

#endif
