#ifndef __CHMNGPROTODATA__
#define __CHMNGPROTODATA__


#include "Utils/IntSet.h"
#include "Utils/UtilsDateTime.h"
#include "Utils/UtilsClassStr.h"

#include "SafeBiProto/SbpSerialize.h"

#include "Ds2155/liuProfile.h"
#include "Ds2155/HdlcStats.h"
#include "Ds2155/LiuPriStatus.h"
#include "BfTdm/TdmStatistic.h"
#include "TdmMng/TdmStreamStat.h"
#include "TdmMng/TdmChannelStatistic.h"
#include "TdmMng/RtpRecvBufferProfile.h"
#include "E1App/E1ApplicationStats.h"

namespace SBProto 
{	
	class SbpRecvPack; 
	class SbpSendPack;
	class SafeBiProto;
}

namespace iCmp
{
    using std::string;
    using SBProto::SbpRecvPack;
    using SBProto::SafeBiProto;
    using Platform::byte;

    typedef SBProto::SbpSendPackInfo SendPack;

    // ---------------------------------------------------------

    struct BfInitDataBase
    {

    private:

        int CmpProtoVersion;

        static void VerifyVersion(int ver);

        template<class Data, class TStream>
        static void Serialize(Data &d, TStream &s)
        {
            s   << d.CmpProtoVersion;
            VerifyVersion(d.CmpProtoVersion);

            s   << d.DeviceName
                << d.StartRtpPort
                << d.VoiceChannels
                << d.UseAlaw
                << d.TestBodyBaseException
                << d.ThrowFromInit
                << d.Sport0BlocksCount
                << d.Sport0BlockCapacity
                << d.Sport1BlocksCount
                << d.Sport1BlockCapacity
                << d.VoiceSportNum
                << d.VoiceSportChNum
                << d.RtpRecvProfile
                << d.Date
                << d.Time;
        }

    protected:

        void FromPacket(SBProto::PacketRead &s);
        void ToPacket(SBProto::PacketWrite &s) const;

    public:

		ESS_TYPEDEF(BadCmpProtoVersion);
        ESS_TYPEDEF(BadDmaBlockCapacity);

		BfInitDataBase();

		static void SetDefault(BfInitDataBase &res);
		static BfInitDataBase DefaultValue();
		bool IsValid() const;

		string DeviceName;
        int StartRtpPort; 
        string VoiceChannels; 
        bool UseAlaw;

        bool TestBodyBaseException;  // test/debug flag
        bool ThrowFromInit;

        // fields listed below is correcty inited in SetDefault()
        int Sport0BlocksCount, Sport0BlockCapacity;
        int Sport1BlocksCount, Sport1BlockCapacity;
        int VoiceSportNum, VoiceSportChNum;
        TdmMng::RtpRecvBufferProfile RtpRecvProfile;

        Utils::Date Date;
        Utils::Time Time;

        int DmaIrqPeriodMs(int sportNum) const;
    };

    // -----------------------------------------------

    struct BfInitDataE1 : public BfInitDataBase
    {		
        int HdlcCh;
        Ds2155::LiuProfile ProfileLIU;

        BfInitDataE1()
        {
            HdlcCh = -1; 
        }

        BfInitDataE1(const SbpRecvPack &pack)
        {
            SBProto::PacketRead s(pack, 1);
            
            BfInitDataBase::FromPacket(s); // Base            
            Serialize(*this, s);           // self
        }

        void ToPacket(SendPack &p) const
        {
            SBProto::PacketWrite s(p);
            
            BfInitDataBase::ToPacket(s); // Base            
            Serialize(*this, s);         // self
        }

        bool IsValid() const
        {
            if(!ProfileLIU.Verify()) return false;

            return BfInitDataBase::IsValid();
        }

        template<class Data, class TStream>
        static void Serialize(Data &data, TStream &s)
        {
            s 
                << data.HdlcCh
                << data.ProfileLIU;
        }

    };

