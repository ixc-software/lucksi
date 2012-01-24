#include "stdafx.h"
#include "iCmp/ChMngProto.h"
#include "NObjCmpDevSettings.h"

namespace iCmpExt
{
	NObjCmpDevSettings::NObjCmpDevSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name,
		iCmp::BfInitDataBase &data) :
		NamedObject(&pParent->getDomain(), name, pParent),
		m_data(data)
	{
		PropertyWriteEvent(pParent);
		m_echoTape = 0;
		m_innerEcho = false;
		iCmp::BfInitDataBase::SetDefault(m_data);
	}

	// -------------------------------------------------------------------------------

	bool NObjCmpDevSettings::OnPropertyWrite(bool isStarted, const QString &propertyName, const QString &val) const
	{
		return !isStarted || 
			(propertyName == "EchoTape" || propertyName == "IsInnerEcho");
	}

	// -------------------------------------------------------------------------------

	bool NObjCmpDevSettings::IsValid() const
	{
		return m_data.IsValid();
	}

	// -------------------------------------------------------------------------------

	int NObjCmpDevSettings::EchoTape() const
	{
		return m_echoTape;
	}

	// -------------------------------------------------------------------------------

	bool NObjCmpDevSettings::UseInternalEcho() const
	{
		return m_innerEcho;
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::StartRtpPort() const
	{
		return m_data.StartRtpPort; 
	}

	void NObjCmpDevSettings::StartRtpPort(int val)
	{
		if(val & 1) ThrowRuntimeException("Rtp port mast be even.");

		m_data.StartRtpPort = val; 
	}

	// --------------------------------------------------------------------------

	bool NObjCmpDevSettings::UseAlaw() const
	{
		return m_data.UseAlaw;
	}
	void NObjCmpDevSettings::UseAlaw(bool val)
	{
		m_data.UseAlaw = val;
	}

	// --------------------------------------------------------------------------

	void NObjCmpDevSettings::TestBodyBaseException(bool val)
	{
		m_data.TestBodyBaseException = val;
	}

	bool NObjCmpDevSettings::TestBodyBaseException() const
	{
		return m_data.TestBodyBaseException;
	}

	// --------------------------------------------------------------------------

	void NObjCmpDevSettings::ThrowFromInit(bool val)
	{
		m_data.ThrowFromInit = val;
	}

	bool NObjCmpDevSettings::ThrowFromInit() const
	{
		return m_data.ThrowFromInit;
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::Sport0BlocksCount() const
	{
		return m_data.Sport0BlocksCount;
	}
	void NObjCmpDevSettings::Sport0BlocksCount(int val)
	{
		m_data.Sport0BlocksCount = val;
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::Sport0BlockCapacity() const
	{
		return m_data.Sport0BlockCapacity;
	}
	void NObjCmpDevSettings::Sport0BlockCapacity(int val)
	{
		if(val % 8 == 0) ThrowRuntimeException("Sport block capacity mast be multiple to 8.");

		m_data.Sport0BlockCapacity = val;
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::Sport1BlocksCount() const
	{
		return m_data.Sport1BlocksCount;
	}
	void NObjCmpDevSettings::Sport1BlocksCount(int val)
	{
		m_data.Sport1BlocksCount = val;
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::Sport1BlockCapacity() const
	{
		return m_data.Sport1BlockCapacity;
	}
	void NObjCmpDevSettings::Sport1BlockCapacity(int val)
	{
		if(val % 8 == 0) ThrowRuntimeException("Sport block capacity mast be multiple to 8.");

		m_data.Sport1BlockCapacity = val;
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::VoiceSportNum() const
	{
		return m_data.VoiceSportNum;
	}
	void NObjCmpDevSettings::VoiceSportNum(int val)
	{
		m_data.VoiceSportNum = val;
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::VoiceSportChNum() const
	{
		return m_data.VoiceSportChNum;
	}		
	void NObjCmpDevSettings::VoiceSportChNum(int val)
	{
		m_data.VoiceSportChNum = val;
	}

	// --------------------------------------------------------------------------
	// TdmMng::RtpRecvBufferProfile 

	int NObjCmpDevSettings::RtpMinBufferingDepth() const	
	{	
		return m_data.RtpRecvProfile.MinBufferingDepth; 
	}
	void NObjCmpDevSettings::RtpMinBufferingDepth(int val)	
	{	
		if(val < 1) ThrowRuntimeException("Value mast be more or equal then 1.");

		if(val < m_data.RtpRecvProfile.DropCount) ThrowRuntimeException("Value mast be more then DropCount.");

		if(val >= m_data.RtpRecvProfile.MaxBufferingDepth) 
		{
			ThrowRuntimeException("Value mast be less then MaxBufferingDepth.");
		}

		m_data.RtpRecvProfile.MinBufferingDepth = val;  
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::RtpMaxBufferingDepth() const	
	{	
		return m_data.RtpRecvProfile.MaxBufferingDepth; 
	}
	void NObjCmpDevSettings::RtpMaxBufferingDepth(int val)	
	{	
		if(val < 1) ThrowRuntimeException("Value mast be more or equal then 1.");

		if(val <= m_data.RtpRecvProfile.MinBufferingDepth) 
		{
			ThrowRuntimeException("Value mast be more then MinBufferingDepth.");
		}

		m_data.RtpRecvProfile.MaxBufferingDepth = val;  
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::RtpUpScalePercent() const		
	{	
		return m_data.RtpRecvProfile.UpScalePercent; 
	}
	void NObjCmpDevSettings::RtpUpScalePercent(int val)		
	{	
		if(val < 1) ThrowRuntimeException("Value mast be less then 100%.");

		m_data.RtpRecvProfile.UpScalePercent = val; 
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::RtpQueueDepth() const			
	{	
		return m_data.RtpRecvProfile.QueueDepth;
	}
	void NObjCmpDevSettings::RtpQueueDepth(int val)			
	{	
		if(val < 1) ThrowRuntimeException("Value mast be more or equal then 1.");

		m_data.RtpRecvProfile.QueueDepth = val; 
	}

	// --------------------------------------------------------------------------

	int NObjCmpDevSettings::RtpDropCount() const			
	{	
		return m_data.RtpRecvProfile.DropCount; 
	}
	void NObjCmpDevSettings::RtpDropCount(int val)			
	{	
		if(val < 1) ThrowRuntimeException("Value mast be more or equal then 1.");

		if(val > m_data.RtpRecvProfile.MinBufferingDepth) 
		{
			ThrowRuntimeException("Value mast be more then MinBufferingDepth.");
		}

		m_data.RtpRecvProfile.DropCount = val; 
	}

};


