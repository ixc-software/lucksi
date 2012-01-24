#ifndef __TDMEXCEPTION__
#define __TDMEXCEPTION__

#include "Utils/ErrorsSubsystem.h"

namespace TdmMng
{

    enum TdmErrorCode   // sync with ResolveErrorCode() - !!
    {
    	terOK,
        terBadDeviceName,
        terDuplicateNameStream,
        terBadDeviceChannel,
        terCommandBeforeInit,
        terAlreadyRecvEnabled,
        terAlreadyRecvDisabled,
        terMoreThenOneSender,
        terLocalChannelNotFound,
        terDublicateSender,
        terNothingToStopSend,
        terBadVoiceChannelString,
        terUnknownCommand,
        terSendToLocalhost,
        terBadGeneratorParams,
        terGeneratorOverRecv,
        terBadEchoParams,
        terNoHDLC,
		terAlreadyBusy,
        terBadDmaBlockCapacity,
        terBadLogHostPort,
        terAppBodyUnsupportedCommand,
		terCmpProtoError,
        terBadPulseCapturePeriodMs,
        terBadAbChCommand,
        terBadLiuProfile,
        terAozExtraInitFail,
		terBfInitDataE1,
		terBfInitDataAoz,
        terBfInitDataPult,
        terFreqRecvError,
        terTdmCompanding,
        terConferenceError,
        terConfPointBadMode,
        terBadPultCofidecState,
        terNullEchoWithBody,
        terTestException,
    };           

    // ---------------------------------------------------------

    class TdmErrorInfo
    {
        TdmErrorCode m_code;
        std::string m_desc;

    public:

        static std::string ResolveErrorCode(TdmErrorCode errCode)
        {
            #define ERR_DECODE(m) if (errCode == m) return #m;

            ERR_DECODE(terOK);
            ERR_DECODE(terBadDeviceName);
            ERR_DECODE(terDuplicateNameStream);
            ERR_DECODE(terBadDeviceChannel);
            ERR_DECODE(terCommandBeforeInit);
            ERR_DECODE(terAlreadyRecvEnabled)
            ERR_DECODE(terAlreadyRecvDisabled);
            ERR_DECODE(terMoreThenOneSender);
            ERR_DECODE(terLocalChannelNotFound);
            ERR_DECODE(terDublicateSender);
            ERR_DECODE(terNothingToStopSend);
            ERR_DECODE(terBadVoiceChannelString);
            ERR_DECODE(terUnknownCommand);
            ERR_DECODE(terSendToLocalhost);
            ERR_DECODE(terBadGeneratorParams);
            ERR_DECODE(terGeneratorOverRecv);
            ERR_DECODE(terBadEchoParams);
            ERR_DECODE(terNoHDLC);
            ERR_DECODE(terBadDmaBlockCapacity);
            ERR_DECODE(terBadLogHostPort);
            ERR_DECODE(terAppBodyUnsupportedCommand);
            ERR_DECODE(terCmpProtoError);
            ERR_DECODE(terBadPulseCapturePeriodMs);
            ERR_DECODE(terBadAbChCommand);
            ERR_DECODE(terBadLiuProfile);
            ERR_DECODE(terAozExtraInitFail);
			ERR_DECODE(terBfInitDataE1);
			ERR_DECODE(terBfInitDataAoz);
            ERR_DECODE(terBfInitDataPult);
            ERR_DECODE(terFreqRecvError);
            ERR_DECODE(terTdmCompanding);
            ERR_DECODE(terConferenceError);
            ERR_DECODE(terConfPointBadMode);
            ERR_DECODE(terBadPultCofidecState);
            ERR_DECODE(terNullEchoWithBody);
            ERR_DECODE(terTestException);

            #undef ERR_DECODE

            return "Unknown error code! Can`t resolve!";
        }

        TdmErrorInfo(TdmErrorCode code) : m_code(code)
        {
        }

        TdmErrorInfo(TdmErrorCode code, const std::string &desc)
            : m_code(code), m_desc(desc)
        {
        }

        TdmErrorCode Code() const { return m_code; }
        const std::string& Desc() const { return m_desc; }
    };

    // ---------------------------------------------------------

    ESS_TYPEDEF_T(TdmException, TdmErrorInfo);
		
}  // namespace TdmMng

#endif
