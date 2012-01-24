
HEADERS += \ 
	UtilsTests/QtEnumResolverExample.h 	\
	DriTests/NamedObjectTestType.h 		\

SOURCES += \
	FinalChecks.cpp \
# test groups \
	iCoreTests/iCoreGroupBasic.cpp \
	UtilsTests/UtilsTestGroup.cpp \
        UtilsTests/ErrorsSubsystemTests.cpp \
	UtilsTests/ErrorGenGroup.cpp \
	PlatformTests/PlatformTestsGroup.cpp \
        TelnetTests/TelnetTestGroup.cpp \
	iNetTests/iNetTests.cpp \
	TestFw/TestFwGroup.cpp \
	iLog/iLogTestGroup.cpp \
	SafeBiProto/SafeBiProtoTests/SafeBiProtoTestGroup.cpp \
	DriTests/DriTestsGroup.cpp \
# TestFw \
	TestFw/TestFw.cpp \
        TestFw/TestFwOutput.cpp \
	TestFw/TestVerification.cpp \
	TestFw/TestFwOutputtersListTest.cpp \
# iCore tests \
	iCoreTests/iCoreAllTests.cpp \
	iCoreTests/iCoreAsyncTests.cpp \
	iCoreTests/iCoreExceptionHooks.cpp \
	iCoreTests/iCoreMsgBenchmarks.cpp \
	iCoreTests/iCoreMsgObjectSafeDelete.cpp \
        iCoreTests/iCoreSimpleMsgTest.cpp \
        iCoreTests/iCoreSynchronSendingMsgTest.cpp \
        iCoreTests/iCoreThreadPoolTest.cpp \
	iCoreTests/iCoreThreadPoolTestA.cpp \
        iCoreTests/iCoreTimerTest.cpp \
	iCoreTests/iCoreTimerTestAdvanced.cpp \
	iCoreTests/TestUtils.cpp \     
	iCoreTests/iCoreOverload.cpp \ 
# Utils tests \
	UtilsTests/SafeRefTest.cpp \
	Utils/ProgramArg.cpp \
	UtilsTests/MutexTest.cpp \
	UtilsTests/StringParserTest.cpp \
	UtilsTests/BoostFsmMultiTest.cpp \
	UtilsTests/BoostFsmTest.cpp \
	UtilsTests/SafeRefServerTest.cpp \
	UtilsTests/TypeFactoryTest.cpp \
	UtilsTests/VirtualInvokeTest.cpp \
	UtilsTests/ContainerMemFindTests.cpp \
	UtilsTests/BinaryReaderWriterTests.cpp \
	UtilsTests/MemReaderWriterStreamTests.cpp \
	UtilsTests/MemoryPoolTests.cpp \
	UtilsTests/StackStringTest.cpp \
	UtilsTests/IntToStringTests.cpp \
	UtilsTests/UtilsDateTimeTests.cpp \
	UtilsTests/BidirBufferTest.cpp \
	UtilsTests/FixedFifoTest.cpp \
	UtilsTests/IntSetTests.cpp \
	UtilsTests/QtDateTimeCaptureTest.cpp \
	UtilsTests/MsDateTimeCaptureTest.cpp \
	UtilsTests/StatisticBaseTest.cpp \
	UtilsTests/HostInfTest.cpp \
	UtilsTests/ProgramOptionsTest.cpp \
	UtilsTests/ManagedListTest.cpp \
	UtilsTests/QtEnumResolverTest.cpp \
	UtilsTests/StringListTest.cpp \
	UtilsTests/IniFileTest.cpp \
    UtilsTests/FullEnumResolverTest.cpp \
	#UtilsTests/IntSetTests.cpp \
	UtilsTests/OutputFormattersTest.cpp \
# Platform tests \
	PlatformTests/ThreadCounterTest.cpp \
	PlatformTests/ThreadWaitUSTest.cpp \
	PlatformTests/FormatTypeidNameTest.cpp \
	PlatformTests/PlatformThreadTest.cpp \
	PlatformTests/PlatformMutexTest.cpp \
	PlatformTests/PlatformWaitCondTest.cpp \
# Telnet tests \
        TelnetTests/TestString.cpp \
	TelnetTests/TelnetUtilsTest.cpp \
	TelnetTests/TelnetDataCollectorTest.cpp \
	TelnetTests/TelnetHelpersTest.cpp \
	TelnetTests/TelnetOptionListTest.cpp \
	TelnetTests/TelnetTestHelpers.cpp \
	TelnetTests/LocalSocketRegister.cpp \
	TelnetTests/TcpSocketEmul.cpp \
	TelnetTests/TcpServerSocketEmul.cpp \	
	TelnetTests/TelnetClientServerTest.cpp \
# iNet tests\
	iNetTests/DataFlowInf.cpp \
	iNetTests/TestTcp.cpp \
	iNetTests/TestUdp.cpp \
	iNetTests/ProfileTcpTest.cpp \
	iNetTests/TestUserTcpSocket.cpp \
	iNetTests/TestTcpPack.cpp \
# iLog tests \
	iLog/LogTest.cpp \
	iLog/LogTest2.cpp \
	iLog/LogWrapperTest.cpp \
# SafeBiProto tests \
	SafeBiProto/SafeBiProtoTests/SafeBiProtoTest.cpp \
    SafeBiProto/SafeBiProtoTests/SafeBiProtoTestParams.cpp \
    SafeBiProto/SafeBiProtoTests/SafeBiProtoTestSimple.cpp \
    SafeBiProto/SafeBiProtoTests/SbpExceptionsTest.cpp \
    SafeBiProto/SafeBiProtoTests/SbpPackExchange.cpp \
    SafeBiProto/SafeBiProtoTests/SbpRecvJunkDataTest.cpp \
    SafeBiProto/SafeBiProtoTests/SbpRecvParsingTest.cpp \
	SafeBiProto/SafeBiProtoTests/SbpUtilsTests.cpp \
	SafeBiProto/SafeBiProtoTests/TestUtils/LocalTestSocketRegister.cpp \
	SafeBiProto/SafeBiProtoTests/TestUtils/TestTcpServerSocket.cpp \
	SafeBiProto/SafeBiProtoTests/TestUtils/TestTcpSocket.cpp \
# iDSP tests \
    iDSP/EchoTest.cpp \
    iDSP/EchoTestGroup.cpp \
    iDSP/EchoTestPc.cpp \
    iDSP/RegSpanTestsGroup.cpp \
    iDSP/g711tests.cpp \
    iDSP/TestGoertzelDetect.cpp \
    iDSP/TestSupertoneDetect.cpp \
    iDSP/TestDtmf.cpp \
# DRI test \
    DriTests/DRITestHelpers.cpp 		\
	DriTests/DRIScriptTests.cpp 		\
	DriTests/DRIParserTests.cpp 		\
	DriTests/NamedObjectTestType.cpp 	\
	DriTests/DriIncludeDirListTest.cpp	\
# SndMix test
    SndMix/MixerTest.cpp \








