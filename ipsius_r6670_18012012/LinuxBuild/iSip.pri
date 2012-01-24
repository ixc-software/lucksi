
LIBS += \
    $$RESIPLIB_PATH/libresip.a 	\
    $$RESIPLIB_PATH/librutil.a 	\
    $$RESIPLIB_PATH/libdum.a     \

LIBS += $$ARES_LIB

HEADERS += \
    iSip/ForwardingProfile.h        \
    iSip/NObjSipAccessPoint.h       \
    iSip/NObjSipAutotrainer.h       \
    iSip/NObjSipOutCallProfile.h    \
    iSip/NObjSipTrustedNodes.h      \
    iSip/NObjSipUserAgent.h         \
    iSip/NObjResipLog.h             \
    iSip/SipUtils.h                 \
    iSip/SipRegistrationState.h     \
    iSip/SipTransportInfo.h

SOURCES +=  \
    iSip/Sdp/Sdp.cpp                \
    iSip/Sdp/SdpCandidate.cpp       \
    iSip/Sdp/SdpCandidatePair.cpp   \
    iSip/Sdp/SdpCodec.cpp           \
    iSip/Sdp/SdpHelperResip.cpp     \
    iSip/Sdp/SdpMediaLine.cpp

SOURCES +=  \
    iSip/AutotrainerSipCall.cpp             \
    iSip/ClientSubscriptionHandler.cpp      \
    iSip/ConversationManager.cpp            \
    iSip/ConversationProfile.cpp            \
    iSip/InviteSessionHandler.cpp           \
    iSip/MediaLine.cpp                      \
    iSip/MediaLineList.cpp                  \
    iSip/MsgRemoteParticipant.cpp           \
    iSip/NObjSipAccessPoint.cpp             \
    iSip/NObjSipAutotrainer.cpp             \
    iSip/NObjSipTrustedNodes.cpp            \
    iSip/NObjSipUserAgent.cpp               \    
    iSip/NObjResipLog.cpp                   \
    iSip/RegistrationHandler.cpp            \
    iSip/RemoteParticipant.cpp              \
    iSip/RemoteParticipantDialogSet.cpp     \
    iSip/SipUtils.cpp                       \
    iSip/SdpUtils.cpp                       \
    iSip/ServerSubscriptionHandler.cpp      \
    iSip/SipMessageHelper.cpp               \
    iSip/SipTrustedNode.cpp                 \
    iSip/UserAgentClientSubscription.cpp    \
    iSip/UserAgent.cpp                      \
    iSip/UserAgentDialogSetFactory.cpp      \
    iSip/UserAgentMasterProfile.cpp         \
    iSip/UserAgentRegistrator.cpp           \
    iSip/UserAgentServerAuthManager.cpp




