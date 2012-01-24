#include "stdafx.h"

#include "SfxProto.h"

#include "EchoEvent.h"


namespace EchoApp
{
    
    void EchoEvent::Send( SBProto::ISafeBiProtoForSendPack &proto ) const
    {
        EchoAsyncEvent::Send(proto, m_eventName, m_chNum, m_params);
    }
        
}  // namespace EchoApp