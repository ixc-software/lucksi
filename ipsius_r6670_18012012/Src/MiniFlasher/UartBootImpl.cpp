
#include "stdafx.h"

#include "UartBootImpl.h"
#include "MfUtils.h"

namespace 
{
    const int CProcessComTimerMs = 1;

	enum
	{
		CLoadRequestSignature = 0xBF    	
	};	
	const char *CHelpStr = "Connect board to com port, put jumper and reset board.";
	const char CConnectReqToUart = '@';
	const Platform::byte CConnectRespSize = 4;
    const Platform::dword CMaxWriteStep = 4 * 1024;
    
} // namespace

// ---------------------------------------------------------------------------------

namespace MiniFlasher
{
	UartBootImpl::UartBootImpl(MfProcessParams params)
	{
        MfProcess process(params, false);
        process.AddLog(CHelpStr);
        process.Run(*this);
	}

    // ---------------------------------------------------------------------------------

    void UartBootImpl::RunImpl(MfProcess &p)
    {   
        QFileInfo f(p.Params().FileName);
        
        p.AddLogHeader("Uart boot", f.absoluteFilePath(), f.size());

        MfProcUtils::CheckFile(f);
        WaitCts(p);
        Connect(p);
        Write(p);
    }

    // ---------------------------------------------------------------------------------
     
    void UartBootImpl::WaitCts(MfProcess &p)
    {
        p.AddLog("Waiting CTS ...");

        Utils::TimerTicks timer;
        dword timeout = p.Params().Timeouts.CommandTimeout;
        while (timer.Get() < timeout)
        {
            p.CheckAbort();
            
            if (p.Uart().Management().LineStatus() & QextSerial::LS_CTS) return;

            // Platform::ThreadSleep(0);
        }

        MfProcess::ThrowErr("Waiting CTS timeout.");
    }

    // ---------------------------------------------------------------------------------
    
    void UartBootImpl::Connect(MfProcess &p)
    {
        p.AddLog("Connecting ...");
            
        p.Uart().IO().Write(CConnectReqToUart);

        Utils::TimerTicks timer;
        dword timeout = p.Params().Timeouts.ConnectTimeout;
        while ((p.Uart().IO().AvailableForRead() < CConnectRespSize) 
                && (timer.Get() < timeout))
        {
            p.CheckAbort();
            // Platform::ThreadSleep(0);
        }

        int size = p.Uart().IO().AvailableForRead();

        if (size == 0) MfProcess::ThrowErr("Connect timeout.");

        if (size == CConnectRespSize) 
        {
            Platform::byte resp[CConnectRespSize];
            ESS_ASSERT(p.Uart().IO().Read(resp, sizeof(resp)) == CConnectRespSize);

            if (resp[0] == 0xBF || resp[CConnectRespSize - 1] == 0) 
            {
                p.AddLog("Connected.");
                return;
            }
        }
        
        MfProcess::ThrowErr("Wrong signature was received by uart.");
    }

    // ---------------------------------------------------------------------------------

    void UartBootImpl::Write(MfProcess &p)
    {
        p.AddLog(QString("Writing file '%1' ...").arg(p.Params().FileName));
        
        // disable flow control 
        p.Uart().Management().SetFlowControl(iUart::FlowOff);

        QFileInfo file(p.Params().FileName);
        MfProcUtils::CheckSizeAndOffset(file.size(), 0);

        // read data
        QByteArray data;
        MfProcUtils::ReadFile(file, data);

        // write
        p.ProgressBar().Start(data.size());

        Utils::TimerTicks timer;
        int writtenSize = 0;
        while (writtenSize < data.size())
        {
            p.CheckAbort();

            int writeStep = data.size() - writtenSize;
            if (writeStep > CMaxWriteStep) writeStep = CMaxWriteStep;
            
            writtenSize += p.Uart().IO().Write(data.data() + writtenSize, writeStep); // can throw
            p.ProgressBar().PrintStep(writtenSize);

            // std::cout << "writtenData = " << writtenData << std::endl;
        }
        ESS_ASSERT(writtenSize == data.size());

        p.ProgressBar().Finish(true);
        
        p.AddLog("Written.");
        p.AddLog(MiniFlasherUtils::OperationInfo("Uart boot", writtenSize, timer.Get()));
    }

    // ---------------------------------------------------------------------------------

    
} // namespace MiniFlasher
