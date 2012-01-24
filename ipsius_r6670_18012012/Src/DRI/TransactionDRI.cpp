#include "stdafx.h"

#include "Domain/DomainClass.h"

#include "TransactionDRI.h"

// ------------------------------------------

namespace DRI
{
    
    

    TransactionDRI::TransactionDRI(Domain::IDomain &domain, ITransToSession &owner) : 
        iCore::MsgObject( domain.getDomain().getMsgThread() ),
        m_domain(domain),
        m_owner(owner)
    {
        m_continueOnError = false;
    }

    // ------------------------------------------
    
    TransactionDRI::~TransactionDRI()
    {
        // verify self state - ?
        // ... 

        if (m_currCmd == 0) return;
        
        m_currCmd->TryComplete(true);
    }

}  // namespace DRI

