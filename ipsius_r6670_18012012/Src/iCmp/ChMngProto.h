#pragma once

/*
	Channels Managment Protocol (CMP)

	Thru this protocol PC manages TDM channels on Blackfin.
	CMP works over SafeBiProto (TCP/IP).

    Don't forget to change CCmpProtoVersion after modify this header!
    Don't forget to register new commands in factory!
	
*/

#include "Utils/IBasicInterface.h"
#include "Utils/HostInf.h"
#include "TdmMng/TdmException.h"
#include "TdmMng/TdmEvents.h"
#include "SafeBiProto/SbpRecvPack.h"
#include "SafeBiProto/CmdReceiveEngine.h"
#include "SafeBiProtoExt/ISbpConnection.h"
#include "Utils/UtilsDateTime.h"
#include "Utils/UtilsClassStr.h"
#include "iDSP/GenUtils.h"
#include "iPult/CofidecState.h"

#include "ChMngProtoShared.h"
#include "ChMngProtoParams.h"

namespace iCmp
{
    enum { CCmpProtoVersion = 62 };
    enum { CPacketMaxSizeBytes = 1024 * 8};
}

#include "ChMngProtoData.h"   // don't forget to change CCmpProtoVersion - !

namespace iCmp
{
    using SBProto::ISafeBiProtoForSendPack;
    using SBProto::ICmdInterface;
    using SBProtoExt::ISbpConnection;
    using Utils::SafeRef;

    // -----------------------------------------------

    // callback for recv command Init() at Blackfin
    class IBfSideRecvGlobal : public Utils::IBasicInterface
    {
    public:

        virtual void OnPcCmdInitE1  (SafeRef<ISbpConnection> src, const BfInitDataE1 &data)   = 0;
        virtual void OnPcCmdInitAoz (SafeRef<ISbpConnection> src, const BfInitDataAoz &data)  = 0;
        virtual void OnPcCmdInitPult(SafeRef<ISbpConnection> src, const BfInitDataPult &data) = 0;

        virtual void OnPcCmdGlobalSetup(SafeRef<ISbpConnection> src, const BfGlobalSetup &data) = 0;
        virtual void OnPcCmdStateInfo(SafeRef<ISbpConnection> src) = 0;
    };

    // -----------------------------------------------

    // callback for recv command at Blackfin
    class IBfSideRecv : public Utils::IBasicInterface
    {
    public:

        // TDM related 
        virtual void OnPcCmdStartGen(const string &devName, int chNum, bool toLine, 
            const iDSP::Gen2xProfile &data) = 0;
        virtual void OnPcCmdStopGen(const string &devName, int chNum, bool toLine) = 0;

        virtual void OnPcCmdChEchoSuppressMode(const string &devName, int chNum, int taps, bool useInternal) = 0;
        virtual void OnPcCmdChCaptureMode(const string &devName, int chNum, bool enabled) = 0;

        virtual void OnPcCmdGetRtpPort(const string &devName, int chNum) = 0;
        virtual void OnPcCmdGetTdmInfo(const string &devName) = 0;
        virtual void OnPcCmdGetChannelInfo(const string &devName, int chNum, bool lastCall) = 0;

        virtual void OnPcCmdStartRecv(const string &devName, int chNum) = 0;
        virtual void OnPcCmdStopRecv(const string &devName, int chNum) = 0;

        virtual void OnPcCmdStartSend(const string &devName, int chNum, 
            const std::string &ip, int port) = 0;
        virtual void OnPcCmdStopSend(const string &devName, int chNum, 
            const std::string &ip, int port) = 0;

        virtual void OnPcCmdSetFreqRecvMode(const string &devName, int chMask, bool useInternal,
            const string &recvName, FreqRecvMode mode, const string &params) = 0;

        // conf related 
        virtual void OnPcCmdConfCreate(int confHandle, bool autoMode, int blockSize) = 0;
        virtual void OnPcCmdConfDelete(int confHandle) = 0;
        virtual void OnPcCmdConfMode(int confHandle, const string &mode) = 0;
        virtual void OnPcCmdConfCreateTdmPoint(int pointHandle, const string &devName, int chNum) = 0;
        virtual void OnPcCmdConfCreateRtpPoint(int pointHandle, const string &codecName, const string &codecParams) = 0;
        virtual void OnPcCmdConfCreateGenPoint(int pointHandle, const iDSP::Gen2xProfile &profile) = 0;
        virtual void OnPcCmdConfDeletePoint(int pointHandle) = 0;
        virtual void OnPcCmdConfRtpPointSend(int pointHandle, const string &ip, int port) = 0;
        virtual void OnPcCmdConfAddPointToConf(int pointHandle, int confHandle, bool send, bool recv) = 0;
        virtual void OnPcCmdConfRemovePointFromConf(int pointHandle) = 0;
        virtual void OnPcCmdConfGetState(int confHandle) = 0;

