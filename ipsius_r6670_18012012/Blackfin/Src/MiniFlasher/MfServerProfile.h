#ifndef __MFSERVERPROFILE__
#define __MFSERVERPROFILE__

#include "Utils/IBasicInterface.h"
#include "Utils/VirtualInvoke.h"

#include "MfTranspPacket.h"

namespace MiniFlasher
{
    using Platform::dword;

    class IFlashTestCallback : public Utils::IBasicInterface
    {
    public:
        virtual void FlashTestState(int percent, int readErrors, int writeErrors) = 0;
    };

    class IFlashInterface : public Utils::IBasicInterface
    {
    public:

        virtual dword FlashSize() const = 0;
        virtual dword FlashWriteGranularity() const = 0;

        virtual bool EraseBegin(dword offs) = 0;
        virtual bool EraseNext(dword &erasedSize) = 0;

        virtual void BeginWrite(dword offs) = 0;
        virtual bool Write(const void *pData, int size) = 0;

        virtual bool Read(void *pData, int offset, int size) = 0;

        virtual void RunFlashTest(IFlashTestCallback &callback) = 0;
    };

    // -------------------------------------------

    class ISysInterface : public Utils::IBasicInterface
    {
    public:
        virtual void Sleep() = 0;
        virtual void Run(dword offs) = 0;
        virtual void Leds(bool on) = 0;
    };

    // -------------------------------------------

    struct ServerProfile
    {
        ITransport &Transport;
        IFlashInterface &Flash;
        ISysInterface &Sys;
        bool AsyncMode;
        int RecvTimeoutMs;

        ServerProfile(ITransport &transport, IFlashInterface &flash, 
            ISysInterface &sys, bool asyncMode) :
            Transport(transport),
            Flash(flash),
            Sys(sys),
            AsyncMode(asyncMode)
        {
            RecvTimeoutMs = 10 * 1000;
        }
    };
	
}  // namespace MiniFlasher

#endif
