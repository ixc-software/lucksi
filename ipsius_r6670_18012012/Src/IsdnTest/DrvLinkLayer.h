#ifndef DRVLINKLAYER_H
#define DRVLINKLAYER_H

#include "stdafx.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"

// #include "IsdnTestConfig.h"
#include "DriverL1Test.h"

namespace IsdnTest
{ 

    class DrvLinkLayer
        : public iCore::MsgObject, 
        public IDrvLinkLayer
    {
    public:

        DrvLinkLayer(DriverL1Test* pReciver, iCore::MsgThread& thread)
            : MsgObject(thread),
            m_pReciver(pReciver), 
            m_pOpposite(0), 
            m_DelayMode(false),
            m_t(this, &DrvLinkLayer::OnTimer)
        {}

        ~DrvLinkLayer()
        {
            ESS_ASSERT( m_cashPack.empty() );
        }

        void SetOpposite(DrvLinkLayer* pOpposite) {m_pOpposite = pOpposite;}

        void SetDelayMode(int delayMsec)
        {
            ESS_ASSERT(!m_DelayMode);
            if ( delayMsec == 0 )
                return;
            m_DelayMode = true;
            m_t.Start(delayMsec, true);
        }

        // ќчередь пакетов к этомумоменту должна быть пуста
        void UnSetDelayMode()
        {
            ESS_ASSERT( m_cashPack.empty() );
            m_t.Stop();
            m_DelayMode = false;
        }

        void SetNewDrv(DriverL1Test* pReciver)
        {
            ESS_ASSERT(pReciver);
            m_pReciver = pReciver;
        }

    // IDrvLinkLayer Impl
    private:

        void SendData(QVector<byte> packet)  {m_pOpposite->SendToDrv(packet);} // override IDrvLinkLayer

        void Activate() {m_pOpposite->ActivateOwn();} //override

        void Deactivate() {m_pOpposite->DeactivateOwn();} // override

    private:

        void ActivateOwn(){m_pReciver->ActFromLL();}

        void DeactivateOwn(){m_pReciver->DeactFromLL();}


        void SendToDrv(QVector<byte> packet) 
        {
            if (m_DelayMode)            
                m_cashPack.push(packet);                
            else
                m_pReciver->ReciveData(packet);            
        }

        void OnTimer(iCore::MsgTimer*)
        {
            ESS_ASSERT(m_DelayMode);
            if (m_cashPack.empty()) return;

            m_pReciver->ReciveData( m_cashPack.back()/*->Clone()*/ );
            m_cashPack.pop();
        }

        DriverL1Test* m_pReciver;
        DrvLinkLayer *m_pOpposite;
        bool m_DelayMode;
        iCore::MsgTimer m_t;
        std::queue<QVector<byte> > m_cashPack;        

    };
        
};

#endif


//class Task
//    : public iCore::MsgObject
//{
//    std::list<Task*>& m_taskList;
//    DrvShell* m_pReciver;        
//    ISDN::IsdnPacket* m_pPack;
//    iCore::MsgTimer m_t;
//
//    void OnTimer(iCore::MsgTimer*)
//    {            
//        m_pReciver->ReciveData( m_pPack );
//        std::list<Task*>::iterator i;
//        i = std::find(m_taskList.begin(), m_taskList.end(), this);
//        ESS_ASSERT( i != m_taskList.end() );
//        m_taskList.erase(i);
//        AsyncDeleteSelf();
//    }
//
//
//public:
//
//    Task(iCore::MsgThread& thread, DrvShell* pReciver, ISDN::IsdnPacket* pPack, int delay, std::list<Task*>& taskList)
//        : MsgObject(thread),
//        m_pReciver(pReciver),
//        m_taskList(taskList),
//        m_pPack(pPack),
//        m_t(this, &Task::OnTimer)
//    {
//        m_t.Start(delay);
//    }
//
//
//};
//
//class DrvLinkLayer
//    : public IDrvLinkLayer
//{
//public:
//
//    DrvLinkLayer(DrvShell* pReciver, iCore::MsgThread& thread)
//        : m_thread(thread),
//        m_pReciver(pReciver), 
//        m_pOpposite(0), 
//        m_delayMsec(0)            
//    {}
//
//    ~DrvLinkLayer()
//    {
//        ESS_ASSERT( m_taskList.empty() );
//    }
//
//    void SetOpposite(DrvLinkLayer* pOpposite) {m_pOpposite = pOpposite;}
//
//    void SetDelayMode(int delayMsec)
//    {
//        ESS_ASSERT(m_delayMsec == 0);
//        m_delayMsec = delayMsec;
//    }
//
//    void UnSetDelayMode()
//    {
//        ESS_ASSERT( m_taskList.empty() );
//        m_delayMsec = 0;
//    }
//
//    void SendData(ISDN::IsdnPacket *pPacket)  {m_pOpposite->SendToDrv(pPacket);} // override IDrvLinkLayer
//
//    void Activate() {m_pOpposite->ActivateOwn();} //override
//
//private:
//
//    void ActivateOwn(){m_pReciver->ActFromLL();}
//    void SendToDrv(ISDN::IsdnPacket *pPacket) 
//    {
//        if (m_delayMsec != 0)            
//            m_taskList.push_back( new Task( m_thread, m_pReciver, pPacket, m_delayMsec, m_taskList) );
//        else
//            m_pReciver->ReciveData(pPacket);            
//    }        
//
//    iCore::MsgThread& m_thread;
//    DrvShell* m_pReciver;
//    DrvLinkLayer *m_pOpposite;
//    int m_delayMsec;       
//
//    std::list<Task*> m_taskList;       
//
//};
