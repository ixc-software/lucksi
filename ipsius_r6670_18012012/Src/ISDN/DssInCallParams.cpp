#include "stdafx.h"

#include "DssInCallParams.h"
#include "L3PacketTypes.h"
#include "IeTypes.h"
#include "L3CallFsmError.h"

namespace ISDN
{
    DssInCallParams::DssInCallParams(const PacketSetup& pack, const BChannelsWrapper& captured) : DssCallParams()
    {
        // TODO - разобрать случай списка Ie channel id, add asserts and throw`s,
        // add initProp for optional fields
        
        //IeChannelIdentification* pIeChanId;
        //if ( pack.FindIe(pIeChanId) ) // optional Ie
        {
            SetReqBchansCount( captured.getChans().Count() );
            SetAckBchans( captured );
        }        

        BearerCapability* pBcIe;
        //ESS_ASSERT ( pack.FindIe(pBcIe) ); // mandatory Ie
        if ( !pack.FindIe(pBcIe) )            
            ESS_THROW_T(
            ErrPacketProcess, 
            shared_ptr<ErrActionCloseCall>(new ErrActionCloseCall(IeConstants::MndIeIsMissing))
            );    

        IeConstants::TransCap transCap =  pBcIe->GetTransCap();     

        switch(transCap)
        {
            case(IeConstants::SPEECH):
                break;
            case(IeConstants::AUDIO_3_1K):
                this->SetMode(IeConstants::Circuit);
                break;
            case(IeConstants::DIGITAL):
                break;
            case(IeConstants::RESTRICTED_DIGITAL):            
                break;
            case(IeConstants::DIGITAL_W_TONES):
                break;
            case(IeConstants::VIDEO):
                this->SetMode(IeConstants::Packet);
                break;
            default:
                ESS_THROW_T(
                    ErrPacketProcess, 
                    shared_ptr<ErrActionCloseCall>(new ErrActionCloseCall(IeConstants::InvalidIeContent))
                    );
        }

        IeCalledPartyNumber *pCalledIe;
        if ( pack.FindIe(pCalledIe) )
        {
            SetCalled( DssPhoneNumber( pCalledIe->GetNum() ) );
            // TODO:
            //pCalledIe->GetPlan();?
            //pCalledIe->GetTypeOfNum();?
        }
        

        IeCallingPartyNumber *pCallingIe;
        if ( pack.FindIe(pCallingIe) )
        {
            SetCalling( DssPhoneNumber( pCallingIe->GetNum() ) );
            // TODO Other ie field ??
        }


        //...
       
        //SetServInd      
        
    }

    DssInCallParams* DssInCallParams::Create(IIsdnAlloc& alloc, const PacketSetup& pack, const BChannelsWrapper& captured)
    {
        return new(alloc) DssInCallParams(pack, captured);
    }

} // ISDN

