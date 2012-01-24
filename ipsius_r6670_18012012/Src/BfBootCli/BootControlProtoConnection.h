#ifndef BOOTCONTROLPROTOCONNECTION_H
#define BOOTCONTROLPROTOCONNECTION_H

#include "SafeBiProtoExt/ISbpConnection.h"
#include "BfBootCore/BootControlProto.h"
#include "iLog/LogWrapper.h"
#include "iCore/MsgTimer.h"
#include "IProtoConnectionToOwner.h"
#include "SafeBiProto/ISbpTransport.h"
#include "BfBootCore/NamedScript.h"

namespace BfBootCli
{
    using boost::scoped_ptr;
    using boost::shared_ptr;
    using Utils::SafeRef;
    using Platform::dword;
    using Platform::byte;    
    
    //using SBProtoExt::ISbpConnection;

    using iLogW::EndRecord;

    /*    
        Задачи 
        - имплеменгтирует интерфейс удаленного сервера
        - выполняет трансляцию и отправку команд по протоколу
        - принимает и транслирует данные с SbpConnection в вызовы методов владельца
    */     

    // обертка поверх ISbpConnection поддерживающая протокол BootControlProto с клиентской стороны
    class BootControlProtoConnection    
        : //virtual public Utils::SafeRefServer,
        public iCore::MsgObject,
        public SBProtoExt::ISbpConnectionEvents,
        public BfBootCore::IClientToBooter
    {        
        IProtoConnectionToOwner& m_owner;
        scoped_ptr<iLogW::LogSession> m_logSession;
        iLogW::LogRecordTag m_tagSbpEv;
        iLogW::LogRecordTag m_tagEvSend;
        iLogW::LogRecordTag m_tagErr;
        scoped_ptr<SBProtoExt::ISbpConnection> m_connection;  
        scoped_ptr<BfBootCore::ClientRspConv> m_protoConverter;

        iCore::MsgTimer m_prcTimer;

        void OnProcess(iCore::MsgTimer*);

        void ProcessProtocolErr(const std::string& errInfo);

        void OnActivateTransport();

        void PackReceived( shared_ptr<SBProto::SbpRecvPack> data);

    // ISbpConnectionEvents
    private:
        void CommandReceived(SafeRef<SBProtoExt::ISbpConnection> src,  shared_ptr<SBProto::SbpRecvPack> data);
        void ResponseReceived(SafeRef<SBProtoExt::ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data);
        void InfoReceived(SafeRef<SBProtoExt::ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data);
        void ProtocolError(SafeRef<SBProtoExt::ISbpConnection> src, boost::shared_ptr<SBProto::SbpError> err);
        void ConnectionActivated(SafeRef<SBProtoExt::ISbpConnection> src);        
        void ConnectionDeactivated(SafeRef<SBProtoExt::ISbpConnection> src, const std::string & errInfo);

    // IClientToBooter
    private:
        void SetTrace(bool on);
        void SetUserParam(const std::string& name, const  std::string& value);
        void SetFactory(dword hwNumber, dword hwType, const std::string& mac);
        void GetParamList() const;
        void GetAppList() const;
        void CloseUpdateTransaction(dword softRelNum);
        //void AddScript(const BfBootCore::NamedScript&);
        void OpenWriteImg(dword size);        
        void WriteImgChunk(const void* pData, dword size);
        void CloseNewImg(const std::string& name, dword CRC);
        void DeleteLastImg();
        void DeleteAllImg();
        void SaveChanges();
        void OpenWriteBootImg(dword size);
        void WriteBooterChunk(const void* pData, dword length);
        void CloseNewBootImg(dword CRC);
        void EraseDefaultCfg();
        void DoLogout(bool withSave);
        void RunAditionalScript(const std::string& scriptName, bool withSave);
        void DoLogin(const std::string& pwd, dword protoVersion);
        void GetScriptList() const;
        

    public:

        BootControlProtoConnection
            (
            iCore::MsgThread& thread, 
            IProtoConnectionToOwner& owner,
            boost::shared_ptr<SBProto::ISbpTransport> transport,                        
            Utils::SafeRef<iLogW::ILogSessionCreator> logCreator
            );

        
        void ResetProtoObserver(BfBootCore::IBootServerToClient& observer)
        {            
            ESS_ASSERT(m_connection != 0);
            if (m_protoConverter == 0) 
            {
                //PutMsg(this, &BootControlProtoConnection::OnActivateTransport);
                m_connection->ActivateConnection();
            }

            m_protoConverter.reset(new BfBootCore::ClientRspConv(observer));            
        }

        void DestroyConnection();
    };
} // namespace BfBootCli

#endif
