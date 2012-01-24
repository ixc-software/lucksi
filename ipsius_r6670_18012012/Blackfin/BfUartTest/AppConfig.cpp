#include "stdafx.h"

#include "iLog/LogManager.h"
#include "Ds2155/boardds2155.h"

#include "MngLwip.h"
#include "AppConfig.h"

using TdmMng::TdmManagerProfile; 

// ---------------------------------------------------------------

enum
{
        // "виртуальные" параметры -- для расчета параметров реальных
        CMaxChannels            = 30,   // максимальное число TDM каналов
        CTypicalRtpPackSize     = 160,

		// basic
		CThreadRunnerSleepInterval = 60 * 1000,

        CEnterTdmTestOnStartup = false,
        CEnterUartTestOnStartup = true,        
	
        // TdmProfile -- глубина буферизации (влияет на задержку звука)
        CDmaBlocksCount         = 6,
        CDmaBlockCapacity       = 160,

        // RTP ports
        CRtpStartPort           = 2000,
        CRtpPortCount           = 1024,

        // RTP recv buffering
        CRtpMinBufferingDepth      = 512,
        CRtpMaxBufferingDepth      = 8 * 1024,
        CRtpUpScalePercent         = 150,
        CRtpQueueDepth             = (CRtpMaxBufferingDepth / CTypicalRtpPackSize) + 4,
        CRtpDropCount              = (CRtpQueueDepth / 2), 

        // Buffers
        CBidirBuffSize              = 512,  
        CBidirBuffOffset            = 32,
        CBidirBuffCount             = CMaxChannels * (CRtpQueueDepth + 6),
        
        // HDLC
		CHdlcMemPoolBlockSize      = 256,
	    CHdlcBlocksNum             = 16,
		CHdlcBuffOffset            = 0,
        CHdlcMaxPackSize           = 200,	        
};

// буфер вмещает типичный RTP пакет
BOOST_STATIC_ASSERT( (CBidirBuffSize - CBidirBuffOffset) > CTypicalRtpPackSize );

// буфер вмещает DMA block
BOOST_STATIC_ASSERT( (CBidirBuffSize - CBidirBuffOffset) > CDmaBlockCapacity );

// log
const char *CLogUdpHost = "192.168.0.158";  // try broadcast
const int   CLogUdpPort = 56001;

// network
const bool CUseDHCP         = false;
const char *CNetIP          = "192.168.0.49";
const char *CNetGateway     = "192.168.0.35";
const char *CNetMask        = "255.255.255.0";

// ---------------------------------------------------------------

iLogW::LogSettings AppConfig::GetLogSettings()
{
    iLogW::LogSettings settings;

    // setup
    settings.SetStoreRecordsLimit(0, 0);  // don't store records
    settings.TimestampOn();
    settings.setCountSyncroToStore(1);  // sync on every records
    settings.UseRecordsReorder(false);
    settings.DublicateSessionNamesDetection(false);
    return settings;
}

// ---------------------------------------------------------------

Utils::HostInf AppConfig::GetLogHost()
{
	return Utils::HostInf(CLogUdpHost, CLogUdpPort);
}

// ---------------------------------------------------------------

Ds2155::HdlcProfile AppConfig::CreateHdlcProfile()
{
    return Ds2155::HdlcProfile(
        CHdlcMemPoolBlockSize, CHdlcBlocksNum, CHdlcBuffOffset, CHdlcMaxPackSize);
}

// ---------------------------------------------------------------

int AppConfig::GetThreadRunnerSleepInterval()
{
	return CThreadRunnerSleepInterval;
}

// ---------------------------------------------------------------

bool AppConfig::EnterTdmTestOnStartup()
{
    return CEnterTdmTestOnStartup;
}

// ---------------------------------------------------------------

bool AppConfig::EnterUartTestOnStartup()
{
    return CEnterUartTestOnStartup;
}

// ---------------------------------------------------------------

void AppConfig::GetNetworkSettings(LwIP::NetworkSettings &cfg)
{
    if (CUseDHCP)
    {
        cfg.UseDHCP();
        return;
    }

    cfg.Set(CNetIP, CNetGateway, CNetMask);
}





