#ifndef _CMP_BOARD_MSG_
#define _CMP_BOARD_MSG_

#include "SafeBiProtoExt/SbpMsgSender.h"

namespace iCmp
{
	struct BfInitInfo;
	class BfGlobalSetup;
}

namespace DRI {	class IAsyncCmd; }

namespace iCmpExt
{
	class ICmpConnection;
	class CmpTemp;

	class CmpBoardMsg
	{
	public:
		CmpBoardMsg(ICmpConnection &connection) :
			m_connection(connection)
		{}

		void GlobalSetup(boost::function<void (void)> setupComplete, 
			const iCmp::BfGlobalSetup &data);
		
		SBProtoExt::SbpMsgSender 
			BoardInit(boost::function<void (const iCmp::BfInitInfo&)> initComplete);

	private:
		ICmpConnection &m_connection;

	};

	class CmpInfoReqMsg
	{
	public:
		typedef boost::function<void (const std::string &)> Result;

		CmpInfoReqMsg(ICmpConnection &connection,
			Result result, 
			Result error) :
	    	m_connection(connection),
			m_result(result),
			m_error(error)
		  {}
		
		void StateInfo();

		void LiuInfo();

		void TdmInfo();

		void ChannelInfo(int chNum, bool lastCall);

		void ConferenceInfo(int confHandle);

	private:
		ICmpConnection &m_connection;
		Result m_result; 
		Result m_error;
	};
}

#endif



