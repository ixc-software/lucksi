#ifndef _CMP_CONF_RTP_PPOINT_CREATOR_H_
#define _CMP_CONF_RTP_PPOINT_CREATOR_H_

#include "Utils/WeakRef.h"
#include "CmpRespReceiver.h"
#include "CmpConfRtpPoint.h"

namespace iCmpExt
{

	class CmpConfRtpPointCreator : public CmpRespReceiver
	{
	public:

		CmpConfRtpPointCreator(Utils::WeakRef<ICmpConnection&> cmpConnection,
			Utils::WeakRef<ICmpConfRtpPointEvents&> user, 
			Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator,
			CmpConfRtpPointId rtpPointId,
			CmpHandleManager &handleManager) : 
			CmpRespReceiver(cmpConnection.Value()),
			m_cmpConnection(cmpConnection),
			m_user(user),
			m_rtpPointId(rtpPointId),
			m_handle(handleManager),
			m_logCreator(logCreator)
		{}

		int Handle() {	return m_handle.Handle(); }

	// iCmpExt::CmpRespReceiver
	private:
		void OnBfResponse(int err, const std::string &desc)
		{
			if(err != 0)  PointDeleted(iCmp::BfResponse::ResponseToString(err, desc));
		}

		void OnBfRespConf(const std::string &params)
		{
			if(!m_user.Valid() || !m_logCreator.Valid()) return;
			
			int port = 0;
			if(!iCmp::ChMngProtoParams::FromParams(params, port)) 
			{
				PointDeleted("Wrong response. " + params);
				return; 
			}

			ICmpConfRtpPoint *point = new CmpConfRtpPoint(m_user.Value(),
				m_cmpConnection,
				m_logCreator.Value(),
				m_handle,
				m_cmpConnection.Value().NameBoard(),
				Utils::HostInf(m_cmpConnection.Value().BoardHost().Address(), port));

			m_user.Value().PointCreated(m_rtpPointId, point);
		}

	private:
		void PointDeleted(const std::string &str)
		{
			if(m_user.Valid())  m_user.Value().PointDeleted(m_rtpPointId, str);
		}

	private:
		Utils::WeakRef<ICmpConnection&> m_cmpConnection;
		Utils::WeakRef<ICmpConfRtpPointEvents&> m_user;
		Utils::WeakRef<iLogW::ILogSessionCreator&> m_logCreator;
		CmpConfRtpPointId m_rtpPointId;
		CmpHandleWrapper m_handle;
	};

};

#endif






