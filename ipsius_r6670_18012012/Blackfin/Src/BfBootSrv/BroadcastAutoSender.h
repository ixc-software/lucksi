#ifndef BROADCASTAUTOSENDER_H
#define BROADCASTAUTOSENDER_H


#include "BfBootCore/BroadcastMsg.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "Lw/UdpSocket.h"

namespace BfBootSrv
{
    class ICloseApp : Utils::IBasicInterface
    {
    public:
        virtual void AsyncReboot() = 0;
    };

    /*
        Получает через конструктор информацию для широковещательной рассылки.
        Рассылает эту информацию с заданным периодом.     
        Если в сообщении сказанно что оно отправленно не из загрузчика, то по тому-же сокету слушает команду перезагрузки и выполняет её.
    */   

    class BroadcastAutoSender : public iCore::MsgObject, boost::noncopyable
    {               
        ICloseApp*const m_pReboot;
        
        BfBootCore::BroadcastMsg m_msg;        
        
        Lw::Packet m_msgPacketBroadcast;
        Lw::Packet m_msgPacketDirectRequest;

        iCore::MsgTimer m_tSend;
        Lw::UdpSocket m_socket;
        Platform::word m_sendPeriod;
        
        Lw::Packet m_pack;                      // optimize
        std::vector<Platform::byte> m_recData;  // optimize

        void Process(iCore::MsgTimer*);
        void Send(bool direct = false);
        void Listen();
        void UpdateAllPacket();
        void UpdatePacket( const BfBootCore::BroadcastMsg& msg, Lw::Packet& pack );
        template<class TCmd>
        bool IsCmd(Lw::Packet& pack);
        void SetBroadcastDest();

    public:

        // do auto send broadcast message. If msg BroadcastMsg::IsBooter = false listen reload command. 
        BroadcastAutoSender(
            iCore::MsgThread& thread, BfBootCore::BroadcastMsg& msg,
            int bindPort, ICloseApp* pReset = 0
            );        

        void UpdateOwnerInfo(const std::string& info);

        // Start send, start listen
        void Start(int cmpPort);

        // Stop send, stop listen
        void Stop();
    };
} // namespace BfBootSrv

#endif
