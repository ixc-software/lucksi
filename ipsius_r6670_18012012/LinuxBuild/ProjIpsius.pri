contains(IPSIUS_MODULE, CallMeeting) : include(../LinuxBuild/CallMeeting.pri)
contains(IPSIUS_MODULE, Pult) : include(../LinuxBuild/iPult.pri)
include(../LinuxBuild/Calls.pri)
contains(IPSIUS_MODULE, Dss1) : include(../LinuxBuild/Dss1ToSip.pri)
include(../LinuxBuild/HiLevelTests.pri)
contains(IPSIUS_MODULE, Aoz) : include(../LinuxBuild/iAoz.pri)
include(../LinuxBuild/iCmp.pri)
include(../LinuxBuild/iCmpExt.pri)

include(../LinuxBuild/iDSP.pri)

include(../LinuxBuild/iMedia.pri)
include(../LinuxBuild/iRtp.pri)
include(../LinuxBuild/iRtpUtils.pri)
contains(IPSIUS_MODULE, Dss1) : include(../LinuxBuild/ISDN.pri)
contains(IPSIUS_MODULE, Dss1) : include(../LinuxBuild/IsdnTest.pri)
#contains(IPSIUS_MODULE, SipOld)  : include(../LinuxBuild/Sip.pri)
#contains(IPSIUS_MODULE, SipOld)  : include(../LinuxBuild/SipTest.pri)

include(../LinuxBuild/Sip.pri)
contains(IPSIUS_MODULE, iSip)  : include(../LinuxBuild/iSip.pri)

include(../LinuxBuild/SndMix.pri)
include(../LinuxBuild/TdmMng.pri)
include(../LinuxBuild/TestRealEcho.pri)
include(../LinuxBuild/TestRtpWithSip.pri)
include(../LinuxBuild/Zip.pri)

contains(IPSIUS_MODULE, KbGPIODriver) : include(../LinuxBuild/KbGPIODrv.pri)

contains(IPSIUS_MODULE, AlsaSound) :  include(../LinuxBuild/AlsaSound.pri)
contains(IPSIUS_MODULE, AlsaSound) :  include(../LinuxBuild/AlsaSoundLib.pri)
