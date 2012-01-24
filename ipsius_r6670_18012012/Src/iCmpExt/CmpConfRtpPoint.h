#ifndef _CMP_CONF_RTP_PPOINT_H_
#define _CMP_CONF_RTP_PPOINT_H_

#include "CmpConfPoint.h"
#include "CmpHandleManager.h"
#include "CmpPointLog.h"
#include "Utils/IntToString.h"

namespace iCmpExt
{
	class CmpConfRtpPoint : public ICmpConfRtpPoint
	{
	public:
		CmpConfRtpPoint(ICmpConfPointEvents &owner,
			Utils::WeakRef<ICmpConnection&> cmpConnection,
			iLogW::ILogSessionCreator& logCreator,
			CmpHandleWrapper handle,
			const std::string &boardAlias,
			const Utils::HostInf &localRtp) :
			m_confPoint(this, owner, cmpConnection),
			m_localRtp(localRtp),
			m_handle(handle),
			m_log(logCreator, boardAlias + "_CmpConfRtpPoint_" + Utils::IntToString(Handle()))
		{
			if(m_log.LogActive()) m_log.Log(
				"Created. Board: " +  boardAlias + 
				". Local rtp: " + m_localRtp.ToString());
		}

		~CmpConfRtpPoint()
		{
			if(m_log.LogActive()) m_log.Log("Deleted");

			m_confPoint.Sender().Send<iCmp::PcCmdConfDeletePoint>(Handle());
		}


	// ICmpConfRtpPoint
	private:
		int Handle() const {	return m_handle.Handle(); }

		Utils::HostInf LocalRtp() const
		{
			return m_localRtp;
		}

		void RtpSend(const Utils::HostInf &dst)
		{
			ESS_ASSERT(m_localRtp.IsValid());

			if(m_log.LogActive()) m_log.Log("RtpSend to " + dst.ToString());

			m_confPoint.Sender().Send<iCmp::PcCmdConfRtpPointSend>(Handle(),
				dst.Address(), dst.Port());
		}

		void RtpStop()
		{
			if(m_log.LogActive()) m_log.Log("RtpStop");

			m_confPoint.Sender().Send<iCmp::PcCmdConfRtpPointSend>(Handle(), "", 0);
		}

	private:
		CmpConfPoint m_confPoint;
		Utils::HostInf m_localRtp;
		CmpHandleWrapper m_handle;
		CmpPointLog m_log;
	};
};

#endif







