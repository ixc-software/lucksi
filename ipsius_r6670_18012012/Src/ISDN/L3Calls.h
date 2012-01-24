#ifndef L3CALLS_H
#define L3CALLS_H

#include "stdafx.h"

#include "IL3Calls.h"
#include "iCore/MsgObject.h"
#include "DssCause.h"
#include "DssUpIntf.h"

#include "L3StatData.h"

namespace ISDN
{
    using boost::shared_ptr;

    class IIsdnL3Internal;
    
    // Владение и управление списком вызовов L3Call, вычисление набора занятых каналов
    class L3Calls : public IL3Calls,
        public iCore::MsgObject,
        boost::noncopyable
    {
        typedef std::list<L3Call* > CallList;

        enum State
        {
            st_normal,
            st_asyncClearing
        };       

    public:

        L3Calls(IIsdnL3Internal* IL3, iCore::MsgThread& thread);
        ~L3Calls();

        // Sync clearing procedure
        void Clear(shared_ptr<const DssCause> pCause); // прим: раньше была асинхронной. Нужна ли AsyncClear ?
        void AsyncClear(shared_ptr<const DssCause> pCause);        
        void CreateOutCall(BinderToUserCall link, shared_ptr<const DssCallParams> pParams, const std::string& callName);        

        // Sync clearing by the channels
        void CloseCalls(int chanIntfId, shared_ptr<const DssCause> pCause);                
        void CloseCalls(const SetBCannels& channels, shared_ptr<const DssCause> pCause);

    private:       
                   
        void RegCall(L3Call* pNewCall);
        // Implementation of IL3Calls: 
        void UnregCall(L3Call* pCall); // override;
        void NotifyIfEmpty();
        L3Call* CreateInCall(const CallRef& callRef); // override;
        /*return 0 if not found*/
        L3Call* Find(const CallRef& callRef); // override;        
        void AsyncDeleteCall(L3Call* pL3Call); // override;
        void Nop(L3Call* pL3Call);  
        void ClearState();

	private:
        State m_state;        
        ILoggable m_logSession;
        CallList m_calls;        
        IIsdnL3Internal& m_IL3;        
    };

} // ISDN

#endif

