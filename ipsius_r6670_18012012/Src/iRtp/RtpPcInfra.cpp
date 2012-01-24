#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "RtpPcInfra.h"
#include "RtpPcSocket.h"

// ---------------------------------------------------

namespace
{
    // RTP pool settings
    enum
    {
        CBuffSize = 1024 * 1, 
        CPreallocCount = 0, 
        CNewLimit = 2 * 1024,
    }; 

}  // namespace

// ---------------------------------------------------

namespace iRtp
{
	RtpPcInfra::RtpPcInfra(iCore::MsgThread &thread, const RtpInfraParams &params) : 
        m_buffCreator(CBuffSize, CPreallocCount, 0, CNewLimit),
        m_infra(thread, params, m_buffCreator)
	{}
} //namespace iRtp

