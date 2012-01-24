
#include "stdafx.h"

#include "NObjMiniFlasher.h"

#include "DRI/driutils.h"
#include "iLog/LogManager.h"
#include "Utils/ExeName.h"
#include "Utils/VirtualInvoke.h"
#include "Utils/IntToString.h"
#include "Utils/AtomicTypes.h"
#include "MfUtils.h"
#include "EraseFlashImpl.h"
#include "FlashDumpImpl.h"
#include "FlashTestImpl.h"
#include "LoadImgImpl.h"
#include "UartBootImpl.h"


// --------------------------------------------------------------------------------

namespace
{
	QString FileName(const QString &arg)
	{
		QFileInfo fileInfo(arg);
		if(!fileInfo.isAbsolute())
        { 
            fileInfo = QString(Utils::ExeName::GetExeDir().c_str()).append(arg);
        }
		return fileInfo.absoluteFilePath();
	}
    
} // namespace

// --------------------------------------------------------------------------------

namespace MiniFlasher
{
    // --------------------------------------------------------------------------------
    
    NObjMiniFlasher::NObjMiniFlasher(Domain::IDomain *pDomain, 
                                     const Domain::ObjectName &name) : 
		Domain::NamedObject(pDomain, name)
	{
		m_comPort = -1;
		m_baudRate = 115200;
		
        // ProgressBar::TestProgressBar();
	}

    // --------------------------------------------------------------------------------
    // IMfProcessEvents impl

    void NObjMiniFlasher::MsgLog(const std::string &msg, bool eof)
    {
        PutMsg(this, &NObjMiniFlasher::OnLog, LogData(msg, eof));
    }
    
    // --------------------------------------------------------------------------------

    void NObjMiniFlasher::MsgProcessEnd(bool ok, const std::string &msg)
	{
		PutMsg(this, &NObjMiniFlasher::OnFinish, TaskResult(msg, ok));
	} 

    // -------------------------------------------------------------------------------- 

    boost::shared_ptr<Utils::ThreadSyncEvent> NObjMiniFlasher::IsAborted()
    {
        return m_abort;
    }

	// -------------------------------------------------------------------------------- 
    // Other

    template<class TProcess>
    void NObjMiniFlasher::Run(DRI::IAsyncCmd *pAsyncCmd, MfProcessParams params)
    {
        ESS_ASSERT(m_runner == 0);
        
		AsyncBegin(pAsyncCmd, boost::bind(&T::AbortAsync, this));

        m_runner.reset(new MfProcessRunner<TProcess>(params));
    }

    // -------------------------------------------------------------------------------- 

    void NObjMiniFlasher::CheckPortAndBaudRate()
    {
        if (!iUart::UartProfile::IsValidBaudRate(m_baudRate))
        {
            ThrowRuntimeException(QString("Invalid baud rate: %1").arg(m_baudRate));
        }

        if (m_comPort < 0)
        {
            ThrowRuntimeException(QString("Invalid serial port number: %1").arg(m_comPort));
        }
    }

    // -------------------------------------------------------------------------------- 

    MfProcessParams NObjMiniFlasher::MakeParams(iUart::FlowType flowType, 
                                                const QString &fileName,
                                                dword flashOffset,
                                                dword flashSize,
                                                bool runFlashApp)
    {
        CheckPortAndBaudRate();
        
        MfProcessParams res(this, m_timeouts);
        
        if (!fileName.isEmpty()) res.FileName = FileName(fileName);
        res.UartProfile = 
            MfProcessParams::DefaultUartProfile(m_comPort, m_baudRate, flowType);
        res.FlashOffset = flashOffset;
        res.FlashSize = flashSize;
        res.RunFlashApp = runFlashApp;

        return res;
    }
    
    // --------------------------------------------------------------------------------

	void NObjMiniFlasher::UartBoot(DRI::IAsyncCmd *pAsyncCmd, QString fileName)
	{
        Run<UartBootImpl>(pAsyncCmd, MakeParams(iUart::FlowHardware, fileName));
    }

	// --------------------------------------------------------------------------------

	void NObjMiniFlasher::LoadImg(DRI::IAsyncCmd *pAsyncCmd, QString fileName, 
                                  QString offset, bool run)
	{
        Run<LoadImgImpl>(pAsyncCmd, 
                           MakeParams(iUart::FlowOff, fileName, ParamToDword(offset), 0, run));
	}

