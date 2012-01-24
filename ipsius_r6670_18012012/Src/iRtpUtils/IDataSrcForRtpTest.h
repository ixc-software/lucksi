#ifndef _I_DATA_SOURCE_FOR_TEST_RTP_SIP_H_
#define _I_DATA_SOURCE_FOR_TEST_RTP_SIP_H_

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"
#include "iMedia/iCodec.h"

namespace iRtpUtils
{
    class IDataSrcForRtpTest : public Utils::IBasicInterface
    {
    public:
        virtual void Setup(const iMedia::Codec &codec) = 0;
        virtual QByteArray Read(int size) = 0;
    };
};

#endif
