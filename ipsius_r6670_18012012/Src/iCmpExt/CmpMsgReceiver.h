#ifndef _CMP_MSG_RECEIVER_H_
#define _CMP_MSG_RECEIVER_H_

#include "iCmp/ChMngProto.h"
#include "ICmpConnection.h"

namespace iCmpExt
{

	class CmpRespReceiver : boost::noncopyable,
		public iCmp::IPcSideRecv
	{
	public:
		CmpRespReceiver(ICmpConnection &cmpConnection) : 
			m_cmpConnection(cmpConnection){}
	
		virtual void OnBfRespGetRtpPort(int rtpPort);
		virtual void OnBfResponse(int err, const std::string &desc);
		virtual void OnBfRespGetTdmInfo(const iCmp::BfTdmInfo &info);
		virtual void OnBfRespGetChannelInfo(const iCmp::BfChannelInfo &info);
		virtual void OnBfRespCmdInit(const iCmp::BfInitInfo &info);
		virtual void OnBfRespStateInfo(const iCmp::BfStateInfo &info);
		virtual void OnBfTdmEvent(const std::string &src, const std::string &event, const std::string &params);
		
        virtual void OnBfGetChannelData(const std::string &devName, int chNum, 
            const std::vector<Platform::byte> &rx, 
            const std::vector<Platform::byte> &tx,
            const std::vector<Platform::byte> &rxFixed);

        virtual void OnBfRespConf(const std::string &params);

		// E1
		virtual void OnBfHdlcRecv(const std::vector<Platform::byte> &pack);
		virtual void OnBfRespGetLiuInfo(const iCmp::BfLiuInfo &info);

		// AOZ
		virtual void OnBfAbEvent(const std::string &deviceName, int chNumber, 
			const std::string &eventName, const std::string &eventParams);

	private:

//		virtual void UnexpectedMsg(const std::string &data, int) = 0;

		void UnexpectedMsg(const std::string &data)
		{
			m_cmpConnection.ResetConnection(data);
		}

	private:
		template <class Info> 
			void UnexpectedMsg(const std::string &title, const Info &info)
		{
			std::ostringstream out;
			out << title << info.ToString();
			UnexpectedMsg(out.str());
		}

	private:
		ICmpConnection &m_cmpConnection;
	};

};

#endif
