
#ifndef __MFPROCESS__
#define __MFPROCESS__

// MfProcess.h

#include "MfProcessParams.h"
#include "MfClient.h"
#include "iUart/Uart.h"
#include "Utils/ProgressBar.h"
#include "Utils/ThreadSyncEvent.h"
#include "Utils/IBasicInterface.h"

namespace MiniFlasher
{
    class MfProcess;

    // Base interface for all classes which are using MfProcess.
    class IMfProcessUser : public Utils::IBasicInterface
    {
    public:
        // can throw MfProcess::ProcessError, MfProcess::ProcessAbort,
        // MfClient::ProtocolError
        virtual void RunImpl(MfProcess &p) = 0;
    };

    // -----------------------------------------------------------------

    // MiniFlasher process (task). 
    class MfProcess : 
        public ITransport,
        Utils::ILogForProgressBar
    {
        // for flash read
        class IStorage;
        class ArrayStorage;
        class FileStorage;
        
        MfProcessParams m_params;
        boost::shared_ptr<Utils::ThreadSyncEvent> m_waitAbort;
        
        boost::scoped_ptr<iUart::Uart> m_uart;
        Utils::ProgressBar m_progressBar;
        boost::scoped_ptr<MfClient> m_proto;
        bool m_isRunning;

        void FlashRead(dword size, dword offset, IStorage &dest);
        void Aborted();
        void CheckProtocolVersion(dword version); // on Connect()
        
    // ILogForProgressBar impl
	private:
		void LogProgressBar(const std::string &str, bool eof = true);

    // ITransport impl
	private:
		void Send(const void *pData, int dataSize);
        int Recv(void *pData, int buffSize);

    public:
        ESS_TYPEDEF(ProcessError);
        ESS_TYPEDEF(ProcessAbort);
        
    public:
        MfProcess(MfProcessParams params, bool enableProto);
        
        iUart::Uart& Uart(); // can throw
        Utils::ProgressBar& ProgressBar() { return m_progressBar; }
        const MfProcessParams& Params() const { return m_params; } 

        // run method, 
        // catch and handle exceptions: ProtocolError, ProcessError & ProcessAbort
        void Run(IMfProcessUser &runImpl);
        
        // using when need to handle other exceptions
        void Finished(bool ok, const std::string &err = "");
        
        void CheckAbort();
        
        void AddLog(const QString &str, bool eof = true);
        void AddLogHeader(const QString &name, const QString file, dword totalBytes);
        
    // If proto enabled in constructor
    public:
        MfClient& Protocol();

        /* methods can throw ProcessError & ProcessAbort */
        void ProtocolConnect();
        const MfPacket* ProtocolWaitPacket(int timeout, bool throwOnTimeout = true); 

        // if size > max --> error
        void ProtocolFlashErase(dword size, dword offset);
        // if size + offset > max --> error
        QByteArray ProtocolFlashDump(dword size, dword offset);
        // if size + offset > max --> error
        void ProtocolFlashDump(dword size, dword offset, const QString &destFileName);

        static void ThrowErr(const std::string &err);
        static void ThrowUnexpectedPacketErr(const MfPacket &pack);
        static void ThrowFileErr(const QString &err, const QString &file);
    };

    // -----------------------------------------------------------------

    class MfProcUtils
    {
    public:
        static dword MaxFlashSize();

        // 0 < offset < max
        static void CheckOffset(dword offset);
        // 0 < size + offset <= max
        static void CheckSizeAndOffset(dword size, dword offset);
        // size == 0 or size > max --> return max size
        static dword CorrectSize(dword size, dword offset);

        static void CheckFile(const QFileInfo &file); // can throw
        static void ReadFile(const QFileInfo &file, QByteArray &dest);
    };

    // -----------------------------------------------------------------
    
    /*
    class Sample : IMfProcessUser 
    {
    // IMfProcessUser impl;
    private:
        void RunImpl(MfProcess &p) // can throw
        {
            while (true)
            {
                p.CheckAbort();

                if (error) MfProcess::ThrowErr(errorMsg);

                p.ProtocolConnect();
                
                // do something
            }
        }

    public:
        Sample(boost::shared_ptr<MfProcessParams> params)
        {
            MfProcess p(params, true);
            p.Run(*this);
        }
    };
    */


    
} // namespace MiniFlasher

#endif
