#include "stdafx.h"
#include "L3PacketTypes.h"
#include "L3Call.h"
#include "IeTypes.h"

#include "isdnl3.h"

namespace ISDN
{
    PacketAlerting::PacketAlerting(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketAlerting::PacketAlerting(IIsdnL3Internal& IL3, const CallRef &callref) :
	    Base(callref, IL3)
    {
        //LogCreationOutgoing();
    }        

    bool PacketAlerting::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {            
        //std::multiset<IeId>::const_iterator end = exist.end();
        //if (end == exist.find( :GetId() )) return false;
        return true;
    }

	void PacketAlerting::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    bool PacketAlerting::IeCorrectForThisMsgType(IeId id) // override
    {
        //табличная проверка
        return true; // !!! пока так
    }

    PacketConnect::PacketConnect(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketConnect::PacketConnect(IIsdnL3Internal& IL3, const CallRef &callref) :
    Base(callref, IL3)
    {
        //LogCreationOutgoing();
    }        

    bool PacketConnect::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {            
        //std::multiset<IeId>::const_iterator end = exist.end();
        //if (end == exist.find( :GetId() )) return false;
        return true;
    }   

	void PacketConnect::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    bool PacketConnect::IeCorrectForThisMsgType(IeId id) // override
    {
        //табличная проверка
        return true; // !!! пока так
    }

    PacketConnectAck::PacketConnectAck(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketConnectAck::PacketConnectAck(IIsdnL3Internal& IL3, const CallRef &callref) :
    Base(callref, IL3)
    {
        //LogCreationOutgoing();
    }        

    bool PacketConnectAck::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {            
        //std::multiset<IeId>::const_iterator end = exist.end();
        //if (end == exist.find( :GetId() )) return false;
        return true;
    }

	void PacketConnectAck::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    bool PacketConnectAck::IeCorrectForThisMsgType(IeId id) // override
    {
        //табличная проверка
        return true; // !!! пока так
    }        

    PacketDisc::PacketDisc(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketDisc::PacketDisc(IIsdnL3Internal& IL3, const CallRef &callref) :
    Base(callref, IL3)
    {
        //LogCreationOutgoing();
    }        

    bool PacketDisc::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {            
        std::multiset<IeId>::const_iterator end = exist.end();
        if (end == exist.find( IeCause::GetId() )) return false;
        return true;
    }

	void PacketDisc::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    bool PacketDisc::IeCorrectForThisMsgType(IeId id) // override
    {
        //табличная проверка
        // IeCause is exist
        return true; // !!! пока так
    }

    PacketSetup::PacketSetup(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketSetup::PacketSetup(IIsdnL3Internal& IL3, const CallRef &callref) :
    Base(callref, IL3) 
    {
        //LogCreationOutgoing();
    }        

    bool PacketSetup::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {            
        std::multiset<IeId>::const_iterator end = exist.end();
        if (end == exist.find( BearerCapability::GetId() )) return false;
        return true;
    }

	void PacketSetup::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    bool PacketSetup::IeCorrectForThisMsgType(IeId id) // override
    {
        //табличная проверка
        return true; // !!! пока так
    }

    PacketSetupAck::PacketSetupAck(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketSetupAck::PacketSetupAck(IIsdnL3Internal& IL3, const CallRef &callref) :
    Base(callref, IL3)
    {
        //LogCreationOutgoing();
    }        

    bool PacketSetupAck::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {            
        //std::multiset<IeId>::const_iterator end = exist.end();
        //if (end == exist.find( :GetId() )) return false;
        return true;
    }

	void PacketSetupAck::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    bool PacketSetupAck::IeCorrectForThisMsgType(IeId id) // override
    {
        //табличная проверка
        return true; // !!! пока так
    }   

    PacketProceeding::PacketProceeding(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketProceeding::PacketProceeding(IIsdnL3Internal& IL3, const CallRef &callref) :
    Base(callref, IL3)
    {
        //LogCreationOutgoing();
    }        

    bool PacketProceeding::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {            
        //std::multiset<IeId>::const_iterator end = exist.end();
        //if (end == exist.find( :GetId() )) return false;
        return true;
    }

	void PacketProceeding::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    bool PacketProceeding::IeCorrectForThisMsgType(IeId id) // override
    {
        //табличная проверка
        return true; // !!! пока так
    }

    PacketRelease::PacketRelease(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketRelease::PacketRelease(IIsdnL3Internal& IL3, const CallRef &callref) :
    Base(callref, IL3)
    {
        //LogCreationOutgoing();
    }        

    
    bool PacketRelease::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {            
        //std::multiset<IeId>::const_iterator end = exist.end();
        //if (end == exist.find( :GetId() )) return false;
        return true;
    }

	void PacketRelease::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    bool PacketRelease::IeCorrectForThisMsgType(IeId id) // override
    {
        //табличная проверка
        return true; // !!! пока так
    }     

    PacketReleaseComplete::PacketReleaseComplete(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketReleaseComplete::PacketReleaseComplete(IIsdnL3Internal& IL3, const CallRef &callref) :
        Base(callref, IL3)
    {
        //LogCreationOutgoing();
    }        
    
    bool PacketReleaseComplete::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {            
        //std::multiset<IeId>::const_iterator end = exist.end();
        //if (end == exist.find( :GetId() )) return false;
        return true;
    }

	void PacketReleaseComplete::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    // вызывыается при добавлении Ие
    bool PacketReleaseComplete::IeCorrectForThisMsgType(IeId id) // override
    {
        //табличная проверка
        return true; // !!! пока так
    }

    //---------------------------------------------------------------------------

    PacketStatus::PacketStatus(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketStatus::PacketStatus(IIsdnL3Internal& IL3, const CallRef &callref)
        : Base(callref, IL3)
    {
        //LogCreationOutgoing();
    }

    bool PacketStatus::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {
        // cause, callState        
        if (exist.end() == exist.find( IeCallState::GetId() )) return false;        
        if (exist.end() == exist.find( IeCause::GetId() )) return false;
        return true;
    }

	void PacketStatus::UpRout(L3Call &call)  // override
	{
        ESS_ASSERT(!GetCallref().IsGlobal());
		call.RoutePacket(this); // UpEvens            
	}

    void PacketStatus::UpRout( IsdnL3& receiver )
    {
        ESS_ASSERT(GetCallref().IsGlobal());
        receiver.ProcessGlobCrefPack(this);
    }

    // вызывыается при добавлении Ие
    bool PacketStatus::IeCorrectForThisMsgType(IeId id) // override
    {
        //M + Display
        return true;
    }

    //-----------------------------------------------------------------------------

    PacketInfo::PacketInfo(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketInfo::PacketInfo(IIsdnL3Internal& IL3, const CallRef &callref)
        : Base(callref, IL3)
    {}

    bool PacketInfo::ValidateMandatoryIes(const std::multiset<IeId>& exist) const // override
    {        
        return true; // all optional
    }

	void PacketInfo::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    // вызывыается при добавлении Ие
    bool PacketInfo::IeCorrectForThisMsgType(IeId id) // override
    {
        // todo SendingComplete, Display, KeypadFacility, signal, calledNum
        return true;
    }

    //-----------------------------------------------------------------------------

    PacketProgress::PacketProgress(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data )
        : Base(l2data, infra, ieFactory) 
    {}

    PacketProgress::PacketProgress( IIsdnL3Internal& IL3, const CallRef &callref) : 
		Base(callref, IL3)
    {}

    bool PacketProgress::ValidateMandatoryIes( const std::multiset<IeId>& exist ) const        
    {                
        return exist.find(IeProgressInd::GetId()) != exist.end();
    }

	void PacketProgress::UpRout(L3Call &call)  // override
	{
		call.RoutePacket(this); // UpEvens            
	}

    bool PacketProgress::IeCorrectForThisMsgType( IeId id )
    {
        // todo BC, Cause, ProgInd, Disp, HLCapability
        return true;
    }

    // ------------------------------------------------------------------------------------

    PacketRestart::PacketRestart( IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data ) 
        : Base(l2data, infra, ieFactory)
    {
        if (!GetCallref().IsGlobal())
        {
            boost::shared_ptr<DssWarning> err(WrongPack::Create(infra, "Restart with no global cref", l2data) );
            ESS_THROW_T(DssWarningExcept, err);
        }
    }

    PacketRestart::PacketRestart( IIsdnL3Internal& IL3/*, const CallRef &callref*/ ) 
        : Base(IL3.GetCallRefGen().GenCallRef(true), IL3)
    {
    }

    bool PacketRestart::ValidateMandatoryIes( const std::multiset<IeId>& exist ) const
    {
        return exist.find(IeRestartInd::GetId()) != exist.end();
    }

    void PacketRestart::UpRout( L3Call & ) /*  */
    {
        ESS_HALT("PacketRestart rout to call not accepted.");
    }

    void PacketRestart::UpRout( IsdnL3& receiver )
    {
        receiver.ProcessGlobCrefPack(this);
    }

    bool PacketRestart::IeCorrectForThisMsgType( IeId id )
    {
        return 
            id == IeChannelIdentification::GetId() ||
            id == IeRestartInd::GetId() ||
            id == IeDisplay::GetId();
    }

    // ------------------------------------------------------------------------------------   

    PacketRestartAck::PacketRestartAck( IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data ) 
        : Base(l2data, infra, ieFactory)
    {
        if (!GetCallref().IsGlobal())
        {
            boost::shared_ptr<DssWarning> 
                err( WrongPack::Create(infra, "RestartAck with no global cref", l2data) );
            ESS_THROW_T(DssWarningExcept, err);
        }
    }

    PacketRestartAck::PacketRestartAck( IIsdnL3Internal& IL3/*, const CallRef &callref*/ ) 
        : Base(IL3.GetCallRefGen().GenCallRef(true), IL3)
    {
    }

    bool PacketRestartAck::ValidateMandatoryIes( const std::multiset<IeId>& exist ) const
    {
        return exist.find(IeRestartInd::GetId()) != exist.end();
    }

    void PacketRestartAck::UpRout( L3Call & ) 
    {
        ESS_HALT("PacketRestart rout not accepted.");
    }

    void PacketRestartAck::UpRout( IsdnL3& receiver )
    {
        receiver.ProcessGlobCrefPack(this);
    }

    bool PacketRestartAck::IeCorrectForThisMsgType( IeId id )
    {
        return 
            id == IeChannelIdentification::GetId() ||
            id == IeRestartInd::GetId() ||
            id == IeDisplay::GetId();
    }

} // namespace ISDN


