#Dtmf test
HEADERS += \
        HiLevelTests/Dtmf/NObjDtmfTester.h      \

SOURCES += \
        HiLevelTests/Dtmf/NObjDtmfTester.cpp    \

#Conf test
HEADERS += \
        HiLevelTests/ConfTest/NObjConfTester.h      \

SOURCES += \
        HiLevelTests/ConfTest/NObjConfTester.cpp    \
        HiLevelTests/ConfTest/RtpSigDetector.cpp    \
        HiLevelTests/ConfTest/RtpSigSource.cpp      \


#HwFindeer  test
HEADERS += \
        HiLevelTests/HwFinder/NObjFwuMaker.h			\
        HiLevelTests/HwFinder/NObjHwFinderTester.h		\
        HiLevelTests/HwFinder/TaskWaitAlloc.h			\

SOURCES += \
        HiLevelTests/HwFinder/NObjFwuMaker.cpp			\
        HiLevelTests/HwFinder/NObjHwFinderTester.cpp	\
        HiLevelTests/HwFinder/TaskWaitAlloc.cpp			\

