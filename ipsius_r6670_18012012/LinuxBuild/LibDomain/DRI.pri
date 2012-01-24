
HEADERS += \
        DRI/TelnetKeys.h            \
        DRI/NObjTelnetServerDRI.h   \

SOURCES += \ 
	DRI/CommandDRI.cpp		\
	DRI/DriArgFactory.cpp 		\
	DRI/DRICommandStreamParser.cpp 	\
	DRI/DriFileLoad.cpp		\
	DRI/DRIParserHelpers.cpp 	\
	DRI/ExecutiveDRI.cpp 		\
	DRI/ParsedDRICmd.cpp 		\
	DRI/TransactionDRI.cpp 		\
    DRI/DriHintDb.cpp		\
    DRI/DriSessionList.cpp		\
	DRI/MetaInvoke.cpp 		\
	DRI/SessionDRI.cpp		\
	DRI/TelnetServerSessionDRI.cpp	\
	DRI/DriIncludeDirList.cpp	\
	DRI/DriDefineAndMacroList.cpp	\
	DRI/DriPropertyFactory.cpp	\
	DRI/MetaObjectsInfo.cpp		\
    DRI/NObjTelnetServerDRI.cpp

SOURCES += DriTests/DRITestHelpers.cpp

contains(IPSIUS_MODULE, DriTest) {
    include(../LinuxBuild/iSip.pri)
    HEADERS += DriTests/NamedObjectTestType.h

    SOURCES += \
        DriTests/DriIncludeDirListTest.cpp      \
        DriTests/DRIScriptTests.cpp             \
        DriTests/DRITestHelpers.cpp             \
        DriTests/NamedObjectTestType.cpp        \
        DriTests/DRIParserTests.cpp             \
        DriTests/DRITest.cpp                    \
        DriTests/DriTestsGroup.cpp              \
        #DriTests/NamedObjectTestType_meta.cpp
}
