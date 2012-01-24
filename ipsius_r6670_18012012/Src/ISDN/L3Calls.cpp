#include "stdafx.h"
#include "L3Calls.h"

#include "IIsdnL3Internal.h"
#include "L3Call.h"
#include "CallRef.h"

namespace 
{
    const int CMaxListLenth = 32 * 3;
} // namespace 

namespace ISDN
{
    L3Calls::L3Calls( IIsdnL3Internal* IL3, iCore::MsgThread& thread ) : MsgObject(thread),
        m_state(st_normal),
        m_logSession(IL3->getParentSession(), "/L3Calls"),
        m_IL3(*IL3)
    {
    }

    L3Calls::~L3Calls()
    {
        if (m_logSession.getTraceOn())
            LogWriter(&m_logSession).Write() << "~L3Calls used calls - " << m_calls.size();        

        Clear( DssCause::Create(&m_IL3, IeConstants::NormUnspec) );
    }

    void L3Calls::Clear( shared_ptr<const DssCause> pCause )
    {
        ClearState();        
        
        CallList::iterator i = m_calls.begin();
        while( i != m_calls.end() )
        {
            L3Call* pCall = *i;
            ++i;

            pCall->DropCall( pCause ); 
        }
    }

    void L3Calls::AsyncClear( shared_ptr<const DssCause> pCause )
    {
        ESS_ASSERT(st_normal == m_state);
        m_state = st_asyncClearing;        
        for (CallList::iterator i = m_calls.begin(); i != m_calls.end(); ++i)
        {
            (*i)->AsyncDisconnect(pCause);
        }        
    }

    void L3Calls::CreateOutCall( BinderToUserCall link, shared_ptr<const DssCallParams> pParams, const std::string& callName )
    {        
        if (st_normal != m_state)
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( TemporaryAnavailable::Create(m_IL3.GetInfra()) ));		

        L3Call* pNewCall = 0;

        pNewCall = L3Call::CreateOutCall( m_IL3, link, pParams, callName );            
        RegCall(pNewCall);                                   
        m_IL3.getStat().CallInc();
        ESS_ASSERT(pNewCall);
    }    

    void L3Calls::RegCall( L3Call* pNewCall )
    {
        ESS_ASSERT( m_calls.size() < CMaxListLenth );
        m_calls.push_back(pNewCall);
    }

    void L3Calls::UnregCall( L3Call* pCall ) /*  */
    {        
        CallList::iterator i;
        i = std::find(m_calls.begin(), m_calls.end(), pCall);
        ESS_ASSERT( i != m_calls.end() ) ;

        m_calls.erase(i);        
        NotifyIfEmpty();
    }

    void L3Calls::NotifyIfEmpty()
    {
        if (m_state != st_asyncClearing || !m_calls.empty()) return;
        
        ClearState();
        m_IL3.AllCallsClearNotification();
    }

    L3Call* L3Calls::CreateInCall( const CallRef& callRef ) /*  */
    {        
        L3Call* pNewCall = L3Call::CreateInCall( callRef, m_IL3 );
        RegCall(pNewCall);
        m_IL3.getStat().CallInc();
        return pNewCall;
    }

    L3Call* L3Calls::Find( const CallRef& callRef ) /*  */
    {
        CallList::iterator i;        

        for(i = m_calls.begin(); i != m_calls.end(); ++i)
        {
            if ((*i)->CallRefEqual(callRef)) return (*i);
        }

        return 0;
    }

    void L3Calls::AsyncDeleteCall( L3Call* pL3Call ) /*  */
    {   
        UnregCall(pL3Call);
        PutMsg(this, &L3Calls::Nop, pL3Call); // удаление вызова в сообщении     
    }

    void L3Calls::Nop( L3Call* pL3Call )
    {
    }

    void L3Calls::CloseCalls( int chanIntfId, shared_ptr<const DssCause> pCause )
    {
        CallList::iterator i = m_calls.begin();
        while( i != m_calls.end() )
        {
            L3Call* pCall = *i;
            ++i;
            if (!pCall->HaveBChannels()) continue;
            if (pCall->GetBCannels().getIntfId() ==  chanIntfId ) 
            {  
                pCall->DropCall(pCause);
            }
        }
    }

    void L3Calls::CloseCalls( const SetBCannels& channels, shared_ptr<const DssCause> pCause )
    {
        CallList::iterator i = m_calls.begin();
        while( i != m_calls.end() )
        {
            L3Call* pCall = *i;
            ++i;
            if (!pCall->HaveBChannels()) continue;
            const SetBCannels& currChannels = pCall->GetBCannels().getChans();
            for (SetBCannels::ConstIterator i = currChannels.Begin(); i != currChannels.Begin(); ++i)
            {
                if (!channels.IsExist(*i)) continue;
                pCall->DropCall(pCause);
                break;
            }               
        }
    }

    void L3Calls::ClearState()
    {
        m_state = st_normal;        
    }
} // namespace BfEmul