        // E1 related
        virtual void OnPcHdlcSend(/* const string &devName, */ std::vector<byte> &pack) = 0;
        virtual void OnPcCmdGetLiuInfo(const string &devName) = 0;

        // AOZ related
        virtual void OnPcCmdAbSetCofidecSetup(const string &devName, int chMask, const AbCofidecParams &params) = 0;
        virtual void OnPcCmdAbSetLineParams(const string &devName, int chMask, const AbLineParams &params) = 0;
        virtual void OnPcCmdAbCommand(const string &devName, int chMask, const string &cmdName, const string &cmdParams) = 0;

        // Pult related
        virtual void OnPcCmdPultCofidec(const string &devName, const iPult::CofidecState &state) = 0;
    };

    // ------------------------------------------------
    // callback for recv command at PC

    class IPcSideRecv : public Utils::IBasicInterface
    {
    public:
        virtual void OnBfRespGetRtpPort(int rtpPort) = 0;
        virtual void OnBfResponse(/* TdmErrorCode */ int errCode, const std::string &desc) = 0;
        virtual void OnBfRespGetTdmInfo(const BfTdmInfo &info) = 0;
        virtual void OnBfRespGetChannelInfo(const BfChannelInfo &info) = 0;
        virtual void OnBfRespCmdInit(const BfInitInfo &info) = 0;
        virtual void OnBfRespStateInfo(const BfStateInfo &info) = 0;

        virtual void OnBfRespConf(const string &params) = 0;

        // E1
        virtual void OnBfRespGetLiuInfo(const BfLiuInfo &info) = 0;
    };

	// ------------------------------------------------
	// Event from board

	class IPcSideEventRecv : public Utils::IBasicInterface
	{
	public:
		virtual void OnBfTdmEvent(const std::string &src, 
			const std::string &event, const std::string &params) = 0;

		virtual void OnBfGetChannelData(const std::string &devName, int chNum, 
			const std::vector<byte> &rx, 
			const std::vector<byte> &tx,
			const std::vector<byte> &rxFixed) = 0;

		// E1
		virtual void OnBfHdlcRecv(/* const string &devName, */ const std::vector<byte> &pack) = 0;

		// AOZ
		virtual void OnBfAbEvent(const std::string &deviceName, int chNumber, 
			const std::string &eventName, const std::string &eventParams) = 0;
	};

    // --------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------

    // resp is BfRespCmdInit or BfResponse(for error)
    struct PcCmdInitE1 
    {
        static string CName() { return "PcCmdInitE1"; }

