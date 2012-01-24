
#include "stdafx.h"
#include "MfProcess.h"
#include "Utils/TimerTicks.h"
#include "Utils/ThreadSyncEvent.h"
#include "Utils/IBasicInterface.h"
#include "MfPacketFieldNames.h"
#include "MfUtils.h"

namespace
{
    using namespace MiniFlasher;
    
    const Platform::dword CMaxDataReadSize = 8 * 1024;
    BOOST_STATIC_ASSERT(CMaxDataReadSize <= Protocol::CMaxDataPayload);
    
} // namespace

// ---------------------------------------------------------------------------------

namespace MiniFlasher
{
    class MfProcess::IStorage : public Utils::IBasicInterface
    {
    public:
        virtual void Open() = 0;
        virtual void Write(const char *pData, int size) = 0;
        virtual void Close() = 0;
        virtual dword Size() const = 0;
    };

    // ---------------------------------------------------------------------------------
        
    class MfProcess::ArrayStorage : public MfProcess::IStorage
    {
        QByteArray m_data;

    public:
        ArrayStorage(int size)
        {
            ESS_ASSERT(size > 0);
            m_data.reserve(size);
        }
        
        QByteArray Get() const { return m_data; }

    // IStore impl
    public:
        void Open() { /* do nothing */ }
        void Close() { /* do nothing */ }

        void Write(const char *pData, int size)
        {
            ESS_ASSERT(pData != 0);
            ESS_ASSERT(size > 0);
            ESS_ASSERT((m_data.size() + size) <= m_data.capacity());

            while ((size > 0) && (m_data.size() < m_data.capacity()))
            {
                m_data.push_back(*pData++); 
                --size;
            }
            ESS_ASSERT(size == 0);
        }

        dword Size() const { return m_data.size(); }
    };

    // ---------------------------------------------------------------------------------

    class MfProcess::FileStorage : public MfProcess::IStorage
    {
        QFile m_file;

    public:
        FileStorage(const QString &name) : m_file(name) {}

    // IStore impl
    public:
        void Open()
        {
            if (m_file.open(QIODevice::WriteOnly)) return;

            ThrowFileErr("opening", m_file.fileName());
        }

        void Close() { m_file.close(); }

        void Write(const char *pData, int size)
        {
            ESS_ASSERT(pData != 0);
            ESS_ASSERT(size > 0);
            
            if (m_file.write(pData, size) == size) return;

            ThrowFileErr("writing", m_file.fileName());
        }
        
        dword Size() const { return m_file.size(); }
    };


} // namespace MiniFlasher

// ---------------------------------------------------------------------------------

namespace MiniFlasher
{
    MfProcess::MfProcess(MfProcessParams params, bool enableProto) : 
        m_params(params),
		// m_uart(params->UartProfile), // init on first call
        m_progressBar(*this),
        m_isRunning(true)
	{
        /*std::string err;
        iUart::Uart *p = iUart::Uart::CreateUart(params->UartProfile, err);
        if (p == 0) ThrowErr(err);
        m_uart.reset(p);
        */
        
        if (enableProto) m_proto.reset(new MfClient(*this));
	}

    // ---------------------------------------------------------------------------------

    iUart::Uart& MfProcess::Uart()
    { 
        // try to create
        if (m_uart == 0)
        {
            std::string err;
            iUart::Uart *p = iUart::Uart::CreateUart(Params().UartProfile, err);
            if (p == 0) 
            {
                QString msg("Uart initialization error");
                if (!err.empty()) msg += QString(": %1").arg(err.c_str());
                ThrowErr(msg.toStdString());
            }
            m_uart.reset(p);
        }
        
        return *m_uart; 
    }

    // ---------------------------------------------------------------------------------

    MfClient& MfProcess::Protocol() 
    {
        ESS_ASSERT(m_proto != 0); 

        return *m_proto.get(); 
    }

    // ---------------------------------------------------------------------------------
        
