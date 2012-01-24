#ifndef _CMP_CONF_RECEIVER_H_
#define _CMP_CONF_RECEIVER_H_


#include "Utils/WeakRef.h"
#include "CmpRespReceiver.h"

namespace iCmpExt
{

	class ICmpConfResponseReceiver : public Utils::IBasicInterface
	{
	public:
		virtual void BfRespConf(const std::string &params) = 0;
		virtual void ErrorOccur(const std::string &desc) = 0;
	};

	class CmpConfResponseReceiver : public CmpRespReceiver
	{
	public:
		CmpConfResponseReceiver(ICmpConnection &unexpectedMsgHandler,
			Utils::WeakRef<ICmpConfResponseReceiver*> owner) : 
			iCmpExt::CmpRespReceiver(unexpectedMsgHandler),
			m_owner(owner)
		{}
	
	// iCmpExt::CmpRespReceiver
	private:
		void OnBfResponse(int err, const std::string &desc)
		{
			if(!m_owner.Valid() || err == 0)  return;
				
			m_owner.Value()->ErrorOccur(iCmp::BfResponse::ResponseToString(err, desc));
		}

		void OnBfRespConf(const std::string &params)
		{
			if(m_owner.Valid()) m_owner.Value()->BfRespConf(params);
		}

	private:
		Utils::WeakRef<ICmpConfResponseReceiver*> m_owner;
	};

};

#endif