        static bool ProcessPacket(const SbpRecvPack &pack, SafeRef<ISbpConnection> src, 
            IBfSideRecvGlobal &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            BfInitDataE1 data(pack);
            callback.OnPcCmdInitE1(src, data);
            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, const BfInitDataE1 &data)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            data.ToPacket(p);
        }
    };

    // --------------------------------------------------------------------------------------

    // resp is BfRespCmdInit or BfResponse(for error)
    struct PcCmdInitAoz
    {
        static string CName() { return "PcCmdInitAoz"; }

        static bool ProcessPacket(const SbpRecvPack &pack, SafeRef<ISbpConnection> src, 
            IBfSideRecvGlobal &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            BfInitDataAoz data(pack);
            callback.OnPcCmdInitAoz(src, data);
            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, const BfInitDataAoz &data)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            data.ToPacket(p);
        }
    };

    // --------------------------------------------------------------------------------------

    // resp is BfRespCmdInit or BfResponse(for error)
    struct PcCmdInitPult
    {
        static string CName() { return "PcCmdInitPult"; }

        static bool ProcessPacket(const SbpRecvPack &pack, SafeRef<ISbpConnection> src, 
            IBfSideRecvGlobal &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            BfInitDataPult data(pack);
            callback.OnPcCmdInitPult(src, data);
            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, const BfInitDataPult &data)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            data.ToPacket(p);
        }
    };
    
    // --------------------------------------------------------------------------------------

    // resp for PcCmdInitE1
    struct BfRespCmdInit : public ICmdInterface<IPcSideRecv>
    {
        static string CName() { return "BfRespCmdInit"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideRecv &callback) const
        {
            BfInitInfo info(pack);
            callback.OnBfRespCmdInit(info);
        }
        static void Send(ISafeBiProtoForSendPack &proto, const BfInitInfo &info)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            info.ToPacket(p);
        }
    };


    // -------------------------------------------------------------------------------------

    // resp is BfResponse
    struct PcCmdGlobalSetup 
    {
        static string CName() { return "PcCmdGlobalSetup"; }

        static bool ProcessPacket(const SbpRecvPack &pack, SafeRef<ISbpConnection> src,
            IBfSideRecvGlobal &callback) 
        {
            if (pack[0].AsString() != CName()) return false;

            BfGlobalSetup data(pack);
            callback.OnPcCmdGlobalSetup(src, data);

            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, const BfGlobalSetup &data)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            data.ToPacket(p);
        }

    };

    // -------------------------------------------------------------------------------------

    // resp is BfRespStateInfo    
    struct PcCmdStateInfo
    {
        static string CName() { return "PcCmdStateInfo"; }

        static bool ProcessPacket(const SbpRecvPack &pack, SafeRef<ISbpConnection> src,
            IBfSideRecvGlobal &callback) 
        {
            if (pack[0].AsString() != CName()) return false;

            callback.OnPcCmdStateInfo(src);

            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendPack p(proto);

            p.WriteString( CName() );
        }

    }; 

    // ------------------------------------

    // resp for PcCmdStateInfo
    struct BfRespStateInfo : public ICmdInterface<IPcSideRecv>
    {
        static string CName() { return "BfRespStateInfo"; }
        string Name() const { return CName(); }

        static bool ProcessPacketStatic(const SbpRecvPack &pack, BfStateInfo &info)
        {
            if (pack[0].AsString() != CName()) return false;

			info = BfStateInfo(pack);
            return true;
        }

		void ProcessPacket(const SbpRecvPack &pack, IPcSideRecv &callback) const
		{
			BfStateInfo info(pack);
			callback.OnBfRespStateInfo(info);
		}

        static void Send(ISafeBiProtoForSendPack &proto, const BfStateInfo &info)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            info.ToPacket(p);
        }

    };

    // ------------------------------------

    // no response
    struct PcHdlcSend : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcHdlcSend"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            std::vector<byte> recvBuff;
            pack[1].AsBinary(recvBuff);

            callback.OnPcHdlcSend(recvBuff);
        }

        static void Send(ISafeBiProtoForSendPack &proto,
            const void *pPack, int size)
        {
            ESS_ASSERT(pPack != 0);
            ESS_ASSERT(size > 0);

            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteBinary(pPack, size);
        }
    };

    // ------------------------------------

    // no response
	struct BfHdlcRecv : public ICmdInterface<IPcSideEventRecv>
    {
	public:
        static string CName() { return "BfHdlcRecv"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideEventRecv &callback) const
        {
			std::vector<byte> recvBuff;
			pack[1].AsBinary(recvBuff);

			callback.OnBfHdlcRecv(recvBuff);
        }

		static bool ProcessPacketStatic(const SbpRecvPack &pack, std::vector<byte> &data)
		{
			if (pack[0].AsString() != CName()) return false;

			pack[1].AsBinary(data);
			return true;
		}

        static void Send(ISafeBiProtoForSendPack &proto,
            const void *pPack, int size)
        {
            ESS_ASSERT(pPack != 0);
            ESS_ASSERT(size > 0);

            SendPack p(proto);
            p.WriteString( CName() );          
            p.WriteBinary(pPack, size);
        }

    };

    // -------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------

    /*
    // resp is BfResponse
    struct PcCmdStartGen : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdStartGen"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            string devName = pack[1].AsString();
            int chNum = pack[2].AsInt32();

            BfStartGenData data(pack);

            callback.OnPcCmdStartGen(devName, chNum, data);
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, int chNum, const BfStartGenData &data)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            p.WriteString(devName);
            p.WriteInt32(chNum);

            data.ToPacket(p);
        }

    }; */

    // resp is BfResponse
    struct PcCmdStartGen : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdStartGen"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            string devName = pack[1].AsString();
            int chNum = pack[2].AsInt32();
            bool toLine = pack[3].AsBool();

            iDSP::Gen2xProfile data;
            {
                SBProto::PacketRead s(pack, 4);
				iDSP::Gen2xProfile::Serialize(data, s);
            }

            callback.OnPcCmdStartGen(devName, chNum, toLine, data);
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, int chNum, bool toLine, const iDSP::Gen2xProfile &data)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            p.WriteString(devName);
            p.WriteInt32(chNum);
            p.WriteBool(toLine);

            {
                SBProto::PacketWrite s(p);
                iDSP::Gen2xProfile::Serialize(data, s);
            }
        }

    };


    // -------------------------------------------------------------------------------------

    // resp is BfResponse
    struct PcCmdStopGen : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdStopGen"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdStopGen(pack[1].AsString(), pack[2].AsInt32(), pack[3].AsBool());
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
                         const string &devName, int chNum, bool toLine) 
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chNum);
            p.WriteBool(toLine);
        }

    };

    // -------------------------------------------------------------------------------------

    // resp is BfResponse
    struct PcCmdChEchoSuppressMode : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdChEchoSuppressMode"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdChEchoSuppressMode(
                pack[1].AsString(), pack[2].AsInt32(), pack[3].AsInt32(), pack[4].AsBool());  
        }

        // taps use from [32, 64, 128]
        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, int chNum, int taps, bool useInternal) 
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chNum);
            p.WriteInt32(taps);
            p.WriteBool(useInternal);
        }

    };

    // -------------------------------------------------------------------------------------

    // resp is BfRespGetRtpPort
    struct PcCmdGetRtpPort : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdGetRtpPort"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdGetRtpPort(pack[1].AsString(), pack[2].AsInt32());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const string &devName, int chNum)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteString(devName);
            p.WriteInt32(chNum);
        }

    };

    // ---------------------------------------------

    // resp for PcCmdGetRtpPort
    struct BfRespGetRtpPort : public ICmdInterface<IPcSideRecv>
    {
        static string CName() { return "BfRespGetRtpPort"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideRecv &callback) const
        {
            callback.OnBfRespGetRtpPort( pack[1].AsInt32() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, int rtpPort)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteInt32(rtpPort);
        }

    };

    // ---------------------------------------------

    // resp is BfRespGetLiuInfo
    struct PcCmdGetLiuInfo : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdGetLiuInfo"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdGetLiuInfo( pack[1].AsString() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, const string &devName)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteString(devName);
        }

    };

    // ---------------------------------------------

    // resp for PcCmdGetLiuInfo
    struct BfRespGetLiuInfo : public ICmdInterface<IPcSideRecv>
    {
        static string CName() { return "BfRespGetLiuInfo"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideRecv &callback) const
        {
            BfLiuInfo info(pack);
            callback.OnBfRespGetLiuInfo(info);
        }

        static void Send(ISafeBiProtoForSendPack &proto, /* const */ BfLiuInfo &info)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            info.ToPacket(p);
        }

    };

    // ---------------------------------------------

    // resp is BfRespGetTdmInfo
    struct PcCmdGetTdmInfo : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdGetTdmInfo"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdGetTdmInfo( pack[1].AsString() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, const string &devName)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteString(devName);
        }

    };


    // ---------------------------------------------

    // resp for PcCmdGetTdmInfo
    struct BfRespGetTdmInfo : public ICmdInterface<IPcSideRecv>
    {
        static string CName() { return "BfRespGetTdmInfo"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideRecv &callback) const
        {
            BfTdmInfo info(pack);
            callback.OnBfRespGetTdmInfo(info);
        }

        static void Send(ISafeBiProtoForSendPack &proto, const BfTdmInfo &info)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            info.ToPacket(p);
        }

    };

    // ---------------------------------------------

    // resp is BfRespGetChannelInfo
    struct PcCmdGetChannelInfo : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdGetChannelInfo"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdGetChannelInfo( pack[1].AsString(), pack[2].AsInt32(), pack[3].AsBool() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, int chNum, bool lastCall)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chNum);
            p.WriteBool(lastCall);
        }

    };

    // ---------------------------------------------

    // resp for PcCmdGetChannelInfo
    struct BfRespGetChannelInfo : public ICmdInterface<IPcSideRecv>
    {
        static string CName() { return "BfRespGetChannelInfo"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideRecv &callback) const
        {
            BfChannelInfo info(pack);
            callback.OnBfRespGetChannelInfo(info);
        }

        static void Send(ISafeBiProtoForSendPack &proto, const BfChannelInfo &info)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            info.ToPacket(p);
        }

    };


    // ---------------------------------------------

    // resp is BfResponse
    struct PcCmdStartRecv : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdStartRecv"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdStartRecv(pack[1].AsString(), pack[2].AsInt32());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const string &devName, int chNum)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chNum);
        }

    };

    // -----------------------------------------------

    // resp is BfResponse
    struct PcCmdStopRecv : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdStopRecv"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdStopRecv(pack[1].AsString(), pack[2].AsInt32());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const string &devName, int chNum)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chNum);
        }

    };

    // ---------------------------------------------

    // resp is BfResponse
    struct PcCmdStartSend : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdStartSend"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdStartSend(
                pack[1].AsString(), 
                pack[2].AsInt32(), 
                pack[3].AsString(), 
                pack[4].AsInt32() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, const string &devName, int chNum, 
            const Utils::HostInf &host)
        {
			SendPack p(proto);
			p.WriteString( CName() );

			p.WriteString(devName);
			p.WriteInt32(chNum);
			p.WriteString(host.Address());
			p.WriteInt32(host.Port());
        }

    };

    // ---------------------------------------------

    // resp is BfResponse
    struct PcCmdStopSend : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdStopSend"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdStopSend(
                pack[1].AsString(), 
                pack[2].AsInt32(), 
                pack[3].AsString(), 
                pack[4].AsInt32() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, const string &devName, int chNum, 
            const std::string &ip, int port)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chNum);
            p.WriteString(ip);
            p.WriteInt32(port);
        }

        static void Send(ISafeBiProtoForSendPack &proto, const string &devName, int chNum, 
            const Utils::HostInf &host)
        {
            Send(proto, devName, chNum, host.Address(), host.Port());
        }

    };
    
    // ---------------------------------------------

    // resp for PcCmdStartRecv, PcCmdStopRecv, PcCmdStartSend, PcCmdStopSend
    // or sended as error indication for any command
    struct BfResponse : public ICmdInterface<IPcSideRecv>
    {
        static string CName() { return "BfResponse"; }
        string Name() const { return CName(); }

		static bool ProcessPacketStatic(const SbpRecvPack &pack, int &err, std::string &desc)
		{
			if (pack[0].AsString() != CName()) return false;

			err = pack[1].AsInt32();
			desc = pack[2].AsString();
			return true;
		}

        void ProcessPacket(const SbpRecvPack &pack, IPcSideRecv &callback) const
        {
            callback.OnBfResponse(pack[1].AsInt32(), pack[2].AsString());
        }

        static void Send(ISafeBiProtoForSendPack &proto, int errCode, const string &desc)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteInt32(errCode);
            p.WriteString(desc);
        }

		static std::string ResponseToString(int err, const std::string &desc)
		{
			return std::string(TdmMng::TdmErrorInfo::ResolveErrorCode(TdmMng::TdmErrorCode(err))
				+ std::string(" ") + desc);
		}
    };

    // --------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------


    // resp is BfResponse
    struct PcCmdAbSetCofidecSetup : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdAbSetCofidecSetup"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            AbCofidecParams params(pack, 3);
            callback.OnPcCmdAbSetCofidecSetup(pack[1].AsString(), pack[2].AsInt32(), params);
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, int chMask, const AbCofidecParams &params)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chMask);
            
            params.ToPacket(p);
        }

    };

    // ---------------------------------------------------

    // resp is BfResponse
    struct PcCmdAbSetLineParams : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdAbSetLineParams"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            AbLineParams params(pack, 3);
            callback.OnPcCmdAbSetLineParams(pack[1].AsString(), pack[2].AsInt32(), params);
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, int chMask, const AbLineParams &params)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chMask);

            params.ToPacket(p);
        }

    };

    // ---------------------------------------------------

    // resp is BfResponse
    struct PcCmdAbCommand : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdAbCommand"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdAbCommand(pack[1].AsString(), pack[2].AsInt32(), 
                pack[3].AsString(), pack[4].AsString());
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, int chMask, const string &cmdName, const string &cmdParams = "")
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chMask);
            p.WriteString(cmdName);
            p.WriteString(cmdParams);
        }

        UTILS_CLASS_STR(CStartRing,             "StartRing"); // (int ringMs, int silenceMs)
        UTILS_CLASS_STR(CStopRing,              "StopRing");  // ()

        UTILS_CLASS_STR(CStartTest,             "StartTest");  // ()
        UTILS_CLASS_STR(CStopTest,              "StopTest");   // ()

        UTILS_CLASS_STR(CSwitchToTestLineOn,    "SwitchToTestLineOn");   // ()
        UTILS_CLASS_STR(CSwitchToTestLineOff,   "SwitchToTestLineOff");  // ()

    };

    // ---------------------------------------------------

    // OnPcCmdPultCofidec(const string &devName, const iPult::CofidecState &state

    // resp is BfResponse
    struct PcCmdPultCofidec : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdPultCofidec"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            iPult::CofidecState state;
            SBProto::PacketRead s(pack, 2);
            iPult::CofidecState::Serialize(state, s);

            callback.OnPcCmdPultCofidec(pack[1].AsString(), state);
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, const iPult::CofidecState &state)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);

            {
                SBProto::PacketWrite s(p);
                iPult::CofidecState::Serialize(state, s);
            }
        }

    };

    // ---------------------------------------------------

    // resp is BfResponse
    // events send as BfTdmEvent::CFreqRecv
    struct PcCmdSetFreqRecvMode : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdSetFreqRecvMode"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdSetFreqRecvMode( pack[1].AsString(), pack[2].AsInt32(),
                pack[3].AsBool(), pack[4].AsString(), 
                (FreqRecvMode)pack[5].AsByte(), pack[6].AsString() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, int chMask, bool useInternal,
            const string &recvName, FreqRecvMode mode, const string &params = "")
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chMask);
            p.WriteBool(useInternal);
            p.WriteString(recvName);
            p.WriteByte(mode);
            p.WriteString(params);
        }

        // known recievers
        UTILS_CLASS_STR(CRecvDTMF, "RecvDTMF");  // no params

    };


    // ---------------------------------------------------

    // no response
    struct BfAbEvent : public ICmdInterface<IPcSideEventRecv>
    {
	public:
        static string CName() { return "BfAbEvent"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideEventRecv &callback) const
        {
			callback.OnBfAbEvent(pack[1].AsString(), pack[2].AsInt32(), 
				pack[3].AsString(), pack[4].AsString());
        }

        static void Send(ISafeBiProtoForSendPack &proto,
            const string &deviceName, int chNumber, 
            const string &eventName, const string &eventParams)
        {
            SendPack p(proto);
            p.WriteString( CName() );          
            p.WriteString(deviceName);
            p.WriteInt32(chNumber);
            p.WriteString(eventName);
            p.WriteString(eventParams);
        }

        UTILS_CLASS_STR(CBoardState,            "BoardState");          // (bool connected)
        UTILS_CLASS_STR(CLineCondition,         "LineCondition");       // (bool damaged)
        UTILS_CLASS_STR(CLineState,             "LineState");           // (bool busy)
        UTILS_CLASS_STR(CLinePulseDial,         "LinePulseDial");       // (int digit)
        UTILS_CLASS_STR(CLinePulseDialBegin,    "LinePulseDialBegin");  // (int digitCount)
        UTILS_CLASS_STR(CLineReset,             "LineReset");           // ()
    };

    // ---------------------------------------------------

    // no response
    struct BfTdmEvent : public ICmdInterface<IPcSideEventRecv>
    {
        static string CName() { return "BfTdmEvent"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideEventRecv &callback) const
        {
            callback.OnBfTdmEvent(pack[1].AsString(), pack[2].AsString(), pack[3].AsString());
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &src, const string &event, const string &params)
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(src);
            p.WriteString(event);
            p.WriteString(params);
        }

    // Sources
    public:

        UTILS_CLASS_STR(SrcEcho,      "EchoClient");
        UTILS_CLASS_STR(SrcQueueTdm,  "QueueTdm");
        UTILS_CLASS_STR(SrcQueueEcho, "QueueEcho");

    // Events
    public:

        // src is device:channel
        UTILS_CLASS_STR(CForcedStopSend,        "ForcedStopSend"); // ()           

        // src is device:channel
        UTILS_CLASS_STR(CDataCaptureBlocked,    "DataCaptureBlocked"); // ()

        // src is device
        UTILS_CLASS_STR(CE1Activated,           "E1Activated"); // ()              

        // src is device
        UTILS_CLASS_STR(CE1Deactivated,         "E1Deactivated"); // ()

        // src is any
        UTILS_CLASS_STR(CTraceDebug,            "TraceDebug"); // (str)

        // src is device:channel; send from TdmChannel::PushFreqRecvEvent()
        UTILS_CLASS_STR(CFreqRecv,              "FreqRecv");  // + :recvName  // (str)

        // src is QueueTdm | QueueEcho
        UTILS_CLASS_STR(CQueueOverflow,         "QueueOverflow");  // ()

    // Events, src is Echo
    public:

        UTILS_CLASS_STR(CEchoBadResp,               "EchoBadResp");             // (str) 
        UTILS_CLASS_STR(CEchoUnexpectedStateResp,   "EchoUnexpectedStateResp"); // ()    
        UTILS_CLASS_STR(CEchoUnexpectedPacket,      "EchoUnexpectedPacket");    // ()    
        UTILS_CLASS_STR(CEchoProtoError,            "EchoProtoError");          // (str) 
        UTILS_CLASS_STR(CEchoRawErrorText,          "EchoRawErrorText");        // (str) 
        UTILS_CLASS_STR(CEchoProtoDisc,             "EchoProtoDisc");           // (str) 
        UTILS_CLASS_STR(CEchoInitRespTimeout,       "EchoInitRespTimeout");     // ()    
        UTILS_CLASS_STR(CEchoStatsRespTimeout,      "EchoStatsRespTimeout");    // ()    
        UTILS_CLASS_STR(CEchoOverloaded,            "EchoOverloaded");          // ()    
        UTILS_CLASS_STR(CEchoBlocked,               "EchoBlocked");             // ()    
        UTILS_CLASS_STR(CEchoSbpException,          "CEchoSbpException");       // (str)    
        UTILS_CLASS_STR(CEchoHwError,               "EchoHwError");             // ()    
        UTILS_CLASS_STR(CEchoHwBadMode,             "EchoHwBadMode");           // ()    

    // misc
    public:

        UTILS_CLASS_STR(EventSrcSep, ":");

        static std::string MakeEventSource(const std::string &streamName, int chNum)
        {
            return streamName + EventSrcSep() + Utils::IntToString(chNum);
        }

        static TdmMng::TdmAsyncEvent MakeFreqRecvEvent(const std::string &eventSrc, 
            const std::string &recvName, const std::string &event)
        {
            std::string eventName = BfTdmEvent::CFreqRecv();
            eventName += BfTdmEvent::EventSrcSep() + recvName;

            return TdmMng::TdmAsyncEvent(eventSrc, eventName, event);                   
        }

        static TdmMng::TdmAsyncEvent MakeFreqRecvEvent(const std::string &streamName, int chNum,
            const std::string &recvName, const std::string &event)
        {
            return MakeFreqRecvEvent( MakeEventSource(streamName, chNum), recvName, event );
        }
		static bool ParseSrc(const std::string &src, std::string &streamName, int &channel)
		{
			size_t sep = src.find(iCmp::BfTdmEvent::EventSrcSep());
			if(sep == std::string::npos) return false;
			streamName = src.substr(0, sep);
			return Utils::StringToInt(src.substr(sep + 1), channel);
		}
		static void ParseEvent(const std::string &par, std::string &event, std::string &recvName)
		{
			size_t sep = par.find(iCmp::BfTdmEvent::EventSrcSep());
			event = par.substr(0, sep);
			if(sep != std::string::npos) recvName = par.substr(sep + 1);
		}


    };

    // --------------------------------------------------------------------------------------

    // resp is BfResponse
    struct PcCmdChCaptureMode : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdChCaptureMode"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdChCaptureMode(
                pack[1].AsString(), pack[2].AsInt32(), pack[3].AsBool());  
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const string &devName, int chNum, bool enabled) 
        {
            SendPack p(proto);
            p.WriteString( CName() );

            p.WriteString(devName);
            p.WriteInt32(chNum);
            p.WriteBool(enabled);
        }

    };

    // --------------------------------------------------------------------------------------

    // async, no response
    struct BfChannelData : public ICmdInterface<IPcSideEventRecv>
    {
        static string CName() { return "BfChannelData"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideEventRecv &callback) const
        {
            std::vector<byte> rx, tx, rxFixed;
            pack[3].AsBinary(rx);
            pack[4].AsBinary(tx);
            pack[5].AsBinary(rxFixed);

            callback.OnBfGetChannelData(pack[1].AsString(), pack[2].AsInt32(), 
                rx, tx, rxFixed);
        }

        static void Send(ISafeBiProtoForSendPack &proto, const string &devName, int chNum,
            const std::vector<Platform::byte> &rx,
            const std::vector<Platform::byte> &tx,
            const std::vector<Platform::byte> &rxFixed)            
        {
            SendPack p(proto);

            p.WriteString( CName() );       // 0
            p.WriteString(devName);         // 1
            p.WriteInt32(chNum);            // 2
            p.WriteBinary(rx);              // 3
            p.WriteBinary(tx);              // 4
            p.WriteBinary(rxFixed);         // 5
        }

    };

    // -----------------------------------------------------------------------

    // resp is BfResponse
    struct PcCmdConfCreate : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfCreate"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfCreate( pack[1].AsInt32(), pack[2].AsBool(), pack[3].AsInt32() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            int confHandle, bool autoMode, int blockSize)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(confHandle);
            p.WriteBool(autoMode);
            p.WriteInt32(blockSize);
        }

    };

    // --------------------------------------

    // resp is BfResponse
    struct PcCmdConfDelete : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfDelete"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfDelete(pack[1].AsInt32());
        }

        static void Send(ISafeBiProtoForSendPack &proto, int confHandle)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(confHandle);
        }

    };

    // --------------------------------------

    // resp is BfResponse
    struct PcCmdConfMode : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfMode"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfMode(pack[1].AsInt32(), pack[2].AsString());
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            int confHandle, const std::string &mode)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(confHandle);
            p.WriteString(mode);
        }

    };

    // --------------------------------------

    // resp is BfResponse
    struct PcCmdConfCreateTdmPoint : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfCreateTdmPoint"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfCreateTdmPoint( pack[1].AsInt32(), pack[2].AsString(), pack[3].AsInt32() );
        }

        static void Send(ISafeBiProtoForSendPack &proto,
            int pointHandle, const std::string &devName, int chNum)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(pointHandle);
            p.WriteString(devName);
            p.WriteInt32(chNum);
        }

    };

    // --------------------------------------

    // resp is BfRespConf<int>, return udpPort
    struct PcCmdConfCreateRtpPoint : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfCreateRtpPoint"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfCreateRtpPoint(pack[1].AsInt32(), pack[2].AsString(), pack[3].AsString());
        }

        static void Send(ISafeBiProtoForSendPack &proto,
            int pointHandle, const string &codecName, const string &codecParams)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(pointHandle);
            p.WriteString(codecName);
            p.WriteString(codecParams);
        }

    };
    
    // --------------------------------------

    // resp is BfResponse
    struct PcCmdConfCreateGenPoint : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfCreateGenPoint"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            int pointHandle = pack[1].AsInt32();

            iDSP::Gen2xProfile data;
            {
                SBProto::PacketRead s(pack, 2);
                data.Serialize(data, s);
            }

            callback.OnPcCmdConfCreateGenPoint(pointHandle, data);
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            int pointHandle, const iDSP::Gen2xProfile &data)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(pointHandle);

            {
                SBProto::PacketWrite s(p);
                data.Serialize(data, s);
            }

        }

    };


    // --------------------------------------

    // resp is BfResponse
    struct PcCmdConfDeletePoint : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfDeletePoint"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfDeletePoint(pack[1].AsInt32());
        }

        static void Send(ISafeBiProtoForSendPack &proto, int pointHandle)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(pointHandle);
        }

    };

    // --------------------------------------

    // resp is BfResponse
    struct PcCmdConfRtpPointSend : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfRtpPointSend"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfRtpPointSend(pack[1].AsInt32(), 
                pack[2].AsString(), pack[3].AsInt32());
        }

        static void Send(ISafeBiProtoForSendPack &proto, int pointHandle, const string &ip, int port)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(pointHandle);
            p.WriteString(ip);
            p.WriteInt32(port);
        }

    };

    // --------------------------------------

    // resp is BfResponse
    struct PcCmdConfAddPointToConf : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfAddPointToConf"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfAddPointToConf(pack[1].AsInt32(), pack[2].AsInt32(),
                pack[3].AsBool(), pack[4].AsBool());
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            int pointHandle, int confHandle, bool send, bool recv)
        {
            ESS_ASSERT(send || recv);

            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(pointHandle);
            p.WriteInt32(confHandle);
            p.WriteBool(send);
            p.WriteBool(recv);
        }

    };

    // ---------------------------------------

    // resp is BfResponse
    struct PcCmdConfRemovePointFromConf : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfRemovePointFromConf"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfRemovePointFromConf( pack[1].AsInt32() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, int pointHandle)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(pointHandle);
        }

    };

    // --------------------------------------

    // resp is BfRespConf<string>
    struct PcCmdConfGetState : public ICmdInterface<IBfSideRecv>
    {
        static string CName() { return "PcCmdConfGetState"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBfSideRecv &callback) const
        {
            callback.OnPcCmdConfGetState( pack[1].AsInt32() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, int confHandle)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteInt32(confHandle);
        }

    };



    // --------------------------------------

    // resp for PcCmdConfCreate, PcCmdConfAddTdmPoint, PcCmdConfAddRtpPoint, PcCmdConfGetState
    struct BfRespConf : public ICmdInterface<IPcSideRecv>
    {
        static string CName() { return "BfRespConf"; }
        string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IPcSideRecv &callback) const
        {
            callback.OnBfRespConf( pack[1].AsString() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, int param0)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteString( ChMngProtoParams::ToParams(param0) );
        }

        static void Send(ISafeBiProtoForSendPack &proto, int param0, int param1)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteString( ChMngProtoParams::ToParams(param0, param1) );
        }

        static void Send(ISafeBiProtoForSendPack &proto, const std::string &param0)
        {
            SendPack p(proto);
            p.WriteString( CName() );
            p.WriteString( ChMngProtoParams::ToParams(param0) );
        }


    };

    // -----------------------------------------------------------------------
    
    class PcRecv : public SBProto::RecvProcess<IPcSideRecv>
    {

    public:

        PcRecv(IPcSideRecv &callback) : SBProto::RecvProcess<IPcSideRecv>(callback) 
        {
            Add( BfRespCmdInit() );
            Add( BfRespGetRtpPort() );
            Add( BfResponse() );
            Add( BfRespGetTdmInfo() );
            Add( BfRespGetChannelInfo() );
            Add( BfRespStateInfo() );
//            Add( BfTdmEvent() );
//            Add( BfChannelData() );
            Add( BfRespConf() );

            // E1
//            Add( BfHdlcRecv() );
            Add( BfRespGetLiuInfo() );

            // AOZ
//            Add( BfAbEvent() );
        }
    };

	// -----------------------------------------------------------------------

	class PcEventRecv : public SBProto::RecvProcess<IPcSideEventRecv>
	{
	public:

		PcEventRecv(IPcSideEventRecv &callback) : SBProto::RecvProcess<IPcSideEventRecv>(callback) 
		{
			Add( BfTdmEvent() );
            Add( BfChannelData() );
			// E1
			Add( BfHdlcRecv() );

			// AOZ
			Add( BfAbEvent() );
		}
	};

    // -----------------------------------------------

    class BfRecv : public SBProto::RecvProcess<IBfSideRecv>
    {

    public:

        BfRecv(IBfSideRecv &callback) : SBProto::RecvProcess<IBfSideRecv>(callback) 
        {
            Add( PcCmdStartGen() );
            Add( PcCmdStopGen() );
            Add( PcCmdChEchoSuppressMode() );
            Add( PcCmdChCaptureMode() );
            Add( PcCmdGetRtpPort() );
            Add( PcCmdStartRecv() );
            Add( PcCmdStopRecv() );
            Add( PcCmdStartSend() );
            Add( PcCmdStopSend() );
            Add( PcCmdGetTdmInfo() );
            Add( PcCmdGetChannelInfo() );
            Add( PcCmdSetFreqRecvMode() );

            // conf
            Add( PcCmdConfCreate() );
            Add( PcCmdConfDelete() );
            Add( PcCmdConfMode() );
            Add( PcCmdConfCreateTdmPoint() );
            Add( PcCmdConfCreateRtpPoint() );
            Add( PcCmdConfCreateGenPoint() );
            Add( PcCmdConfDeletePoint() );
            Add( PcCmdConfRtpPointSend() );
            Add( PcCmdConfAddPointToConf() );
            Add( PcCmdConfRemovePointFromConf() );
            Add( PcCmdConfGetState() );

            // E1
            Add( PcCmdGetLiuInfo() );
            Add( PcHdlcSend() );

            // AOZ
            Add( PcCmdAbSetCofidecSetup() );
            Add( PcCmdAbSetLineParams() );
            Add( PcCmdAbCommand() );

            // Pult
            Add( PcCmdPultCofidec() );
        }

    };


	
}  // namespace iCmp



