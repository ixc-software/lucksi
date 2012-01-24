
#ifndef __SAFEBIPROTOTESTPARAMS__
#define __SAFEBIPROTOTESTPARAMS__

#include "SafeBiProto/SafeBiProto.h"

namespace SBPTests
{
	class ITestTransportsCreator;

    // -----------------------------------------------------------
    // Test params
    class SafeBiProtoTestParams
    {
        ITestTransportsCreator &m_creator;
        bool m_testWithMsgs;
        bool m_silentMode;
        bool m_clientFinished;
        bool m_serverFinished;
		SBProto::SbpSettings m_settings;
    public:
		// default value you can see in SafeBiProtoTestParams.cpp
        SafeBiProtoTestParams(ITestTransportsCreator &creator, bool testWithMsgs, 
                              bool silentMode, bool doNotProcessTimeouts = false);

        ITestTransportsCreator& getTransportsCreator() { return m_creator; }
        bool getSilentMode() const { return m_silentMode; }
        bool getTestWithMsgs() const { return m_testWithMsgs; }
        void setClientFinished() { m_clientFinished = true; }
        void setServerFinished() { m_serverFinished = true; }
        SBProto::SbpSettings& getSettings() { return m_settings; }
        void CheckFinished() { TUT_ASSERT(m_clientFinished && m_serverFinished); }
    };
   
} // namespace SBPTests


#endif
