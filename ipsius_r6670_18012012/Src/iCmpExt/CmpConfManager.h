#ifndef _CMP_CONF_MANAGER_H_
#define _CMP_CONF_MANAGER_H_

#include "Utils/WeakRef.h"
#include "ICmpConnection.h"
#include "iCmpConfPoint.h"
#include "CmpConfTdmPoint.h"
#include "CmpConfGenPoint.h"
#include "CmpConfRtpPointCreator.h"
#include "CmpConf.h"
#include "SndMix/MixerCodec.h"

namespace iCmpExt
{
	
	class CmpConfManager : boost::noncopyable,
		public ICmpConfManager
	{
	public:
		CmpConfManager(Utils::WeakRef<ICmpConnection&> cmpConnection) : 
			m_cmpConnection(cmpConnection)
		{}

	// ICmpConfManager	
	private:
		
		iRtp::RtpPayloadList PayloadList() const
		{
			return  SndMix::CodecMng::GetPayloadList();
		}

		ICmpConf* CreateCmpConf(ICmpConfPointEvents &owner,
			iLogW::ILogSessionCreator &logCreator,
			bool autoMode, int blockSize = 160)
		{
			return new CmpConf(owner, m_cmpConnection, logCreator, m_confHandlers, autoMode, blockSize); 
		}

		ICmpConfPoint* CreateTdmPoint(ICmpConfPointEvents &owner, 
			iLogW::ILogSessionCreator &logCreator,
			int channel)
		{
			return new CmpConfTdmPoint(owner, m_cmpConnection, logCreator, m_pointHandlers, channel);
		}

		ICmpConfPoint* CreateGenPoint(ICmpConfPointEvents &owner, 
			iLogW::ILogSessionCreator &logCreator,
			const iDSP::Gen2xProfile &data)
		{
			return new CmpConfGenPoint(owner, m_cmpConnection, logCreator, m_pointHandlers, data); 
		}

		void CreateRtpPoint(CmpConfRtpPointId rtpPointId,
			Utils::WeakRef<ICmpConfRtpPointEvents&> owner,
			Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator, 
			const std::string &codecName, const std::string &codecParams)
		{
			CmpConfRtpPointCreator *creator = new CmpConfRtpPointCreator(m_cmpConnection, 
				owner, 
				logCreator, 
				rtpPointId, 
				m_pointHandlers);

			m_cmpConnection.Value().SbpSender(creator).
				Send<iCmp::PcCmdConfCreateRtpPoint>(creator->Handle(), codecName, codecParams);
		}

	private:
		Utils::WeakRef<ICmpConnection&> m_cmpConnection;
		CmpHandleManager m_confHandlers;
		CmpHandleManager m_pointHandlers;
	};
}

#endif
