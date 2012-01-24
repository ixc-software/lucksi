#include "stdafx.h"
#include "CmpBoardMsg.h"
#include "ICmpConnection.h"
#include "CmpRespReceiver.h"
#include "iCmp/ChMngProto.h"
#include "NObjCmpConnection.h"
namespace 
{
	using namespace iCmpExt;

	class GlobalSetupResp : public CmpRespReceiver
	{
	public:
		GlobalSetupResp(boost::function<void (void)> setupComplete,
			iCmpExt::ICmpConnection &unexpectedMsgHandler) : 
			iCmpExt::CmpRespReceiver(unexpectedMsgHandler),
			m_setupComplete(setupComplete)
		{
			ESS_ASSERT(!m_setupComplete.empty());
		}

	private:
		void OnBfResponse(int err, const std::string &desc)
		{
			if (err == 0) 
				m_setupComplete();
			else
				UnexpectedResp(iCmp::BfResponse::ResponseToString(err, desc));
		}

	private:
		boost::function<void (void)> m_setupComplete;
	};

	// -------------------------------------------------------------------------------

	class InitResp : public CmpRespReceiver
	{
	public:
		InitResp(boost::function<void (const iCmp::BfInitInfo&)> initComplete,
			iCmpExt::ICmpConnection &unexpectedMsgHandler) : 
			iCmpExt::CmpRespReceiver(unexpectedMsgHandler),
			m_initComplete(initComplete)
		{
			ESS_ASSERT(!m_initComplete.empty());
		}

		// iCmpExt::CmpRespReceiver
	private:
		void OnBfRespCmdInit(const iCmp::BfInitInfo &info)
		{
			// initialization is completed
			m_initComplete(info);
		}

	private:
		boost::function<void (const iCmp::BfInitInfo &)> m_initComplete;
	};

	// -------------------------------------------------------------------------------

	class AsyncCmpRespReceiver : public CmpRespReceiver
	{
		typedef AsyncCmpRespReceiver T;

	public:
		AsyncCmpRespReceiver(CmpInfoReqMsg::Result result, CmpInfoReqMsg::Result error) : 
			CmpRespReceiver(boost::bind(&T::UnexpectedMsg, this, _1), true),
			m_result(result),
			m_error(error)
			{}

	// iCmpExt::CmpRespReceiver
	private:
		void OnBfResponse(int errCode, const std::string &desc)
		{
			m_error(iCmp::BfResponse::ResponseToString(errCode, desc));
		}

		void OnBfRespGetTdmInfo(const iCmp::BfTdmInfo &info)
		{
			m_result(info.ToString("\n"));
		}

		void OnBfRespGetChannelInfo(const iCmp::BfChannelInfo &info)
		{
			m_result(info.ToString("\n"));
		}

		void OnBfRespStateInfo(const iCmp::BfStateInfo &info)
		{
			m_result(info.ToString("\n"));
		}

		void OnBfRespGetLiuInfo(const iCmp::BfLiuInfo &info)
		{
			m_result(info.ToString("\n"));
		}

		void OnBfRespConf(const std::string &params)
		{
			m_result(params);
		}

	private:
		void UnexpectedMsg(const std::string &data)
		{
			m_error(data);
		}

	private:
		CmpInfoReqMsg::Result m_result;
		CmpInfoReqMsg::Result m_error;
	};

};


namespace iCmpExt
{

	void CmpBoardMsg::GlobalSetup(boost::function<void (void)> setupComplete, 
		const iCmp::BfGlobalSetup &data)
	{
		m_connection.SbpSender(new GlobalSetupResp(setupComplete, m_connection)).
			Send<iCmp::PcCmdGlobalSetup>(data);
	}

	// -------------------------------------------------------------------------------

	SBProtoExt::SbpMsgSender 
		CmpBoardMsg::BoardInit(boost::function<void (const iCmp::BfInitInfo&)> initComplete)
	{
		return m_connection.SbpSender(new InitResp(initComplete, m_connection));
	}

	// -------------------------------------------------------------------------------

	void CmpInfoReqMsg::StateInfo()
	{
		m_connection.SbpSender(new AsyncCmpRespReceiver(m_result, m_error)).
			Send<iCmp::PcCmdStateInfo>();
	}

	// -------------------------------------------------------------------------------

	void CmpInfoReqMsg::LiuInfo()
	{
		m_connection.SbpSender(new AsyncCmpRespReceiver(m_result, m_error)).
			Send<iCmp::PcCmdGetLiuInfo>(m_connection.NameBoard());
	}

	// -------------------------------------------------------------------------------

	void CmpInfoReqMsg::TdmInfo()
	{
		m_connection.SbpSender(new AsyncCmpRespReceiver(m_result, m_error)).
			Send<iCmp::PcCmdGetTdmInfo>(m_connection.NameBoard());
	}

	// -------------------------------------------------------------------------------

	void CmpInfoReqMsg::ChannelInfo(int chNum, bool lastCall)
	{
		m_connection.SbpSender(new AsyncCmpRespReceiver(m_result, m_error)).
			Send<iCmp::PcCmdGetChannelInfo>(m_connection.NameBoard(), chNum, lastCall);
	}

	// -------------------------------------------------------------------------------

	void CmpInfoReqMsg::ConferenceInfo(int confHandle)
	{
		m_connection.SbpSender(new AsyncCmpRespReceiver(m_result, m_error)).
			Send<iCmp::PcCmdConfGetState>(confHandle);
	}

}