    // -----------------------------------------------

    struct AbCofidecParams : public SBProto::ISerialazable
    {
        Platform::dword TS1_4;  // cofidec + Uref + sign
                                // lowest byte goto to lowest timeslot

        AbCofidecParams()
        {
            TS1_4 = 0;
        }

        AbCofidecParams(const SbpRecvPack &pack, int offs)
        {            
            SBProto::PacketRead s(pack, offs);
            Serialize(*this, s);
        }

        void ToPacket(SendPack &p) const
        {
            SBProto::PacketWrite s(p);
            Serialize(*this, s);
        }

        void Set(byte cofA, byte cofB, bool enableDivUref)
        {
            byte *p = (byte*)&TS1_4;
            p[0] = cofA;
            p[1] = cofB;
            p[2] = 0x35 | (enableDivUref ? 0x80 : 0x00);
            p[3] = 0xca;
        }

        void SetDefault()
        {
            Set(0x1B, 0x08, true);
        }

		template<class Data, class TStream>
	    static void Serialize(Data &data, TStream &s)
		{
			s << data.TS1_4;
		}

    };

    // -----------------------------------------------

    struct AbLineParams : public SBProto::ISerialazable
    {
        // line damage detection filter params
        bool LineDamageDetectionEnable;
        int LineDamageSwitchPeriodMs;    // 100 ms
        int LineDamageSwitchPercent;     // 75%

        // pulse params (ms)
        int TProtBounceHookDown,        // 15 ms
            TProtBounceHookUp,          // 20 ms
            TProtHookUp,                // 200 ms
            TMaxDialPulce,              // 160 ms
            TMinSerialDial,             // 240 ms
            TProtHookDown;              // 700 ms

        AbLineParams()
        {
            LineDamageDetectionEnable = false;
            LineDamageSwitchPeriodMs = -1;
            LineDamageSwitchPercent = -1;

            TProtBounceHookDown = -1;
            TProtBounceHookUp = -1;
            TProtHookUp = -1;
            TMaxDialPulce = -1;
            TMinSerialDial = -1;
            TProtHookDown = -1;     
        }

        AbLineParams(const SbpRecvPack &pack, int offs)
        {            
            SBProto::PacketRead s(pack, offs);
            Serialize(*this, s);
        }

        void ToPacket(SendPack &p) const 
        {
            SBProto::PacketWrite s(p);
            Serialize(*this, s);
        }

        void SetDefault();

		template<class Data, class TStream>
	    static void Serialize(Data &data, TStream &s)
		{
			s 
				<< data.LineDamageDetectionEnable
				<< data.LineDamageSwitchPeriodMs
				<< data.LineDamageSwitchPercent;

			s
				<< data.TProtBounceHookDown
				<< data.TProtBounceHookUp
				<< data.TProtHookUp
				<< data.TMaxDialPulce
				<< data.TMinSerialDial
				<< data.TProtHookDown;     
		}

    };

    // -----------------------------------------------

    struct BfInitDataAoz : public BfInitDataBase
    {
		static BfInitDataAoz DefaultValue();

        // TDM
        int SignSportNum,      // 0
            SignSportChNum;    // 1
        bool HiTdmChPart;      // false

        // Ring optimization
        int RingMaxDelayMs,           // 500 ms
            RingSearchPoints,         // 3
            RingMaxActiveCount;       // 4

        Platform::byte BoardStateSignByte;   // 0xA6
        int PulseCapturePeriodMs;            // 4 ms  (2 ms точнее, но в два раза больше нагрузка)
                                             // DMA capacity должен быть кратен этому значению!

        AbCofidecParams DefaultCofidecSetup;
        AbLineParams DefaultLineSetup;

        BfInitDataAoz()
        {
            SignSportNum = -1;
            SignSportChNum = -1;
            HiTdmChPart = true;
            RingMaxDelayMs = 0; 
            RingSearchPoints = 0; 
            RingMaxActiveCount = 0;
            BoardStateSignByte = 0;
            PulseCapturePeriodMs = 0;
        }

