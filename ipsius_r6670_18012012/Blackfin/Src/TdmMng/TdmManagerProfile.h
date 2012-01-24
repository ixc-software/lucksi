#ifndef TDMMANAGERPROFILE_H
#define TDMMANAGERPROFILE_H

#include "iCore/MsgThread.h"
#include "iLog/LogWrapper.h"
#include "BfTdm/TdmProfile.h"
#include "iRtp/RtpParams.h"
#include "RtpRecvBufferProfile.h"
#include "TdmHAL.h"


namespace TdmMng
{
    // just many open fields
    struct TdmManagerProfile
    {
        TdmHAL &Hal;
        Utils::SafeRef<iLogW::ILogSessionCreator> LogCreator; 
        iCore::MsgThread &Thread; 

        std::string LocalIp;

        iRtp::RtpInfraParams RtpInfraPar;

        int BidirBuffSize;
        int BidirBuffCount;
        int BidirBuffOffset; // резерв для дописывания в начало буфера используемого для формирования исходящих пакетов

        RtpRecvBufferProfile BufferingProf;

        TdmManagerProfile(iCore::MsgThread &thread,
			TdmHAL &hal, 
			Utils::SafeRef<iLogW::ILogSessionCreator> logCreator) : 
            Hal(hal),
            LogCreator(logCreator),
            Thread(thread)
        {
            LocalIp = "";

            BidirBuffSize = 0;
            BidirBuffCount = 0;
            BidirBuffOffset = 0;            
        }

        bool IsCorrect() const
        {
			if(LogCreator.IsEmpty()) return false;
            if (LocalIp.empty()) return false;

            if (!RtpInfraPar.IsValid())          return false;

            if (BidirBuffSize == 0)         return false;
            if (BidirBuffCount == 0)        return false;
            if (BidirBuffOffset == 0)       return false;

            if (!BufferingProf.IsCorrect()) return false;

            return true;
        }

        std::string getAsString() const
        {
            std::ostringstream ss;
            ss << "LocalIp = " << LocalIp
                << ", MinRtpPort = " << RtpInfraPar.MinPort()
                << ", MaxRtpPort = " << RtpInfraPar.MaxPort()
                << ", BidirBuffSize = " << BidirBuffSize
                << ", BidirBuffCount = " << BidirBuffCount
                << ", BidirBuffOffset = " << BidirBuffOffset
                << "BufferingProf:{" << BufferingProf.getAsString() << "}";

            return ss.str();
        }
    };

} // namespace TdmMng

#endif
