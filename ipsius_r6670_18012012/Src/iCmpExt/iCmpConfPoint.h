#ifndef _I_CMP_CONF_POINT_H_
#define _I_CMP_CONF_POINT_H_

#include "Utils/HostInf.h"
#include "Utils/SafeRef.h"
#include "Utils/WeakRef.h"
#include "iDSP/GenUtils.h"
#include "iRtp/RtpPayload.h"

namespace iLogW {	class ILogSessionCreator; }

namespace iCmpExt
{

	// -------------------------------------------------------
    // ������� ����� ��� ���� ����������� �����������

	class ICmpConfPoint : public Utils::IBasicInterface
	{
	public:
		virtual int Handle() const = 0;
	};

	// -------------------------------------------------------
	// ��������� ������������ 

    class ICmpConfPointEvents : public Utils::IBasicInterface
	{
	public:
		virtual void PointDeleted(const ICmpConfPoint *point, const std::string &desc) = 0;
	};

	// -------------------------------------------------------
	// ��������� Rtp ����� �����������

	class ICmpConfRtpPoint : virtual public ICmpConfPoint
	{
	public:
		virtual Utils::HostInf LocalRtp() const = 0;
		virtual void RtpSend(const Utils::HostInf &dst) = 0;
		virtual void RtpStop() = 0;
	};

	// ��������� ������������ Rtp ����� �����������

	typedef const void* CmpConfRtpPointId;

	class ICmpConfRtpPointEvents : public ICmpConfPointEvents 
	{
	public:
		virtual void PointCreated(CmpConfRtpPointId rtpPointId, ICmpConfRtpPoint *rtpPoint) = 0;
		virtual void PointDeleted(CmpConfRtpPointId pointId, const std::string &desc) = 0;
	};


	// -------------------------------------------------------
	// ��������� �����������

	class ICmpConf : public ICmpConfPoint
	{
	public:
		virtual void ConfMode(const std::string &mode) = 0;
		virtual void AddPoint(const ICmpConfPoint &point, bool send, bool recv) = 0;
		virtual void RemovePoint(const ICmpConfPoint &point) = 0;
	};

	// -------------------------------------------------------
	// ��������� ��������� �����������

	class ICmpConfManager : public Utils::IBasicInterface
	{
	public:
		
		virtual iRtp::RtpPayloadList PayloadList() const = 0;

		virtual ICmpConf* CreateCmpConf(
			ICmpConfPointEvents &owner,
			iLogW::ILogSessionCreator &logCreator,
			bool autoMode, 
			int blockSize = 160) = 0;

		virtual ICmpConfPoint* CreateTdmPoint(
			ICmpConfPointEvents &owner, 
			iLogW::ILogSessionCreator &logCreator,
			int channel) = 0;

		virtual ICmpConfPoint* CreateGenPoint(
			ICmpConfPointEvents &owner, 
			iLogW::ILogSessionCreator &logCreator, 
			const iDSP::Gen2xProfile &data) = 0;

		virtual void CreateRtpPoint(
			CmpConfRtpPointId rtpPointId,
			Utils::WeakRef<ICmpConfRtpPointEvents&> owner,
			Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator, 
			const std::string &codecName, const std::string &codecParams) = 0;

	};

	class ICmpManager : Utils::IBasicInterface
	{
	public:
		virtual ICmpConfManager& ConfManager() = 0;
	};

}

#endif