        BfInitDataAoz( const SbpRecvPack &pack )
        {
            SBProto::PacketRead s(pack, 1);

            
            BfInitDataBase::FromPacket(s);  // Base            
            Serialize(*this, s);            // Own
        }

        void ToPacket( SendPack &p ) const
        {
            SBProto::PacketWrite s(p);
            
            BfInitDataBase::ToPacket(s);    // Base            
            Serialize(*this, s);            // Own
        }

		template<class Data, class TStream>
	    static void Serialize(Data &data, TStream &s)
		{
			s
				<< data.SignSportNum
				<< data.SignSportChNum
				<< data.HiTdmChPart
				<< data.RingMaxDelayMs 
				<< data.RingSearchPoints 
				<< data.RingMaxActiveCount
				<< data.BoardStateSignByte
				<< data.PulseCapturePeriodMs
                << data.DefaultCofidecSetup
                << data.DefaultLineSetup;
		}

    };

    // -----------------------------------------------

    struct BfInitDataPult : public BfInitDataBase
    {

		static BfInitDataPult DefaultValue();

        // debug flags
        bool NoCofidec;
        bool DebugMode;
        bool TraceCofidec;
        bool CofidecBugCapture;

        BfInitDataPult()
        {
            NoCofidec = false;
            DebugMode = false;
            TraceCofidec = false;
            CofidecBugCapture = false;
        }

        BfInitDataPult(const SbpRecvPack &pack)
        {
            SBProto::PacketRead s(pack, 1);            
            BfInitDataBase::FromPacket(s);  // Base            
            Serialize(*this, s);            // Own
        }

        void ToPacket(SendPack &p) const
        {
            SBProto::PacketWrite s(p);            
            BfInitDataBase::ToPacket(s); // Base           
            Serialize(*this, s);         // Own
        }

        template<class Data, class TStream>
        static void Serialize(Data &data, TStream &s)
        {
            s << data.NoCofidec 
              << data.DebugMode
              << data.TraceCofidec
              << data.CofidecBugCapture;
        }

		bool IsValid() const { return BfInitDataBase::IsValid(); }

    };

    // -----------------------------------------------

    struct BfLiuInfo
    {
        Ds2155::PriStatus LiuStatus;
        Ds2155::HdlcStatistic HdlcStats; 

        BfLiuInfo()
        {
        }

        BfLiuInfo(const SbpRecvPack &pack)
        {
            SBProto::PacketRead s(pack, 1);
            Serialize(*this, s);
        }

        void ToPacket(SendPack &p) const
        {
            SBProto::PacketWrite s(p);
            Serialize(*this, s);
        }

        std::string ToString(const char *sep = "\n") const
        {
            std::ostringstream out;
            out << "Liu status:" << sep
                << LiuStatus.ToString() << sep
                << "Hdlc statistic:" << sep
                << HdlcStats.ToString();
            return out.str();
        }

	private:

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
		{
            s 
                << data.LiuStatus
                << data.HdlcStats;
		}

    };

    // -----------------------------------------------

    struct BfTdmInfo
    {
		Platform::dword FrequencySys;
        BfTdm::TdmStatistic TdmStat;
        TdmMng::TdmStreamStat TdmMngStat;

        BfTdmInfo()
        {
            FrequencySys = 0;
        }

        BfTdmInfo(const SbpRecvPack &pack)
        {
            SBProto::PacketRead s(pack, 1);
            Serialize(*this, s);
        }

        void ToPacket(SendPack &p) const
        {
            SBProto::PacketWrite s(p);
            ESS_ASSERT(FrequencySys != 0);
            Serialize(*this, s);
        }

        std::string ToString(const char *sep = "\n") const
        {
            std::ostringstream out;
            out << "Tdm statistic:" << sep
                << TdmStat.ToString() << sep
                << "TdmMng statistic: " << sep
                << TdmMngStat.ToString(FrequencySys);
            return out.str();
        }

