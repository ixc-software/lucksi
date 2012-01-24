#ifndef _NOBJ_CMP_DEV_SETTINGS_H_
#define _NOBJ_CMP_DEV_SETTINGS_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "iCmp/ChMngProto.h"

namespace iCmp { class BfInitDataBase; };

namespace iCmpExt
{
	class NObjCmpDevSettings : public Domain::NamedObject,
		public DRI::INonCreatable
	{
		Q_OBJECT;
	public:
		NObjCmpDevSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name,
			iCmp::BfInitDataBase &data);

		bool OnPropertyWrite(bool isStarted, const QString &propertyName, const QString &val) const;

		Q_PROPERTY(int StartRtpPort READ StartRtpPort WRITE StartRtpPort);

		Q_PROPERTY(bool UseAlaw READ UseAlaw WRITE UseAlaw);
		bool UseAlaw() const;

		Q_PROPERTY(bool TestBodyBaseException READ TestBodyBaseException WRITE TestBodyBaseException);
		Q_PROPERTY(bool ThrowFromInit READ ThrowFromInit WRITE ThrowFromInit);

		Q_PROPERTY(int Sport0BlocksCount READ Sport0BlocksCount WRITE Sport0BlocksCount);
		Q_PROPERTY(int Sport0BlockCapacity READ Sport0BlockCapacity WRITE Sport0BlockCapacity);
		Q_PROPERTY(int Sport1BlocksCount READ Sport1BlocksCount WRITE Sport1BlocksCount);
		Q_PROPERTY(int Sport1BlockCapacity READ Sport1BlockCapacity WRITE Sport1BlockCapacity);
		Q_PROPERTY(int VoiceSportNum READ VoiceSportNum WRITE VoiceSportNum);
		Q_PROPERTY(int VoiceSportChNum READ VoiceSportChNum WRITE VoiceSportChNum);

		Q_PROPERTY(int RtpMinBufferingDepth READ RtpMinBufferingDepth WRITE RtpMinBufferingDepth);
		Q_PROPERTY(int RtpMaxBufferingDepth READ RtpMaxBufferingDepth WRITE RtpMaxBufferingDepth);
		Q_PROPERTY(int RtpUpScalePercent READ RtpUpScalePercent WRITE RtpUpScalePercent);
		Q_PROPERTY(int RtpQueueDepth READ RtpQueueDepth WRITE RtpQueueDepth);
		Q_PROPERTY(int RtpDropCount READ RtpDropCount WRITE RtpDropCount);

		Q_PROPERTY(int EchoTape READ m_echoTape WRITE m_echoTape);
		Q_PROPERTY(bool UseInternalEcho READ m_innerEcho WRITE m_innerEcho);

		void InitSettings(iCmp::BfInitDataBase &par) const
		{
			par = m_data;
		}

		bool IsValid() const;
		int EchoTape() const;
		bool UseInternalEcho() const;

	private:
		int StartRtpPort() const;
		void StartRtpPort(int val);
		void UseAlaw(bool val);

		void TestBodyBaseException(bool par);
		bool TestBodyBaseException() const;
		
		void ThrowFromInit(bool par);
		bool ThrowFromInit() const;

		int Sport0BlocksCount() const;
		void Sport0BlocksCount(int val);
		int Sport0BlockCapacity() const;
		void Sport0BlockCapacity(int val);
		int Sport1BlocksCount() const;
		void Sport1BlocksCount(int val);
		int Sport1BlockCapacity() const;
		void Sport1BlockCapacity(int val);
		int VoiceSportNum() const;
		void VoiceSportNum(int val);
		int VoiceSportChNum() const;
		void VoiceSportChNum(int val);

	// TdmMng::RtpRecvBufferProfile
	private:
		int RtpMinBufferingDepth() const;
		void RtpMinBufferingDepth(int val);
		int RtpMaxBufferingDepth() const;
		void RtpMaxBufferingDepth(int val);
		int RtpUpScalePercent() const;
		void RtpUpScalePercent(int val);
		int RtpQueueDepth() const;
		void RtpQueueDepth(int val);
		int RtpDropCount() const;
		void RtpDropCount(int val);

	private:
		iCmp::BfInitDataBase &m_data;
		int m_echoTape;
		bool m_innerEcho;
	};
};

#endif


