#include "stdafx.h"
#include "CmpMsgReceiver.h"

namespace iCmpExt
{

	// -------------------------------------------------------------------------------
	// iCmp::IPcSideRecv

	void CmpRespReceiver::OnBfRespGetRtpPort(int rtpPort)
	{
		std::ostringstream out;
		out << "OnBfRespGetRtpPort. Port: " << rtpPort;
		UnexpectedMsg(out.str());
	}

	// -------------------------------------------------------------------------------

	void CmpRespReceiver::OnBfResponse(int err, const std::string &desc)
	{
		std::ostringstream out;
		out << "OnBfResponse. Error code: " << err;
		if(!desc.empty()) out << desc;
		UnexpectedMsg(out.str());
	}

	// -------------------------------------------------------------------------------

	void CmpRespReceiver::OnBfRespGetTdmInfo(const iCmp::BfTdmInfo &info)
	{
		UnexpectedMsg("OnBfRespGetTdmInfo. ", info);
	}

	// -------------------------------------------------------------------------------

	void CmpRespReceiver::OnBfRespGetChannelInfo(const iCmp::BfChannelInfo &info)
	{
		UnexpectedMsg("OnBfRespGetChannelInfo. ", info);
	}

	// -------------------------------------------------------------------------------

	void CmpRespReceiver::OnBfRespCmdInit(const iCmp::BfInitInfo &info)
	{
		UnexpectedMsg("OnBfRespCmdInit. " + info.BuildString);
	}

	// -------------------------------------------------------------------------------

	void CmpRespReceiver::OnBfRespStateInfo(const iCmp::BfStateInfo &info)
	{
		UnexpectedMsg("OnBfRespStateInfo. ", info);
	}

	// -------------------------------------------------------------------------------

	void CmpRespReceiver::OnBfTdmEvent(const std::string &src, const std::string &event, const std::string &params)
	{
		ESS_HALT("Unexpected msg");
	}

	// -------------------------------------------------------------------------------

	void CmpRespReceiver::OnBfGetChannelData(const std::string &devName, int chNum, 
		const std::vector<Platform::byte> &rx, 
		const std::vector<Platform::byte> &tx,
		const std::vector<Platform::byte> &rxFixed)
	{
		ESS_HALT("Unexpected msg");
	}

    // -------------------------------------------------------------------------------

    void CmpRespReceiver::OnBfRespConf( const std::string &params )
    {
        UnexpectedMsg("OnBfRespConf. " + params);		
    }

	// -------------------------------------------------------------------------------
	// E1
	void CmpRespReceiver::OnBfHdlcRecv(const std::vector<Platform::byte> &pack)
	{
		ESS_HALT("Unexpected msg");
	}

	// -------------------------------------------------------------------------------

	void CmpRespReceiver::OnBfRespGetLiuInfo(const iCmp::BfLiuInfo &info)
	{
		UnexpectedMsg("OnBfRespGetLiuInfo. ", info);		
	}

	// -------------------------------------------------------------------------------
	// AOZ
	void CmpRespReceiver::OnBfAbEvent(const std::string &deviceName, int chNumber, 
		const std::string &eventName, const std::string &eventParams)
	{
		ESS_HALT("Unexpected msg");
	}

};