	private:

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
		{
			s << data.FrequencySys
              << data.TdmStat
              << data.TdmMngStat;
		}

    };

    // -----------------------------------------------

    struct BfChannelInfo
    {
        TdmMng::TdmChannelStatistic Stat;

        BfChannelInfo()
        {
        }

        BfChannelInfo(const SbpRecvPack &pack)
        {
            SBProto::PacketRead s(pack, 1);
            Serialize(*this, s);
        }

        void ToPacket(SendPack &p) const
        {
            SBProto::PacketWrite s(p);
            Serialize(*this, s);
        }

        std::string ToString(const char *sep = "\n") const
        {
            return Stat.ToString(sep);
        }

	private:

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
		{
            s << data.Stat;
		}
    };

    // -----------------------------------------------
    
    struct BfInitInfo
    {
        std::vector<int> RtpPorts;  // rtp ports array for BfInitData::VoiceChannels
        std::string BuildString;
        int FreeHeapBytes;

        ESS_TYPEDEF(BadRtpPortsString);

        BfInitInfo();

        BfInitInfo(const SbpRecvPack &pack);

        void ToPacket(SendPack &p) const;

    }; 

    // -----------------------------------------------

    struct BfStateInfo
    {
        E1App::E1ApplicationStats Stats;

        BfStateInfo()
        {
        }

        BfStateInfo( const SbpRecvPack &pack )
        {
            SBProto::PacketRead s(pack, 1);
            Serialize(*this, s);
        }

        void ToPacket( SendPack &p ) const
        {
            SBProto::PacketWrite s(p);
            Serialize(*this, s);
        }

        std::string ToString(const char *sep = "; ") const
        {
            return Stats.ToString(sep);
        }

	private:

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
		{
            s << data.Stats;
		}

    };

    // -----------------------------------------------

    struct BfGlobalSetup
    {
        bool LogEnable; 
        string UdpHostPort;

        /* 
            0            -- for disable 
            ~5 * 1000    -- typical value (watchdog serve with interval E1AppConfig.AppPollTimeMs)
            ~1           -- small value for do reset by watchdog (hack)
            ~1000 * 1000 -- huge value for assert in watchdog constructor (hack)
        */
        Platform::dword WatchdogTimeoutMs;  
        bool  ResetOnFatalError;

        bool DoLoopForever;  // for test watchdog
        bool DoAssert;       // for test reset on fatal error
        bool DoOutOfMemory;  // for test kernel panic
        bool DoEchoHalt;     // for test Echo DSP halt

        bool SetupNullEcho;
        bool AsyncDeleteBody;
        bool AsyncDropConnOnException;
        bool DoSafeRefTest;

        BfGlobalSetup()
        {
            LogEnable = false;
            WatchdogTimeoutMs = 0;
            ResetOnFatalError = false;
            DoLoopForever = false;
            DoAssert = false;
            DoOutOfMemory = false;
            DoEchoHalt = false;
            SetupNullEcho = false;
            AsyncDeleteBody = false;
            AsyncDropConnOnException = false;
            DoSafeRefTest = false;
        }

        BfGlobalSetup( const SbpRecvPack &pack )
        {
            SBProto::PacketRead s(pack, 1);
            Serialize(*this, s);
        }

        void ToPacket( SendPack &p ) const
        {
            SBProto::PacketWrite s(p);
            Serialize(*this, s);
        }

	private:

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
		{
			s << data.LogEnable
		      << data.UdpHostPort
			  << data.WatchdogTimeoutMs 
			  << data.ResetOnFatalError
			  << data.DoLoopForever
			  << data.DoAssert
              << data.DoOutOfMemory
              << data.DoEchoHalt
              << data.SetupNullEcho
              << data.AsyncDeleteBody
              << data.AsyncDropConnOnException
              << data.DoSafeRefTest;
		}

    };

    
}  // namespace iCmp

#endif
