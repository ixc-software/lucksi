#include "stdafx.h"

#include "Utils/DateTimeCapture.h"

#include "SafeBiProto/SbpRecvPack.h"

#include "ChMngProtoData.h"
#include "ChMngProto.h"


namespace iCmp
{

    struct DtFormat
    {
        UTILS_CLASS_STR(CDate, "dd.MM.yyyy");
        UTILS_CLASS_STR(CTime, "hh:mm:ss");
    };

    // ---------------------------------------------------------

    SBProto::PacketRead& operator << (SBProto::PacketRead &s, Utils::Date &data)
    {
        std::string val;
        s << val;

        data = Utils::Date::FromString(val, DtFormat::CDate()); 
        return s;
    }

    SBProto::PacketWrite& operator << (SBProto::PacketWrite &s, const Utils::Date &data)
    {
        Utils::DateTimeCapture curr;
        curr.Capture();

        s << ( curr.date().IsNull() ? "" : curr.date().ToString(DtFormat::CDate()) );

        return s;
    }

    // ---------------------------------------------------------

    SBProto::PacketRead& operator << (SBProto::PacketRead &s, Utils::Time &data)
    {
        std::string val;
        s << val;

        data = Utils::Time::FromString(val, DtFormat::CTime()); 
        return s;
    }

    SBProto::PacketWrite& operator << (SBProto::PacketWrite &s, const Utils::Time &data)
    {
        Utils::DateTimeCapture curr;
        curr.Capture();

        s << ( curr.time().IsNull() ? "" : curr.time().ToString(DtFormat::CTime()) );

        return s;
    }


	// -----------------------------------------------
	// BfInitDataBase

	void BfInitDataBase::FromPacket(SBProto::PacketRead &s)
    {
        Serialize(*this, s);
    }

	// -----------------------------------------------

	void BfInitDataBase::ToPacket(SBProto::PacketWrite &s) const 
    {
        Serialize(*this, s);
    }

	// -----------------------------------------------

	BfInitDataBase::BfInitDataBase()
	{
		CmpProtoVersion = CCmpProtoVersion;

		DeviceName = "Device";
		StartRtpPort = -1; 
		VoiceChannels = ""; 
		UseAlaw = true;
        TestBodyBaseException = false;
        ThrowFromInit = false;
		Sport0BlocksCount = -1;
		Sport0BlockCapacity = -1;
		Sport1BlocksCount = -1;
		Sport1BlockCapacity = -1;
		VoiceSportNum = -1; 
		VoiceSportChNum = -1;
	}

	// -----------------------------------------------

	void BfInitDataBase::SetDefault(BfInitDataBase &res)
    {
		res.StartRtpPort = 8000;
        res.Sport0BlocksCount = 6;
        res.Sport0BlockCapacity = 160;
        res.Sport1BlocksCount = 6;
        res.Sport1BlockCapacity = 160;
        res.VoiceSportNum = 0; 
        res.VoiceSportChNum = 0;

        res.RtpRecvProfile.MinBufferingDepth = 512;
        res.RtpRecvProfile.MaxBufferingDepth = 8 * 1024;
        res.RtpRecvProfile.UpScalePercent = 150;
        res.RtpRecvProfile.QueueDepth = (res.RtpRecvProfile.MaxBufferingDepth / 160) + 4;
        res.RtpRecvProfile.DropCount = res.RtpRecvProfile.QueueDepth / 2;
    }

	// -----------------------------------------------

	BfInitDataBase BfInitDataBase::DefaultValue()
	{
		BfInitDataBase res;
        SetDefault(res);
		return res;
	}

	// -----------------------------------------------

	bool BfInitDataBase::IsValid() const
	{
		return (Sport0BlockCapacity % 8) == 0 && 
			(Sport1BlockCapacity % 8) == 0 && 
			!(StartRtpPort & 1) &&
			RtpRecvProfile.IsCorrect();
	}

	// -----------------------------------------------

	int BfInitDataBase::DmaIrqPeriodMs(int sportNum) const
    {
        ESS_ASSERT(sportNum < 2);

        int capacity = (sportNum == 0) ? Sport0BlockCapacity : Sport1BlockCapacity;

        if (capacity % 8 != 0) ESS_THROW(BadDmaBlockCapacity);
        return capacity / 8;
    }

    void BfInitDataBase::VerifyVersion(int ver) 
    {
        if (ver == CCmpProtoVersion) return;

        std::ostringstream oss;
        oss << "Version is " << CCmpProtoVersion << " get " << ver;
        ESS_THROW_MSG(BadCmpProtoVersion, oss.str());
    }
	
	// -----------------------------------------------

	void AbLineParams::SetDefault()
    {
        LineDamageDetectionEnable = true;
        LineDamageSwitchPeriodMs = 100;
        LineDamageSwitchPercent = 75;  

        TProtBounceHookDown = 15;
        TProtBounceHookUp = 20;
        TProtHookUp = 200;
        TMaxDialPulce = 160;
        TMinSerialDial = 240;
        TProtHookDown = 700;
    }

    // -----------------------------------------------
	// BfInitDataAoz

	iCmp::BfInitDataAoz BfInitDataAoz::DefaultValue()
	{
		BfInitDataAoz res;

		BfInitDataBase::SetDefault(res);

		res.SignSportNum = 0;
		res.SignSportChNum = 1;
		res.HiTdmChPart = false;

		// Ring optimization
		res.RingMaxDelayMs = 500;
		res.RingSearchPoints = 3;
		res.RingMaxActiveCount = 4;

		res.BoardStateSignByte = 0xA6;
		res.PulseCapturePeriodMs = 4; 

		res.DefaultCofidecSetup.SetDefault();
		res.DefaultLineSetup.SetDefault();

		return res;
	}

	// -----------------------------------------------

	iCmp::BfInitDataPult BfInitDataPult::DefaultValue()
	{
		BfInitDataPult res;

		BfInitDataBase::SetDefault(res);

		return res;
	}

	// -----------------------------------------------
	// BfInitInfo

	BfInitInfo::BfInitInfo()
	{
		FreeHeapBytes = 0;
	}

	// -----------------------------------------------

	BfInitInfo::BfInitInfo( const SbpRecvPack &pack )
	{
		if (!Utils::IntContParse(pack[1].AsString(), RtpPorts))
		{
			ESS_THROW(BadRtpPortsString);
		}

		BuildString     = pack[2].AsString();
		FreeHeapBytes   = pack[3].AsInt32();
	}

	// -----------------------------------------------

	void BfInitInfo::ToPacket( SendPack &p ) const
	{
		p.WriteString( Utils::IntContToString(RtpPorts) );
		p.WriteString(BuildString);
		p.WriteInt32(FreeHeapBytes);
	}

}  // namespace iCmp

