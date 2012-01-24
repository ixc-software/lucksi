#include "stdafx.h"

#include "iCmp/ChMngProto.h"

#include "AozShared.h"

// --------------------------------------------------

namespace DrvAoz
{
         
    void AbAozEvent::Send( SBProto::ISafeBiProtoForSendPack &proto ) const
    {
        iCmp::BfAbEvent::Send(proto, m_devName, 
            m_event.ChNumber, m_event.Name, m_event.Params);        
    }

    // -------------------------------------------------

    void AbAozEvent::DispatchImpl( TdmMng::IIEventsQueueItemDispatch &callback ) const
    {
        // aleDialBegin
        if (m_event.Name == iCmp::BfAbEvent::CLinePulseDialBegin())
        {
            int digitCount;
            ESS_ASSERT( iCmp::ChMngProtoParams::FromParams(m_event.Params, digitCount) );

            if (digitCount == 0) 
            {
                callback.OnAozLineEvent(m_devName, m_event.ChNumber, TdmMng::aleDialBegin);
            }
        }

        // aleBoardOn/aleBoardOff
        if (m_event.Name == iCmp::BfAbEvent::CBoardState())
        {
            bool connected;
            ESS_ASSERT( iCmp::ChMngProtoParams::FromParams(m_event.Params, connected) );

            callback.OnAozLineEvent(m_devName, -1, 
                connected ? TdmMng::aleBoardOn : TdmMng::aleBoardOff);
        }

        // aleLineBusy/aleLineFree
        if (m_event.Name == iCmp::BfAbEvent::CLineState())
        {
            bool busy;
            ESS_ASSERT( iCmp::ChMngProtoParams::FromParams(m_event.Params, busy) );

            callback.OnAozLineEvent(m_devName, m_event.ChNumber, 
                busy ? TdmMng::aleLineBusy : TdmMng::aleLineFree);
        }

    }

}  // namespace DrvAoz
