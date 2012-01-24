#ifndef __MFPCPROTOCOL__
#define __MFPCPROTOCOL__

#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "MfTranspPacket.h"
#include "MfProtocol.h"

namespace MiniFlasher
{
    using Platform::dword;

    class MfPacket;
    
    class IMfProtocol : public Utils::IBasicInterface
    {

    // execute protocol async commands
    public:

        // -> Connected() | error | nothing
        virtual void SendConnect() = 0;

        // -> EraseProgress(), Erased() | error
        virtual void SendErase(dword offs, dword size) = 0;

        // -> Writed() | error
        virtual void SendWrite(bool compressed, const void *pData, dword dataSize) = 0;

        // -> VerifyProgress(), Verified() | error
        virtual void SendVerify() = 0;

        // -> Runned()
        virtual void SendRun(dword offs) = 0;

        // -> Readed()
        virtual void SendRead(dword offset, dword size) = 0;

        // -> FlashTestProgress(), FlashTestCompleted()
        virtual void StartFlashTest() = 0;

    // managment and info 
    public:

        // virtual void Process() = 0;

        virtual const MfPacket* Receive() = 0;

        virtual dword MaxDataSize() const = 0; 
    };

    // --------------------------------------------------

    /*
    class IMfProtocolCallback : public Utils::IBasicInterface
    {
    public:

        virtual void Connected(dword ver) = 0;
        virtual void EraseProgress(dword size) = 0;
        virtual void Erased() = 0;
        virtual void Writed() = 0;
        virtual void VerifyProgress(dword size) = 0;
        virtual void Verified(dword crc) = 0;
        virtual void Runned() = 0;
        virtual void Readed(const void *pData, dword dataSize) = 0;
        virtual void FlashTestProgress(dword percent, dword errors) = 0;
        virtual void FlashTestCompleted(dword errors) = 0;

        virtual void RespError(Protocol::Error error) = 0;        
        virtual void ProtocolError(MfTranspPacketRecv::ResultCode code) = 0;
    };
    */
    
    
}  // MiniFlasher

#endif
