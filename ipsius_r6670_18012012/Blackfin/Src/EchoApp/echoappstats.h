#ifndef __ECHOAPPSTATS__
#define __ECHOAPPSTATS__

// Warning! This file affected SfxProto and CmpProto version!

namespace EchoApp
{
    
    struct EchoAppStats
    {
        bool IsEmpty;

        int CpuUsage;
        int BlockCollisions;
        int ChannelsWithEcho;
        std::string BuildInfo;
        int SbpErrorsCount;
        std::string LastSbpErrors;        

        int HeapFreeBytes;
        int SysTicks;
        int SendCounter;        

        // recv side
        int RecvCounter;
        bool Blocked;
        
        EchoAppStats()
        {
            Clear();
        }

        void Clear()
        {
            IsEmpty = true;

            CpuUsage = 0;
            BlockCollisions = 0;
            ChannelsWithEcho = 0;
            SbpErrorsCount = 0;

            HeapFreeBytes = 0;
            SysTicks = 0;
            SendCounter = 0;        

            RecvCounter = 0;
            Blocked = false;
        }

        std::string ToString(const std::string &sep = "; ") const
        {
            std::ostringstream oss;

            if (!IsEmpty)
            {
                oss << "CPU " << CpuUsage << "%" << sep
                    << "BlockCollisions " << BlockCollisions << sep
                    << "ChannelsWithEcho " << ChannelsWithEcho << sep
                    << "BuildInfo " << BuildInfo << sep
                    << "SbpErrorsCount " << SbpErrorsCount << sep
                    << "LastSbpErrors [" <<  LastSbpErrors << "]" << sep
                    << "Heap " << HeapFreeBytes << sep
                    << "Ticks " << SysTicks << sep
                    << "SendCounter " << SendCounter << sep
                    << "RecvCounter " << RecvCounter << sep
                    << "Blocked " << Blocked;
            }
            else
            {
                oss << "<empty>";
            }

            return oss.str();
        }

        template<class Data, class TStream>
        static void Serialize(Data &data, TStream &s)
        {            

            s
                << data.IsEmpty
                << data.CpuUsage
                << data.BlockCollisions 
                << data.BuildInfo
                << data.ChannelsWithEcho
                << data.SbpErrorsCount
                << data.LastSbpErrors

                << data.HeapFreeBytes
                << data.SysTicks
                << data.SendCounter

                << data.RecvCounter
                << data.Blocked;
        }

    };
    
}  // namespace EchoApp


#endif
