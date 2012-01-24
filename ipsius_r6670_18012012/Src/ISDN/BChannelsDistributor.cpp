#include "stdafx.h"
#include "BChannelsDistributor.h"

#include "DssCallParams.h"
#include "L3Packet.h"
#include "IeTypes.h"
#include "L3Call.h"
#include "idsstogroup.h"
#include "Utils/InitVar.h"

#include "L3CallFsmError.h"
#include "IeConstants.h"

namespace 
{
    int InitPropIntfToIntForStrategy(const Utils::InitProp<int>& id)
    {
        return id.IsInited() ? id : -1;
    }

    void ThrowAndCloseCallWithError(ISDN::IeConstants::CauseNum cause)
    {
        ESS_THROW_T(
            ISDN::ErrPacketProcess, 
            boost::shared_ptr<ISDN::ErrActionCloseCall>(new ISDN::ErrActionCloseCall(cause))
            );
    }

} // namespace 

namespace ISDN
{
    /*
    todo:
        большинство ассертов здесь на самом деле должны приводить к дисконнекту вызова.
        надо реализовать механизм закрытия вызова с причиной отсюда.
    */
    using Utils::InitProp;

    SharedIeChan BChannelsDistributor::CreateInquiry(shared_ptr<const DssCallParams> pInParams, L3Call* pCall)
    {            
        SharedBChansWrapper chansInquiry = 
            m_intfGroup->CaptureAny(pInParams->GetReqBchansCount(), -1, pCall->IsOutgoing());
        
        if (chansInquiry.get() == 0) return SharedIeChan();

        pCall->InitBCannels(chansInquiry);        
        
        Utils::InitProp<int> idInInquiry;
        if (m_IL3.GetOptions().m_AssignBChannalsIntfId) idInInquiry.Init( chansInquiry->getIntfId() );
        
        return SharedIeChan(
            new ( m_IL3.GetInfra() ) IeChannelIdentification(chansInquiry->getChans(), idInInquiry)
            );        
    }

    SharedIeChan BChannelsDistributor::CreateResponse(const L3Packet& incoming, L3Call* pCall)
    {
        SharedBChansWrapper captured;

        IeChannelIdentification* pInIe;        
        if ( !incoming.FindIe(pInIe) ) ThrowAndCloseCallWithError(IeConstants::MndIeIsMissing); // стандарт?
            

        IeConstants::ChanSelect selection = pInIe->GetChanSelection();
        IeConstants::PrefExcl prefExcl = pInIe->GetPrefExcl();        
        const SetBCannels inquiry = pInIe->GetBchannels(); //запрошенные каналы        

        
        InitProp<int> intfId = pInIe->GetIntfId();                

        switch(selection)
        {
        case(IeConstants::NoChannel):
            ThrowAndCloseCallWithError(IeConstants::InvalidIeContent); // стандарт?            

        case(IeConstants::B2orReserved):
            ThrowAndCloseCallWithError(IeConstants::InvalidIeContent); // стандарт?

        case(IeConstants::AnyChannel):
            
            captured = m_intfGroup->CaptureAny(
                inquiry.Count(), InitPropIntfToIntForStrategy(intfId), pCall->IsOutgoing()
                );
            if (captured.get() == 0) return SharedIeChan();
            break;

        case(IeConstants::B1orIndInFollowing):

            captured = m_intfGroup->Capture(
                inquiry, InitPropIntfToIntForStrategy(intfId),
                prefExcl == IeConstants::Exclusive, pCall->IsOutgoing()
                );
            if (captured.get() == 0) return SharedIeChan();
            break;

        default:
            ThrowAndCloseCallWithError(IeConstants::InvalidIeContent); // стандарт?

        }

        pCall->InitBCannels(captured);

        Utils::InitProp<int> idInCaptured;
        if ( intfId.IsInited() ) idInCaptured.Init(captured->getIntfId());

        return SharedIeChan(
            new (m_IL3.GetInfra()) IeChannelIdentification(captured->getChans(), idInCaptured, IeConstants::Exclusive)
            );
    }
    
    bool BChannelsDistributor::ProcessResponse(const L3Packet& incoming, L3Call* pCall)
    {        
        IeChannelIdentification* pInIe;
        if( !incoming.FindIe(pInIe) ) ThrowAndCloseCallWithError(IeConstants::MndIeIsMissing); // стандарт?

        const BChannelsWrapper& inquiry = pCall->GetBCannels(); 
        IeConstants::ChanSelect selection = pInIe->GetChanSelection();
        
        const SetBCannels responsed = pInIe->GetBchannels();
        InitProp<int> rspIntfId = pInIe->GetIntfId();
        
        // обработка ситуации когда в ответе тоже, что и в запросе
        if (responsed == inquiry.getChans())
        {
             if (!rspIntfId.IsInited()) return true;
             if ( rspIntfId == inquiry.getIntfId() ) return true;

             // todo обдумать эту ситуацию
             //if ( rspIntfId.IsInited() && !inquiry.getIntfId().IsInited() ) ESS_HALT("BDistr create wrong inquiry");
        }


        if(responsed.Count() != inquiry.getChans().Count() && 
            IeConstants::B1orIndInFollowing == selection)
        {            
            // todo ThrowSendStatusAndDoWarning
            ThrowAndCloseCallWithError(IeConstants::InvalidIeContent);
        }

        //IeConstants::PrefExcl prefExcl = pInIe->GetPrefExcl();   
        bool exclusive = pInIe->GetPrefExcl() == IeConstants::Exclusive;

        //переназначение
        SharedBChansWrapper newChannels;
        switch(selection)
        {
        case(IeConstants::NoChannel):
            ThrowAndCloseCallWithError(IeConstants::InvalidIeContent); // стандарт?

        case(IeConstants::B2orReserved):
            ThrowAndCloseCallWithError(IeConstants::InvalidIeContent); // стандарт?

        case(IeConstants::AnyChannel):
            return true; //nothing todo

        case(IeConstants::B1orIndInFollowing):
            newChannels = 
                m_intfGroup->Capture(responsed, InitPropIntfToIntForStrategy(rspIntfId), exclusive, pCall->IsOutgoing());
            if (newChannels.get() == 0) return false;            
            break;

        default:
            ThrowAndCloseCallWithError(IeConstants::InvalidIeContent); // стандарт?

            //ESS_ASSERT(0 && "Invalid Ie");            
        }

        ESS_ASSERT(newChannels.get() != 0); // paranoid
        pCall->InitBCannels(newChannels);            
        return true;
        
    }

} // namespace ISDN

