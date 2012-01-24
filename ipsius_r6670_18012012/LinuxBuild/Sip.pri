INCLUDEPATH += $$MISC_PATH/resip/main

#HEADERS += Sip/NObjResipLog.h

#SOURCES += Sip/NObjResipLog.cpp



contains(IPSIUS_MODULE, SipOld) {

    debug{
        LIBS += \
                $$CONTRIB_LIB/libresipd.a 	\
                $$CONTRIB_LIB/librutild.a 	\
                $$CONTRIB_LIB/libdumd.a	\
                $$CONTRIB_LIB/libaresd.a  \
    }

    release{
        LIBS += \
                $$CONTRIB_LIB/libresip.a 	\
                $$CONTRIB_LIB/librutil.a 	\
                $$CONTRIB_LIB/libdum.a	\
                $$CONTRIB_LIB/libares.a  \
    }


    HEADERS += \
        Sip/NObjGateSip.h			\
        Sip/NobjProxySip.h			\
        Sip/ProfileSip.h

    SOURCES +=  \
        Sip/CallManagerSip.cpp 			    \
        Sip/ClientRegistrationHandler.cpp 	\
        Sip/ConversationProfile.cpp 		\
        Sip/EndPointMasterProfile.cpp 		\
        Sip/GateInviteDialogSet.cpp 		\
        Sip/GateNoInviteDialogSet.cpp 		\
        Sip/GateSipCall.cpp                 \
        Sip/InviteSessionWrapper.cpp 		\
        Sip/MsgGateSipCall.cpp              \
        Sip/NObjGateSip.cpp                 \
        Sip/NobjProxySip.cpp                \
        Sip/ProfileSip.cpp                  \
        Sip/SdpHandler.cpp                  \
        Sip/SipMessageHandler.cpp           \
        Sip/SipRtpParams.cpp                \
        Sip/WrapperDum.cpp
}

