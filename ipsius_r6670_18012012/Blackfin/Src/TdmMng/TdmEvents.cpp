#include "stdafx.h"

#include "iCmp/ChMngProto.h"

#include "TdmEvents.h"

// --------------------------------------------------

namespace TdmMng
{

    void TdmAsyncEvent::Send( SBProto::ISafeBiProtoForSendPack &proto ) const
    {
        iCmp::BfTdmEvent::Send(proto, m_source, m_event, m_params);
    }

    // ----------------------------------------------------

    void TdmAsyncCaptureData::Send( SBProto::ISafeBiProtoForSendPack &proto ) const
    {
        ESS_ASSERT(m_body != 0);

        iCmp::BfChannelData::Send(proto, m_body->DevName, m_body->ChNumber, 
            m_body->Rx, m_body->Tx, m_body->RxFixed);
    }
    
}  // namespace TdmMng
