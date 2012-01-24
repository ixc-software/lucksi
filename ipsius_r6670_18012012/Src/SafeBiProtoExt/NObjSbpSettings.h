
#ifndef _NOBJ_SBPSETTINGS__
#define _NOBJ_SBPSETTINGS__

#include "Platform/Platform.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "SafeBiProto/SbpSettings.h"
#include "SafeBiProtoExt/SbpLogSettings.h"

namespace SBProtoExt
{
	class NObjSbpSettings : public Domain::NamedObject, 
		public DRI::INonCreatable,
		SBProto::SbpSettings
	{
		Q_OBJECT;
	public:
		NObjSbpSettings(Domain::NamedObject *pParent, 
			const Domain::ObjectName &name = "SbpSettings") :
			NamedObject(&pParent->getDomain(), name, pParent),
			m_logSettings(new SbpLogSettings())
		{
			PropertyWriteEvent(pParent);
		}

		SBProto::SbpSettings Settings() const
		{
			return *this;
		}

		boost::shared_ptr<SbpLogSettings> LogSettings() const
		{
			return m_logSettings;
		}

		Q_PROPERTY(int Version READ getCurrentProtoVersion);
		Q_PROPERTY(int PacketMaxSize READ getMaxSendSize);
		Q_PROPERTY(int ResponceTimeout READ getResponceTimeoutMs WRITE setResponceTimeoutMs);
		Q_PROPERTY(int ReceiveEndTimeout READ getReceiveEndTimeoutMs WRITE setReceiveEndTimeoutMs);
		Q_PROPERTY(int PoolTimeout READ getPoolTimeout WRITE setPoolTimeout);
		Q_PROPERTY(int TimeoutCheckInterval READ getTimeoutCheckIntervalMs WRITE setTimeoutCheckIntervalMs);
		Q_PROPERTY(int TraceCountPar READ CountPar WRITE CountPar);		
		Q_PROPERTY(int TraceBinMaxBytes READ BinMaxBytes WRITE BinMaxBytes);

	private:
		int CountPar() const {	return m_logSettings->CountPar(); }
		void CountPar(int val) {	m_logSettings->CountPar(val); }

		int BinMaxBytes() const {	return m_logSettings->BinaryMaxBytes(); }
		void BinMaxBytes(int val) {	m_logSettings->BinaryMaxBytes(val); }

	private:
		boost::shared_ptr<SbpLogSettings> m_logSettings;
    };
    
} // namespace SBProto

#endif 
