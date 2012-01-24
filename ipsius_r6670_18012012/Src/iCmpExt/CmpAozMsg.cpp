#include "stdafx.h"
#include "CmpAozMsg.h"
#include "ICmpConnection.h"
#include "CmpRespReceiver.h"

namespace 
{
	class SimpleResp : public iCmpExt::CmpRespReceiver
	{
	public:

		SimpleResp(iCmpExt::ICmpConnection &connection) : 
			iCmpExt::CmpRespReceiver(connection)
		{}

	// iCmpExt::CmpRespReceiver
	private:
		void OnBfResponse(int err, const std::string &desc)
		{
			if(err == 0) return;
			
			UnexpectedResp(iCmp::BfResponse::ResponseToString(err, desc));
		}
	};

	class DtmfResp : public iCmpExt::CmpRespReceiver
	{
	public:
		DtmfResp(boost::function<void (bool, const std::string &)> result,
			iCmpExt::ICmpConnection &connection) : 
			iCmpExt::CmpRespReceiver(connection),
			m_result(result)
		{}

    // iCmpExt::CmpRespReceiver
	private:
		void OnBfResponse(int err, const std::string &desc)
		{
			if(err == 0) 
				m_result(true, "");
			else
				m_result(false, iCmp::BfResponse::ResponseToString(err, desc));
		}

	private:
		boost::function<void (bool, const std::string &)> m_result;

	};

}

namespace iCmpExt
{

	void CmpAozMsg::CofidecSetup(int chMask, const iCmp::AbCofidecParams &params)
	{
		m_connection.SbpSender(new SimpleResp(m_connection)).
			Send<iCmp::PcCmdAbSetCofidecSetup>(m_connection.NameBoard(), chMask, params);
	}

	// -------------------------------------------------------------------------------

	void CmpAozMsg::LineSetup(int chMask, const iCmp::AbLineParams &params)
	{
		m_connection.SbpSender(new SimpleResp(m_connection)).
			Send<iCmp::PcCmdAbSetLineParams>(m_connection.NameBoard(), chMask, params);
	}

	// -------------------------------------------------------------------------------

	void CmpAozMsg::DtmfSetup(boost::function<void (bool, const std::string &)> result,
		int chMask, bool useInternal,
		iCmp::FreqRecvMode mode, 
		const std::string &params)
	{

		SBProtoExt::SbpMsgSender sender = 
			m_connection.SbpSender(new DtmfResp (result, m_connection));
		
		sender.Send<iCmp::PcCmdSetFreqRecvMode>(m_connection.NameBoard(), 
			chMask, 
			useInternal, 
			iCmp::PcCmdSetFreqRecvMode::CRecvDTMF(), 
			mode,
			params);
	}


}




