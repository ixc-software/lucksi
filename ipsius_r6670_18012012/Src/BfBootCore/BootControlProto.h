#ifndef BOOTCONTROLPROTO_H
#define BOOTCONTROLPROTO_H

#include "SafeBiProto/SbpRecvPack.h"
#include "SafeBiProto/SafeBiProto.h"
#include "SafeBiProto/SbpSendPack.h"
#include "SafeBiProto/CmdReceiveEngine.h"
#include "SafeBiProto/SbpError.h"

#include "IClientToBooter.h"
#include "IBootServerToClient.h"
#include "UserResponseCode.h"
#include "StringToTypeConverter.h"

namespace BfBootCore
{   
    using SBProto::SbpRecvPack;   
    using SBProto::ISafeBiProtoForSendPack;
    
    typedef SBProto::SbpSendPackCmd SendCmdPack;       
    typedef SBProto::SbpSendPackResp SendRespPack;       
    typedef SBProto::SbpSendPackInfo SendInfoPack;      

    const int CProtoVersion = 0;
    
    const int CDefaultCbpPort = 1111;

    /*
    *   Server command (IClientToBooter) types description
    */    
    
    // команда регистрации клиента
    struct BooterCmdLogin : SBProto::ICmdInterface<ILogin>
    {
        
        // void DoLogin(const std::string& pwd, int protoVersion);
        
