#ifndef __SFXPROTO__
#define __SFXPROTO__

/*

    Sfx Protocol (SfxProto)
    
    SfxProto used between BfMain and BfEcho.
    It works over SBP/UART.

    Don't forget to change CSfxProtoVersion after modify this header!


*/

#include "Utils/IBasicInterface.h"
#include "Utils/UtilsClassStr.h"

#include "SafeBiProto/SbpRecvPack.h"
#include "SafeBiProto/SbpSendPack.h"
#include "SafeBiProto/CmdReceiveEngine.h"

#include "SfxProtoResp.h"
#include "echoappstats.h"

namespace EchoApp
{
    enum
    {
        CSfxProtoVersion = 7,
    };
}

#include "SfxProtoData.h"

namespace EchoApp
{
    using std::string;
    using SBProto::SbpRecvPack;
    using SBProto::ISafeBiProtoForSendPack;

    enum
    {
        CSfxProtoUartSpeed = 38400,
    };

    // ----------------------------------------------------------

    class IEchoRecvSide : public Utils::IBasicInterface
    {
    public:
        virtual void OnCmdInit(const CmdInitData &data) = 0;
        virtual void OnCmdEcho(int chNum, int taps) = 0;
        virtual void OnCmdSetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, bool enable) = 0;
        virtual void OnCmdRequestState() = 0;
        virtual void OnCmdRequestHalt() = 0;
        virtual void OnCmdShutdown() = 0;
    };

    // ----------------------------------------------------------

    class IMainRecvSide : public Utils::IBasicInterface
    {
    public:
        virtual void OnEchoResponse(const EchoResp &resp) = 0;
        virtual void OnEchoStateResp( const EchoAppStats &stats) = 0;
        virtual void OnEchoAsyncEvent(const std::string &eventName, 
            int chNum, const std::string &params) = 0;
    };

    // ----------------------------------------------------------

    // resp is EchoResponse
    struct MainCmdInit
    {
        static string CName() { return "MainCmdInit"; }

        static bool ProcessPacket(const SbpRecvPack &pack, IEchoRecvSide &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            CmdInitData data(pack);
            callback.OnCmdInit(data);
            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, const CmdInitData &data)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            data.ToPacket(p);
        }
    };

    // ----------------------------------------------------------

    // no response
    struct MainCmdEcho
    {
        static string CName() { return "MainCmdEcho"; }

        static bool ProcessPacket(const SbpRecvPack &pack, IEchoRecvSide &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            callback.OnCmdEcho(pack[1].AsInt32(), pack[2].AsInt32());

            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, int chNum, int taps)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            p.WriteInt32(chNum);
            p.WriteInt32(taps);
        }
    };

    // ----------------------------------------------------------

    // no response
    struct MainCmdSetFreqRecvMode
    {
        static string CName() { return "MainCmdSetFreqRecvMode"; }

        static bool ProcessPacket(const SbpRecvPack &pack, IEchoRecvSide &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            callback.OnCmdSetFreqRecvMode(pack[1].AsInt32(),  pack[2].AsString(),
                                          pack[3].AsString(), pack[4].AsBool());

            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            int chNum, const std::string &recvName, const std::string &params, bool enable)
        {
            SendPack p(proto);

            p.WriteString( CName() );
            p.WriteInt32(chNum);
            p.WriteString(recvName);
            p.WriteString(params);
            p.WriteBool(enable);            
        }
    };

    // ----------------------------------------------------------

    // no response
    struct MainCmdShutdown
    {
        static string CName() { return "MainCmdShutdown"; }

        static bool ProcessPacket(const SbpRecvPack &pack, IEchoRecvSide &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            callback.OnCmdShutdown();

            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendPack p(proto);
            p.WriteString( CName() );
        }
    };

    // ----------------------------------------------------------

    // resp is EchoStateResp
    struct MainCmdRequestState
    {
        static string CName() { return "MainCmdRequestState"; }

        static bool ProcessPacket(const SbpRecvPack &pack, IEchoRecvSide &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            callback.OnCmdRequestState();

            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendPack p(proto);
            p.WriteString( CName() );
        }

    };

    // ----------------------------------------------------------

    struct MainCmdRequestHalt
    {
        static string CName() { return "MainCmdRequestHalt"; }

        static bool ProcessPacket(const SbpRecvPack &pack, IEchoRecvSide &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            callback.OnCmdRequestHalt();

            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendPack p(proto);
            p.WriteString( CName() );
        }
    };

    // ----------------------------------------------------------

    // resp for MainCmdRequestState
    struct EchoStateResp
    {

        static string CName() { return "EchoStateResp"; }

        static bool ProcessPacket(const SbpRecvPack &pack, IMainRecvSide &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            SBProto::PacketRead s(pack, 1);
            EchoAppStats stats;
            EchoAppStats::Serialize(stats, s);

            callback.OnEchoStateResp(stats);
            
            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, const EchoAppStats &stats)
        {
            SendPack p(proto);

            p.WriteString( CName() );

            SBProto::PacketWrite s(p);
            EchoAppStats::Serialize(stats, s);
        }

    };

    // ----------------------------------------------------------
    
    struct EchoResponse
    {

        static string CName() { return "EchoResponse"; }

        static bool ProcessPacket(const SbpRecvPack &pack, IMainRecvSide &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            EchoResp resp((EchoResp::Code)pack[1].AsInt32(), pack[2].AsString());
            callback.OnEchoResponse(resp);

            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            EchoResp::Code code, const string &text = "")
        {
            SendPack p(proto);

            p.WriteString( CName() );
            p.WriteInt32(code);
            p.WriteString(text);
        }

    };

    // ----------------------------------------------------------

    struct EchoAsyncEvent
    {

        static string CName() { return "EchoAsyncEvent"; }

        static bool ProcessPacket(const SbpRecvPack &pack, IMainRecvSide &callback) 
        {   
            if (pack[0].AsString() != CName()) return false;

            callback.OnEchoAsyncEvent( pack[1].AsString(), pack[2].AsInt32(), pack[3].AsString() );

            return true;
        }

        static void Send(ISafeBiProtoForSendPack &proto, 
            const std::string &eventName, int chNum, const std::string &params)
        {
            SendPack p(proto);

            p.WriteString( CName() );

            p.WriteString(eventName);
            p.WriteInt32(chNum);
            p.WriteString(params);
        }

        UTILS_CLASS_STR(CSep, ":");

        UTILS_CLASS_STR(CFreqRecvEvent, "FreqRecv");

        // for MainCmdSetFreqMode:
        // eventName = CFreqRecvEvent + CSep + recvName
        // chNum     = channel number
        // params    = freq detected

    };

            
}  // namespace EchoApp

#endif
