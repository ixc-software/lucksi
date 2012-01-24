#include "stdafx.h"
#include "Utils/RawCRC32.h"
#include "Utils/ExeName.h"
#include "Utils/ZlibPipe.h"
#include "Utils/MemWriterStream.h"
#include "MfPcProtocol.h"
#include "MfProtocol.h" 
#include "LoadImgImpl.h"
#include "MfPacketFieldNames.h"
#include "MfUtils.h"

namespace 
{
    const bool CEnableCompression = true;
    
} // namespace

// -----------------------------------------------------------------

namespace 
{
    QByteArray Deflate(const char *pData, int size)
    {
        ESS_ASSERT(pData != 0);
        ESS_ASSERT(size > 0);

        QByteArray compressed(size * 2, 0);
        Utils::MemWriterStream stream(compressed.data(), compressed.size());
        Utils::ZlibDeflate def(stream);
        def.Add(pData, size, true);

        compressed.resize(stream.BufferSize());
        return compressed;
    }
} // namespace

// -----------------------------------------------------------------

namespace MiniFlasher
{
    LoadImgImpl::LoadImgImpl(MfProcessParams params)
	{
        MfProcess process(params, true);
        process.Run(*this);
	}
   
    // -----------------------------------------------------------------

    void LoadImgImpl::RunImpl(MfProcess &p)
    {
        QFileInfo f(p.Params().FileName);

        p.AddLogHeader("Load image", f.absoluteFilePath(), f.size());

        MfProcUtils::CheckFile(f);
        MfProcUtils::CheckSizeAndOffset(f.size(), p.Params().FlashOffset);
        
        p.ProtocolConnect();
        p.ProtocolFlashErase(f.size(), p.Params().FlashOffset);
        dword crc = Write(p);
        Verify(p, crc, f.size());
        if (p.Params().RunFlashApp) Run(p);
    }

    // -----------------------------------------------------------------

    int LoadImgImpl::SendCompressed(MfProcess &p, const char *pData, int size)
    {
        if (CEnableCompression)
        {
            QByteArray compressed = Deflate(pData, size);
            int ratio = (compressed.size() * 100) / size;
            if (ratio < 90)
            {
                p.Protocol().SendWrite(true, compressed.data(), compressed.size());
                return compressed.size();
            }
        }

        p.Protocol().SendWrite(false, pData, size);
        return size;
    }

    // -----------------------------------------------------------------

    dword LoadImgImpl::Write(MfProcess &p)
    {
        p.AddLog("Writing ...");

        QByteArray data;
        MfProcUtils::ReadFile(QFileInfo(p.Params().FileName), data);

        dword timeout = p.Params().Timeouts.CommandTimeout;
        dword crc = Utils::RawCRC32::InitialValue;
        int writtenData = 0;
        int writtenDataReal = 0;
        
        Utils::TimerTicks timer;
        p.ProgressBar().Start(data.size());
        while(writtenData < data.size())
        {
            int step = data.size() - writtenData;
            if (step > p.Protocol().MaxDataSize()) step = p.Protocol().MaxDataSize();
            const char *pCurr = data.data() + writtenData;
            writtenData += step;
            crc = Utils::UpdateCRC32(pCurr, step, crc, (writtenData == data.size()));

            writtenDataReal += SendCompressed(p, pCurr, step);

            const MfPacket *pPack = p.ProtocolWaitPacket(timeout); // can throw

            if (pPack->Type() != Protocol::RespWrited) 
            {
                MfProcess::ThrowUnexpectedPacketErr(*pPack);
            }
            
            p.ProgressBar().PrintStep(writtenData);
        }
        ESS_ASSERT(writtenData == data.size());
        
        p.ProgressBar().Finish(true);

        QString msg = MiniFlasherUtils::OperationInfo("Writing", writtenData, timer.Get());
        ESS_ASSERT(writtenData != 0);
        msg += QString("Ratio %1%").arg(writtenDataReal * 100 / writtenData);
        p.AddLog(msg);

        return crc;
    }

    // -----------------------------------------------------------------

    void LoadImgImpl::Verify(MfProcess &p, dword crc, dword progressBarSize)
    {
        p.AddLog("Verifying ...");

        p.Protocol().SendVerify();
        p.ProgressBar().Start(progressBarSize);

        Utils::TimerTicks timer;
        dword timeout = p.Params().Timeouts.CommandTimeout;
        while (true)
        {
            const MfPacket *pPack = p.ProtocolWaitPacket(timeout); // can throw
            
            using namespace MfPacketFieldNames;
            
            if (pPack->Type() == Protocol::RespVerifyProgress)
            {
                p.ProgressBar().PrintStep(pPack->Get(FSize).AsDword());
                continue;
            }
            
            if (pPack->Type() == Protocol::RespVerifyDone)
            {
                if (pPack->Get(FCRC32).AsDword() != crc) MfProcess::ThrowErr("Crc error.");
                break;
            }

            MfProcess::ThrowUnexpectedPacketErr(*pPack);
        }
        p.ProgressBar().Finish();

        p.AddLog(MiniFlasherUtils::OperationInfo("Verifying", 
                                                         p.Params().FlashSize, 
                                                         timer.Get()));
    }
    
    // -----------------------------------------------------------------

    void LoadImgImpl::Run(MfProcess &p)
    {
        ESS_ASSERT(p.Params().RunFlashApp);
        
        p.AddLog("Launch application ...");
        p.Protocol().SendRun(p.Params().FlashOffset);
        
        Utils::TimerTicks timer;
        dword timeout = p.Params().Timeouts.CommandTimeout;
        while (true)
        {
            const MfPacket *pPack = p.ProtocolWaitPacket(timeout); // can throw
            
            if (pPack->Type() == Protocol::RespRunned) 
            {
                p.AddLog("Application is run.");
                break;
            }

            MfProcess::ThrowUnexpectedPacketErr(*pPack);
        }
    }
    
} // namespace
