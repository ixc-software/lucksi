#ifndef __MFPROTOCOL__
#define __MFPROTOCOL__

#include "Platform/PlatformTypes.h"

namespace MiniFlasher
{
    using Platform::byte;

    struct Protocol
    {

    public:
        enum
        {
            CHeaderID   = 0xA1,     // byte 
            CVersion    = 0x11f3,   // dword
            
            // max packet size is word
            CMaxDataPayload     = 32 * 1024,

            CMaxPacketBodySize  = CMaxDataPayload + 32,
        };

    public:

        // client -> server
        // created at MfClient::ProcessPacket
        // parsed  at MfServer::ProcessPacket
        enum CmdCode
        {
            CmdConnect = 1,     // ()

            // write cycle commands
            CmdErase,           // (dword offs, dword size)
            CmdWrite,           // (bool compressed, dword size, byte[size] data)
            CmdVerify,          // ()

            CmdRead,            // (dword offs, dword size)
            CmdRun,             // (dword offs)
            CmdFlashTest,       // ()
        };

    public:

        // server -> client
        // created at MfServer::ProcessPacket
        // parsed  at MfClient::ProcessPacket
        enum RespCode
        {
            RespError     = 0x80,   // (dword code as Protocol::Error)
            RespConnected,          // (dword ver)
            RespEraseProgress,      // (dword size)
            RespErased,             // ()
            RespWrited,             // ()
            RespVerifyProgress,     // (dword size)
            RespVerifyDone,         // (dword crc32)
            RespRunned,             // ()
            RespReaded,             // (dword size, byte[size] data)
            RespFlashTestProgress,  // (dword percent, dword errors)
            RespFlashTestCompleted, // (dword errors)
        };

        static bool IsValidRespCode(byte code)
        {
            return ((code == RespError) 
                    || (code == RespConnected)
                    || (code == RespEraseProgress)
                    || (code == RespErased)
                    || (code == RespWrited)
                    || (code == RespVerifyProgress)
                    || (code == RespVerifyDone)
                    || (code == RespRunned)
                    || (code == RespReaded)
                    || (code == RespFlashTestProgress)
                    || (code == RespFlashTestCompleted));
        }

        static std::string RespCodeToString(RespCode code)
        {
            #define RESP_DECODE(m) if (code == m) return #m;

            RESP_DECODE(RespError);
            RESP_DECODE(RespConnected);
            RESP_DECODE(RespEraseProgress);
            RESP_DECODE(RespErased);
            RESP_DECODE(RespWrited);
            RESP_DECODE(RespVerifyProgress);
            RESP_DECODE(RespVerifyDone);
            RESP_DECODE(RespRunned);
            RESP_DECODE(RespReaded);
            RESP_DECODE(RespFlashTestProgress);
            RESP_DECODE(RespFlashTestCompleted);

            #undef RESP_DECODE

            return "Can't decode!";
        }

    public:

        enum Error
        {
            ErrPacketLevelError = 1,
            ErrPacketArg,
            ErrUnknownCmd,
            ErrBadState,
            ErrBadArgValue,
            ErrFlashErase,
            ErrFlashWrite,
            ErrFlashVerify,
            ErrExtractError,
            ErrFlashRead,
        };

        static bool IsValidError(byte err)
        {
            return ((err == ErrPacketLevelError) 
                    || (err == ErrPacketArg)
                    || (err == ErrUnknownCmd)
                    || (err == ErrBadState)
                    || (err == ErrBadArgValue)
                    || (err == ErrFlashErase)
                    || (err == ErrFlashWrite)
                    || (err == ErrFlashVerify)
                    || (err == ErrExtractError)
                    || (err == ErrFlashRead));
        }

        static std::string ErrorToString(Error code)
        {
            #define VAL_DECODE(m) if (code == m) return #m;

            VAL_DECODE(ErrPacketLevelError);
            VAL_DECODE(ErrPacketArg);
            VAL_DECODE(ErrUnknownCmd);
            VAL_DECODE(ErrBadState);
            VAL_DECODE(ErrBadArgValue);
            VAL_DECODE(ErrFlashErase);
            VAL_DECODE(ErrFlashWrite);
            VAL_DECODE(ErrFlashVerify);
            VAL_DECODE(ErrExtractError);
            VAL_DECODE(ErrFlashRead);

            #undef VAL_DECODE

            return "Can't decode!";
        }

    };
	
	
}   // namespace MiniFlasher

#endif