        static std::string CName() { return "BooterCmdLogin"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, ILogin &callback) const
        {
            callback.DoLogin(pack[1].AsString(), pack[2].AsDword());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const std::string &pwd, dword protoVersion)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteString(pwd);  
            p.WriteDword(protoVersion);
        }
    };

    // ------------------------------------------------------------------------------------
    // void SetTrace(bool on)

    // команда регистрации клиента
    struct BooterCmdSetTrace : SBProto::ICmdInterface<ILogin>
    {        
        static std::string CName() { return "BooterCmdSetTrace"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, ILogin &callback) const
        {
            callback.SetTrace(pack[1].AsBool());
        }

        static void Send(ISafeBiProtoForSendPack &proto, bool on) //on = true, off = false
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteBool(on);            
        }
    };

    // ------------------------------------------------------------------------------------

    struct BooterCmdCloseUpdateTransaction : SBProto::ICmdInterface<IBootCtrl>
    {   /*void CloseUpdateTransation(dword releaseNumber)*/
        static std::string CName() { return "CloseUpdateTransaction"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.CloseUpdateTransaction(pack[1].AsDword());
        }

        static void Send(ISafeBiProtoForSendPack &proto, dword releaseNumber)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteDword(releaseNumber);            
        }
    };

    // ------------------------------------------------------------------------------------

    struct BooterCmdSetUserParam : SBProto::ICmdInterface<IBootCtrl>
    {
        // void SetUserParam(const std::string& name, const std::string& value)

        static std::string CName() { return "BooterCmdSetUserParam"; }
        std::string Name() const { return CName(); }
        
        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.SetUserParam(pack[1].AsString(), pack[2].AsString());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const std::string& name, const std::string& value)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteString(name);
            p.WriteString(value);
        }
    };

    // ------------------------------------------------------------------------------------

    // void SetBoardId(const std::string& id)
    struct BooterCmdSetBoardId : SBProto::ICmdInterface<IBootCtrl>
    {
        
        static std::string CName() { return "BooterCmdSetBoardId"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.SetFactory( pack[1].AsDword(), pack[2].AsDword(), pack[3].AsString() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, dword hwNumber, dword hwType, const std::string& mac)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );                   

            p.WriteDword(hwNumber);
            p.WriteDword(hwType);
            p.WriteString(mac);
        }
    };

    // ------------------------------------------------------------------------------------

    // void GetParamList() const
    struct BooterCmdGetParamList : SBProto::ICmdInterface<IBootCtrl>
    {

        static std::string CName() { return "BooterCmdGetParamList"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.GetParamList();
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );                   
        }
    };

    // ------------------------------------------------------------------------------------

    // void GetAppList() const 
    struct BooterCmdGetAppList : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdGetAppList"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.GetAppList();
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );           
        }
    };

    // ------------------------------------------------------------------------------------

    // void OpenWriteImg(dword size)
    struct BooterCmdOpenWriteImg : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdOpenWriteImg"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.OpenWriteImg( pack[1].AsDword() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, dword totalSize)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );
            
            p.WriteDword(totalSize);            
        }
    };  

    // ------------------------------------------------------------------------------------    
    
    struct BooterCmdWriteImgChunk : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdWriteImgChunk"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {            
            callback.WriteImgChunk(pack[1].DataPtr(), pack[1].Length());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const void* data, dword length)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteBinary(data, length);
        }
    };

    // ------------------------------------------------------------------------------------

    // void CloseNewImg(const std::string& name, dword CRC)        
    struct BooterCmdCloseNewImg : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdCloseNewImg"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.CloseNewImg(pack[1].AsString(), pack[2].AsDword());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const std::string& name, dword CRC)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteString(name);
            p.WriteDword(CRC);
        }
    };

    // ------------------------------------------------------------------------------------

    // void DeleteLastImg()
    struct BooterCmdDeleteLastImg : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdDeleteLastImg"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.DeleteLastImg();
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );
        }
    };

    // ------------------------------------------------------------------------------------

    // void DeleteAllImg()
    struct BooterCmdDeleteAllImg : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdDeleteAllImg"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.DeleteAllImg();
        }

        static void Send(ISafeBiProtoForSendPack &proto )
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );
        }
    };

    // ------------------------------------------------------------------------------------

   

    // void SaveChanges()
    struct BooterCmdSaveChanges : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdSaveChanges"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.SaveChanges();
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );
        }
    };

    // ------------------------------------------------------------------------------------

    // void OpenWriteBootImg(dword size)
    struct BooterCmdOpenWriteBootImg : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdOpenWriteBootImg"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.OpenWriteBootImg(pack[1].AsDword());
        }

        static void Send(ISafeBiProtoForSendPack &proto, dword totalSize)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteDword(totalSize);            
        }
    };

    // ------------------------------------------------------------------------------------

    // void WriteBooterChunk(const std::vector<byte>& data)
    struct BooterCmdWriteBooterChunk : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdWriteBooterChunk"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.WriteBooterChunk(pack[1].DataPtr(), pack[1].Length());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const void* pData, dword length)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteBinary(pData,length);
        }
    };

    // ------------------------------------------------------------------------------------

    // void CloseNewBootImg(dword CRC) 
    struct BooterCmdCloseNewBootImg: SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdCloseNewBootImg"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.CloseNewBootImg( pack[1].AsDword() );
        }

        static void Send(ISafeBiProtoForSendPack &proto, dword CRC)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteDword(CRC);
        }
    };

    // ------------------------------------------------------------------------------------


    // void EraseDefaultCfg() 
    struct BooterCmdEraseDefaultCfg : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdEraseDefaultCfg"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.EraseDefaultCfg();
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );
        }
    };

    // ------------------------------------------------------------------------------------
    // void GetScriptList() const

    struct BooterCmdGetScriptList : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdGetScriptList"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            callback.GetScriptList();
        }

        static void Send(ISafeBiProtoForSendPack &proto)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );
        }
    };

    // ------------------------------------------------------------------------------------
    // void RunAditionalScript(const std::string& scriptName, bool withSaveChanges)

    struct BooterCmdRunAditionalScript : SBProto::ICmdInterface<ILogin>
    {
        static std::string CName() { return "BooterCmdRunAditionalScript"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, ILogin &callback) const
        {
            callback.RunAditionalScript(pack[1].AsString(), pack[2].AsBool());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const std::string& scriptName, bool withSave)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteString(scriptName);
            p.WriteBool(withSave);
        }
    };

    // ------------------------------------------------------------------------------------

    // void DoLogout(bool withSave)
    struct BooterCmdDoLogout : SBProto::ICmdInterface<ILogin>
    {
        static std::string CName() { return "BooterCmdDoLogout"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, ILogin &callback) const
        {
            callback.DoLogout(pack[1].AsBool());
        }

        static void Send(ISafeBiProtoForSendPack &proto, bool withSave)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );

            p.WriteBool(withSave);
        }
    };

    // ------------------------------------------------------------------------------------
    // void AddScript(const NamedScript&)

    /*struct BooterCmdAddScript : SBProto::ICmdInterface<IBootCtrl>
    {
        static std::string CName() { return "BooterCmdAddScript"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootCtrl &callback) const
        {
            NamedScript script;
            StringToTypeConverter::fromString( pack[1].AsString(), script);
            callback.AddScript(script);
        }

        static void Send(ISafeBiProtoForSendPack &proto, const NamedScript& script)
        {
            SendCmdPack p(proto);
            p.WriteString( CName() );            

            p.WriteString(StringToTypeConverter::toString(script));
        }
    };*/


    // ------------------------------------------------------------------------------------


    /*
    *   Client response (IBootServerToClient) types description
    */      

    // Оvoid RespLogin(bool loginComplete, word booterVersion, bool userMode) 
    struct ClientRespLogin : SBProto::ICmdInterface<IBootServerToClient>
    {
        static std::string CName() { return "ClientRespLogin"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootServerToClient &callback) const
        {            
            callback.RespLogin( UserResponseInfo( pack[1].AsInt32() ), pack[2].AsString(), pack[3].AsString(), pack[4].AsBool());
        }


        // todo Use response code
        static void Send(ISafeBiProtoForSendPack &proto, 
            const UserResponseInfo& info, const std::string& additionalInfo,
            std::string booterVersion, bool userMode
            )
        {
            SendRespPack p(proto);
            p.WriteString( CName() );

            p.WriteInt32(info.AsCode());
            p.WriteString(additionalInfo);
            p.WriteString(booterVersion);        
            p.WriteBool(userMode);                
        }

        // todo добавить перегрузку команды для wrongPwd, чтоб не показывать инфу о сервере
    };    

    // ------------------------------------------------------------------------------------

    // void RespCmd(UserResponseCode code) 
    struct ClientRespCmd : SBProto::ICmdInterface<IBootServerToClient>
    {
        static std::string CName() { return "ClientRespCmd"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootServerToClient &callback) const
        {
            callback.RespCmd(UserResponseInfo( pack[1].AsInt32() ));
        }

        static void Send(ISafeBiProtoForSendPack &proto, const UserResponseInfo& info)
        {
            SendRespPack p(proto);
            p.WriteString( CName() );

            p.WriteInt32(info.AsCode());
        }
    };

    // ------------------------------------------------------------------------------------

    // void RespGetParam(const std::vector<BfBootCore::ConfigLine>& userParams)
    struct ClientRespGetParam : SBProto::ICmdInterface<IBootServerToClient>
    {
        static std::string CName() { return "ClientRespGetParam"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootServerToClient &callback) const
        {
            std::vector<BfBootCore::ConfigLine> arg;
            StringToTypeConverter::fromString( pack[1].AsString(), arg );
            callback.RespGetParam(arg);
        }

        static void Send(ISafeBiProtoForSendPack &proto, const std::vector<BfBootCore::ConfigLine>& userParams)
        {
            SendRespPack p(proto);
            p.WriteString( CName() );

            std::string str = StringToTypeConverter::toString(userParams);
            // todo validate str.size() limit ?
            p.WriteString(str);            
        }
    };

    // ------------------------------------------------------------------------------------

    // void RespGetAppList(const std::vector<ImgDescriptor>& appList)
    struct ClientRespGetAppList : SBProto::ICmdInterface<IBootServerToClient>
    {
        static std::string CName() { return "ClientRespGetAppList"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootServerToClient &callback) const
        {
            ImgDescrList arg;
            StringToTypeConverter::fromString(pack[1].AsString(), arg);
            callback.RespGetAppList( arg );
        }

        static void Send(ISafeBiProtoForSendPack &proto, const ImgDescrList& appList)
        {
            SendRespPack p(proto);
            p.WriteString( CName() );

            std::string str = StringToTypeConverter::toString(appList);
            // todo validate str.size() limit ?
            p.WriteString(str);
        }
    };

    // ------------------------------------------------------------------------------------

    // void RespNewImgAdded(const ImgDescriptor&)
    struct ClientRespNewImgAdded : SBProto::ICmdInterface<IBootServerToClient>
    {
        static std::string CName() { return "ClientNewImgAdded"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootServerToClient &callback) const
        {
            ImgDescriptor descr;
            StringToTypeConverter::fromString(pack[1].AsString(), descr);
            callback.RespNewImgAdded(descr);
        }

        static void Send(ISafeBiProtoForSendPack &proto, const ImgDescriptor& descr)
        {
            SendRespPack p(proto);
            p.WriteString( CName() );

            p.WriteString(StringToTypeConverter::toString(descr));
        }
    };

    
    // ------------------------------------------------------------------------------------
    // void RespGetScriptList(const ScriptList&)
    struct ClientRespGetScriptList : SBProto::ICmdInterface<IBootServerToClient>
    {
        static std::string CName() { return "ClientRespGetScriptList"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootServerToClient &callback) const
        {
            ScriptList scripts;
            StringToTypeConverter::fromString(pack[1].AsString(), scripts);            
            callback.RespGetScriptList(scripts);
        }

        static void Send(ISafeBiProtoForSendPack &proto, const ScriptList& scripts)
        {
            SendRespPack p(proto);
            p.WriteString( CName() );

            p.WriteString(StringToTypeConverter::toString(scripts));
        }
    };

    // ------------------------------------------------------------------------------------
    // void StateInfo(const StateInfo& info, dword progress = -1)

    struct InfoState : SBProto::ICmdInterface<IBootServerToClient>
    {
        static std::string CName() { return "InfoState"; }
        std::string Name() const { return CName(); }

        void ProcessPacket(const SbpRecvPack &pack, IBootServerToClient &callback) const
        {                           
            callback.StateInfo(SrvStateInfo(pack[1].AsInt32()), pack[2].AsInt32());
        }

        static void Send(ISafeBiProtoForSendPack &proto, const SrvStateInfo& info, Platform::int32 progress = -1)
        {
            SendInfoPack p(proto);
            p.WriteString( CName() );

            p.WriteInt32(info.AsCode());
            p.WriteInt32(progress);            
        }
    };

    
    // ------------------------------------------------------------------------------------  
       
    /*
    *   Registration, converters.
    */         
       
    class BootCtrlConv : public SBProto::RecvProcess<IBootCtrl>
    {
    public:
        BootCtrlConv(IBootCtrl& intf) : SBProto::RecvProcess<IBootCtrl>(intf)
        {
            Add(BooterCmdSetUserParam());
            Add(BooterCmdSetBoardId());
            Add(BooterCmdGetParamList());
            Add(BooterCmdGetAppList());
            Add(BooterCmdOpenWriteImg());
            //Add(BooterCmdWriteImgChunk());
            Add(BooterCmdWriteImgChunk());
            Add(BooterCmdCloseNewImg());
            Add(BooterCmdCloseUpdateTransaction());

            Add(BooterCmdDeleteLastImg());
            Add(BooterCmdDeleteAllImg()); 
            Add(BooterCmdSaveChanges());

            Add(BooterCmdOpenWriteBootImg());
            Add(BooterCmdWriteBooterChunk());
            Add(BooterCmdCloseNewBootImg());
            Add(BooterCmdEraseDefaultCfg());

            Add(BooterCmdCloseUpdateTransaction());
            Add(BooterCmdGetScriptList());            
            //Add(BooterCmdAddScript());
        }
    };
    
    class ClientRspConv : public SBProto::RecvProcess<IBootServerToClient>
    {
    public:
        ClientRspConv(IBootServerToClient& intf) : SBProto::RecvProcess<IBootServerToClient>(intf)
        {
            Add( ClientRespLogin() );
            Add( ClientRespCmd() );
            Add( ClientRespGetParam() );
            Add( ClientRespNewImgAdded() );
            Add( ClientRespGetAppList() ); 
            Add( ClientRespGetScriptList() );
            Add( InfoState() );
        }
    };

    class LoginConv : public SBProto::RecvProcess<ILogin>
    {
    public:
        LoginConv(ILogin& intf) : SBProto::RecvProcess<ILogin>(intf)
        {
            Add(BooterCmdLogin()   );
            Add(BooterCmdDoLogout());
            Add(BooterCmdSetTrace());
            Add(BooterCmdRunAditionalScript());
        }
    };
   
        
} // namespace BfBootCore

#endif
