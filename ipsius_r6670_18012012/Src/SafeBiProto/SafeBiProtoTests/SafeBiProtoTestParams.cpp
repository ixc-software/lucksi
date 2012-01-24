#include "stdafx.h"
#include "safebiprototestparams.h"

namespace 
{
    using namespace SBProto;

    const dword CCheckTimeoutIntervalMs = 1000;
    const dword CRecvEndTimeoutMs = 2 * 1000;
    const dword CResponceTimeoutMs = 1 * 1000;
};    

namespace SBPTests
{
	using namespace SBProto;

	SafeBiProtoTestParams::SafeBiProtoTestParams(ITestTransportsCreator &creator, bool testWithMsgs, 
	    bool silentMode, bool doNotProcessTimeouts) : 
		m_creator(creator), m_testWithMsgs(testWithMsgs), m_silentMode(silentMode), 
        m_clientFinished(false), m_serverFinished(false)
    {
        if (doNotProcessTimeouts) m_settings.setTimeoutCheckIntervalMs(0);
        else m_settings.setTimeoutCheckIntervalMs(CCheckTimeoutIntervalMs);

        m_settings.setReceiveEndTimeoutMs(CRecvEndTimeoutMs);
        m_settings.setResponceTimeoutMs(CResponceTimeoutMs);
    }
};