    const MfPacket* MfProcess::ProtocolWaitPacket(int timeout, bool throwOnTimeout)
    {
        Utils::TimerTicks timer;
        while (timer.Get() < timeout)
        {
            CheckAbort();

            const MfPacket *p = Protocol().Receive();
            if (p != 0) return p;

            Platform::ThreadSleep(0);
        }

        if (throwOnTimeout) ThrowErr("Waiting responce packet timeout.");

        return 0;
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::ProtocolConnect()
    {
        dword connectTimeout = Params().Timeouts.ConnectTimeout;
        dword repeatInterval = Params().Timeouts.ConnectRepeatInterval;
        if(repeatInterval >= connectTimeout)
        {
            ThrowErr("Connect repeat interval is more than connect timeout.");
        }
        
        AddLog("Connecting ...");
        const MfPacket *pPack = 0;
        Utils::TimerTicks timer;
        while (timer.Get() < connectTimeout)
        {
            Protocol().SendConnect();

            pPack = ProtocolWaitPacket(repeatInterval, false); // can throw
            
            if (pPack != 0)
            {
                if (pPack->Type() != Protocol::RespConnected)
                {
                    ThrowUnexpectedPacketErr(*pPack);
                }
                break;
            }
        }
        if (pPack == 0) ThrowErr("Connect timeout.");
                
        CheckProtocolVersion(pPack->Get(MfPacketFieldNames::FVersion).AsDword());

        AddLog("Connected.");
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::CheckProtocolVersion(dword version)
    {
        if(version == Protocol::CVersion) return;

        std::ostringstream msg;
        msg << "Version error. Local version: " << Protocol::CVersion 
                << ". Remote version: " << version;
        ThrowErr(msg.str());
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::ProtocolFlashErase(dword size, dword offset)
    {
        MfProcUtils::CheckSizeAndOffset(size, offset);

        AddLog("Erasing ...");
        // Protocol().SendErase(Params().FlashOffset, Params().FlashSize);
        Protocol().SendErase(offset, size);
        ProgressBar().Start(size);

        Utils::TimerTicks timer;
        dword timeout = Params().Timeouts.CommandTimeout;
        while(true)
        {
            const MfPacket *pPack = ProtocolWaitPacket(timeout); // can throw
            
            if (pPack->Type() == Protocol::RespEraseProgress)
            {
                ProgressBar().PrintStep(pPack->Get(MfPacketFieldNames::FSize).AsDword());
                continue;
            }
            
            if (pPack->Type() == Protocol::RespErased)
            {
                ProgressBar().Finish(true);
                break;
            }

            ThrowUnexpectedPacketErr(*pPack);
        }

        AddLog(MiniFlasherUtils::OperationInfo("Erasing", timer.Get()));
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::FlashRead(dword size, dword offset, IStorage &dest)
    {
        using namespace MfPacketFieldNames;
        
        MfProcUtils::CheckSizeAndOffset(size, offset);
        
        // read
        dword timeout = Params().Timeouts.CommandTimeout;
        dword gotDataSize = 0;
        ProgressBar().Start(size);
        dest.Open();
        while (gotDataSize < size)
        {
            dword toReadSize = size - gotDataSize;
            if (toReadSize > CMaxDataReadSize) toReadSize = CMaxDataReadSize;
            
            Protocol().SendRead(offset, toReadSize);

            const MfPacket *pPack = 0;
            pPack = ProtocolWaitPacket(timeout);

            ESS_ASSERT(pPack != 0);
            if (pPack->Type() != Protocol::RespReaded) 
            {
                ThrowUnexpectedPacketErr(*pPack);
            }

            const char *pData = static_cast<const char*>(pPack->Get(FData).AsBinary());
            int dataSize = pPack->Get(FData).Size();
            dest.Write(pData, dataSize);
            
            ProgressBar().PrintStep(gotDataSize);

            gotDataSize += toReadSize;
            offset += toReadSize;
        }
        ESS_ASSERT(gotDataSize == dest.Size());

        dest.Close();
        ProgressBar().Finish();
    }

    // ---------------------------------------------------------------------------------

    QByteArray MfProcess::ProtocolFlashDump(dword size, dword offset)
    {
        AddLog("Reading ...");

        Utils::TimerTicks t;
        
        MfProcUtils::CheckSizeAndOffset(size, offset);
        ArrayStorage s(size);
        
        FlashRead(size, offset, s);

        AddLog(MiniFlasherUtils::OperationInfo("Reading", size, t.Get()));

        return s.Get();
    }

    // ---------------------------------------------------------------------------------
    
    void MfProcess::ProtocolFlashDump(dword size, dword offset, const QString &destFileName)
    {
        AddLog("Reading and saving to file ...");

        Utils::TimerTicks t;
        FileStorage s(destFileName);

        FlashRead(size, offset, s);

        AddLog(MiniFlasherUtils::OperationInfo("Reading and saving to file", size, t.Get()));
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::Run(IMfProcessUser &runImpl)
    {
        try
        {
            runImpl.RunImpl(*this); 
        }
        catch (MfProcess::ProcessError &e)
        {
            Finished(false, e.getTextMessage());
            return;
        }
        catch (MfClient::ProtocolError &e)
        {
            Finished(false, e.getTextMessage());
            return;
        }
        catch (MfProcess::ProcessAbort &e)
        {
            Aborted();
            return;
        }

        Finished(true);
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::CheckAbort()
    {
        m_waitAbort = Params().Owner->IsAborted();

        if (m_waitAbort != 0) ESS_THROW(ProcessAbort);
    }
    
    // ---------------------------------------------------------------------------------

    void MfProcess::Finished(bool ok, const std::string &err)
    {
        ESS_ASSERT(m_isRunning);
        // ESS_ASSERT(!m_progressBar.IsActive());
        if (m_progressBar.IsActive()) m_progressBar.Finish(false);

        m_isRunning = false;
        Params().Owner->MsgProcessEnd(ok, err);
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::Aborted()
    {
        ESS_ASSERT(m_isRunning);

        m_isRunning = false;
        if (m_progressBar.IsActive()) m_progressBar.Finish(false);

        m_proto.reset();
        m_params.Owner.Clear();
        
        m_waitAbort->Ready();
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::AddLog(const QString &str, bool eof)
    {
        LogProgressBar(str.toStdString(), eof);
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::AddLogHeader(const QString &name, const QString file, dword totalBytes)
    {
        using namespace MiniFlasherUtils;
        AddLog(OperationHead(name, Params().UartProfile.m_port, 
                             Params().UartProfile.m_baudRate, file, totalBytes));
    }

    // ---------------------------------------------------------------------------------
    // ILogForProgressBar impl
    
    void MfProcess::LogProgressBar(const std::string &str, bool eof)
    {
        Params().Owner->MsgLog(str, eof);
    }

    // ---------------------------------------------------------------------------------
    // ITransport impl

    void MfProcess::Send(const void *pData, int dataSize)
    {
        try
        {
            Uart().IO().Write(pData, dataSize);
        }
        catch (iUart::IUartIO::UartIoWriteError &e)
        {
            std::string msg = (e.getTextMessage().empty())? 
                              ("Uart writing error.") : e.getTextMessage();
            ThrowErr(msg);
        }
        
    }

    // ---------------------------------------------------------------------------------

    int MfProcess::Recv(void *pData, int buffSize)
    {
        int res = 0;
        try
        {
            res = Uart().IO().Read(pData, buffSize);
        }
        catch (iUart::IUartIO::UartIoReadError &e)
        {
            std::string msg = (e.getTextMessage().empty())? 
                              ("Uart reading error.") : e.getTextMessage();
            ThrowErr(msg);
        }
        
        return res;
    }

    // ---------------------------------------------------------------------------------
    // static 
    
    void MfProcess::ThrowErr(const std::string &err)
    {
        ESS_THROW_MSG(MfProcess::ProcessError, err);
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::ThrowUnexpectedPacketErr(const MfPacket &pack)
    {
        if (pack.Type() == Protocol::RespError) 
        {
            ThrowErr(Protocol::ErrorToString(pack.Get(MfPacketFieldNames::FCode).AsError()));
        }

        std::string msg("Unexpected responce packet: ");
        msg += Protocol::RespCodeToString(pack.Type());
        msg += ".";

        ThrowErr(msg);
    }

    // ---------------------------------------------------------------------------------

    void MfProcess::ThrowFileErr(const QString &err, const QString &file)
    {
        ThrowErr(QString("File '%1' %2.").arg(file).arg(err).toStdString());
    }

    // ---------------------------------------------------------------------------------
    // MfProcUtils impl

    dword MfProcUtils::MaxFlashSize()
    {
        return (16 * 1024 * 1024);
    }

    // ---------------------------------------------------------------------------------

    dword MfProcUtils::CorrectSize(dword size, dword offset)
    {
        CheckOffset(offset);

        if ((size == 0) || (size > MaxFlashSize())) size = MaxFlashSize();
        if ((size + offset) > MaxFlashSize()) size = MaxFlashSize() - offset;

        return size;
    }

    // ---------------------------------------------------------------------------------

    void MfProcUtils::CheckFile(const QFileInfo &file)
    {
        if(!file.exists())
        {
            MfProcess::ThrowFileErr("wasn't found", file.absoluteFilePath());
        }    
        if (file.size() == 0)
        {
            MfProcess::ThrowFileErr("is empty", file.absoluteFilePath());
        }
    }

    // ---------------------------------------------------------------------------------

    void MfProcUtils::ReadFile(const QFileInfo &file, QByteArray &dest)
    {
        CheckFile(file);

        QString fName(file.absoluteFilePath());
        QFile f(fName);
        if (!f.open(QIODevice::ReadOnly)) MfProcess::ThrowFileErr("opening error", fName);

        dest = f.read(f.size());
        if (dest.size() != f.size()) MfProcess::ThrowFileErr("reding error", fName);
    }

    // ---------------------------------------------------------------------------------

    void MfProcUtils::CheckOffset(dword offset)
    {
        if (offset < MaxFlashSize())  return;

        MfProcess::ThrowErr(QString("Too big offset: %1").arg(offset).toStdString());
    }

    // ---------------------------------------------------------------------------------

    void MfProcUtils::CheckSizeAndOffset(dword size, dword offset)
    {
        CheckOffset(offset);
        if (size == 0) MfProcess::ThrowErr("Invalid size: 0");
        if ((size + offset) > MaxFlashSize()) 
        {
            MfProcess::ThrowErr(QString("Too big size : %1").arg(size).toStdString());
        }
    }

    // ---------------------------------------------------------------------------------

    
    
} // namespace MiniFlasher