    // --------------------------------------------------------------------------------

    void NObjMiniFlasher::FlashDump(DRI::IAsyncCmd *pAsyncCmd, QString destFileName,
                                    QString size, QString offset)
    {
        dword offsetNum = ParamToDword(offset);
        dword sizeNum = ParamToDword(size);
        
        Run<FlashDumpImpl>(pAsyncCmd,
                             MakeParams(iUart::FlowOff, destFileName, offsetNum, sizeNum));
    }

    // --------------------------------------------------------------------------------

    void NObjMiniFlasher::FlashDumpVerify(DRI::IAsyncCmd *pAsyncCmd, QString file, 
                                          QString offset)
    {
        Run<FlashDumpVerifyImpl>(pAsyncCmd,
                                   MakeParams(iUart::FlowOff, file, ParamToDword(offset)));
    }

    // --------------------------------------------------------------------------------
    
    void NObjMiniFlasher::FlashTest(DRI::IAsyncCmd *pAsyncCmd)
    {
        Run<FlashTestImpl>(pAsyncCmd, MakeParams(iUart::FlowOff));
    }

    // --------------------------------------------------------------------------------	

    void NObjMiniFlasher::EraseFlash(DRI::IAsyncCmd *pAsyncCmd, QString size)
    {
        Run<EraseFlashImpl>(pAsyncCmd, 
                              MakeParams(iUart::FlowOff, "", 0, ParamToDword(size)));
        
    }

    // --------------------------------------------------------------------------------	
    
    void NObjMiniFlasher::OnFinish(const NObjMiniFlasher::TaskResult &res)
	{
        m_runner.reset();
        m_abort.reset();
        AsyncComplete(res.Flag, res.Str);
	}

    // --------------------------------------------------------------------------------	

    void NObjMiniFlasher::OnLog(const NObjMiniFlasher::LogData &msg)
    {
        // after Telnet session closed we still can have msgs 
        // from runned async command, so we ignore them
        if (!AsyncActive()) return;

        bool eof = msg.Flag;
        AsyncOutput(msg.Str, eof);
        AsyncFlush();
    }

    // --------------------------------------------------------------------------------	

    void NObjMiniFlasher::AbortAsync()
    {
        m_abort.reset(new Utils::ThreadSyncEvent);
        m_abort->Wait();
        
        // AsyncComplete(false, "Aborted!");
        OnFinish(TaskResult(QString("Aborted"), false));
    }

    // --------------------------------------------------------------------------------	

    void NObjMiniFlasher::UartBenchmark( DRI::IAsyncCmd *pAsyncCmd, int timeToRun )
    {
        AsyncBegin(pAsyncCmd);

        iUart::UartProfile profile = 
            MfProcessParams::DefaultUartProfile(m_comPort, m_baudRate);
        iUart::Uart uart(profile);

        const int CBlockSize = 1024;

        std::vector<Platform::byte> data(CBlockSize, 0);
        for(int i = 0; i < CBlockSize; ++i) data.at(i) = i;
    
        int totalData = 0;
        int start = Platform::GetSystemTickCount();

        while(Platform::GetSystemTickCount() - start < timeToRun)
        {
            uart.IO().Write(&data[0], CBlockSize);
            totalData += CBlockSize;

            Platform::ThreadSleep(0);
        }

        int time = Platform::GetSystemTickCount() - start;
        int speed = (int)((totalData / 1024.0) / (time / 1000.0));

        LogData msg(QString("Speed %1 Kb/sec").arg(speed), true);
        OnLog(msg);

        OnFinish(TaskResult(QString(), true));
    }

    // --------------------------------------------------------------------------------

    dword NObjMiniFlasher::ParamToDword(const QString &param)
    {
        dword res = 0;
        
        if (param.startsWith("0x")) 
        {
            if (Utils::HexStringToInt(param.toStdString(), res, true)) return res;
        }
        else if (Utils::StringToInt(param.toStdString(), res, false)) return res;
        
        ThrowRuntimeException(QString("Invalid parameter value: %1").arg(param));

        return 0; // dummi
    }
   
    // --------------------------------------------------------------------------------
    
} // namespace
