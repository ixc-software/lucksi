#ifndef _ASYNC_CMP_RESP_RECEIVER_H_
#define  _ASYNC_CMP_RESP_RECEIVER_H_

#include "CmpMsgReceiver.h"

namespace iCmpExt
{
	class IAsyncCmpRespOwner : Utils::IBasicInterface
	{
	public:
		virtual void ReqToBoardIncomplete(const std::string &desc) = 0;
		virtual void ReqToBoardComplete(const std::string &info) = 0;
	};

	// -------------------------------------------------------------------------------

	class AsyncCmpRespReceiver : IUnexpectedMsgHandler,
		public CmpRespReceiver
	{
	public:
		AsyncCmpRespReceiver(IUnexpectedMsgHandler &v, IAsyncCmpRespOwner &user) : 
			CmpRespReceiver(*static_cast<IUnexpectedMsgHandler*>(this)),
			m_user(user)
		{}
	// iCmpExt::CmpMsgReceiver
	private:
		void OnBfResponse(int errCode, const std::string &desc)
		{
			m_user.ReqToBoardIncomplete(iCmp::BfResponse::ResponseToString(errCode, desc));
		}
	
		void OnBfRespGetTdmInfo(const iCmp::BfTdmInfo &info)
		{
			m_user.ReqToBoardComplete(info.ToString("\n"));
		}

		void OnBfRespGetChannelInfo(const iCmp::BfChannelInfo &info)
		{
			m_user.ReqToBoardComplete(info.ToString("\n"));
		}

		void OnBfRespStateInfo(const iCmp::BfStateInfo &info)
		{
			m_user.ReqToBoardComplete(info.ToString("\n"));
		}

		void OnBfRespGetLiuInfo(const iCmp::BfLiuInfo &info)
		{
			m_user.ReqToBoardComplete(info.ToString("\n"));
		}
	private:
		void UnexpectedMsg(const std::string &data)
		{
			m_user.ReqToBoardIncomplete(data);
		}
	private:
		IAsyncCmpRespOwner &m_user;
	};
};

#endif
