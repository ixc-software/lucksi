#ifndef NOBJBROADCASTRECEIVER_H
#define NOBJBROADCASTRECEIVER_H

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "BfBootCli/BroadcastReceiver.h"
#include "BfBootCore/GeneralBooterConst.h"

#include "BfBootCli/BroadcastFilter.h"

namespace BfBootDRI
{
    // todo отделить property от объекта

    class IBroadcastReceiverReport;

    // DRI wrapper for BroadcastReceiver.
    // Main task: waiting broadcast message with assigned parameters
    //  all parameters are optional
    class NObjBroadcastReceiver 
        : public Domain::NamedObject,
        public BfBootCli::IBroadcastReceiverToOwner,
        public BfBootCli::BroadcastFilter
    {
        Q_OBJECT;
        typedef NObjBroadcastReceiver T;    

    // DRI interface:
    public:        
        // External
        NObjBroadcastReceiver(Domain::IDomain *pDomain, const Domain::ObjectName &name);

        Q_PROPERTY (int HwNumber WRITE m_hwNum READ getNum);
        Q_PROPERTY (int HwType WRITE m_hwType READ getType);
        Q_PROPERTY (int CbpPort WRITE m_cbpPort READ getCbp);
        Q_PROPERTY (int CmpPort WRITE m_cmpPort READ getCmp);
        Q_PROPERTY (QString Mac WRITE setMac READ getMac);
        Q_PROPERTY (int SoftNumber WRITE m_softNum READ getRel);
        Q_PROPERTY (QString SrcAddress WRITE m_srcAddress READ getAddr); // ignore com address

        Q_INVOKABLE void StartWaiting(DRI::IAsyncCmd *pAsyncCmd, bool resetAfter, int maxMsec = -1);              
        Q_INVOKABLE void ViewLastWaitingResult(DRI::IAsyncCmd *pAsyncCmd);        

    public:
        // Internal use
        NObjBroadcastReceiver(Domain::NamedObject *pParent, const Domain::ObjectName &name);
        void StartWaiting(IBroadcastReceiverReport& observer, int maxMsec);  
        
        int getNum() {return Read(m_hwNum);}
        int getType(){return Read(m_hwType);}
        int getCbp(){return Read(m_cbpPort);}
        int getCmp(){return Read(m_cmpPort);}
        QString getMac() {return Read(m_mac);}
        int getRel(){return Read(m_softNum);} 
        QString getAddr() {return Read(m_srcAddress);}        
        void setMac(QString mac);
        
        int Read(const Utils::InitProp<int>& prop);
        QString Read(const Utils::InitProp<QString>& prop);

    // IBroadcastReceiverToOwner impl:
    private:
        void MsgReceived(const BfBootCore::BroadcastMsg& msg);          

    private:
        void Run(int maxMsec);
        void WaitingComplete(const BfBootCore::BroadcastMsg& msg);
        void OnDeleteReceiver();
        void OnTimeout(iCore::MsgTimer* p);
        void Log(QString msg);
        void AbortAsync()
        {
            AsyncComplete(false, "Aborted by user");
        }

    // private fields
    private:        

        boost::scoped_ptr<BfBootCli::BroadcastReceiver> m_receiver;      
        bool m_active;
        bool m_resetAfter;

        iCore::MsgTimer m_waitingTimeout;

        IBroadcastReceiverReport* m_pRefObserver;
        
        boost::scoped_ptr<BfBootCore::BroadcastMsg> m_lastFilteredMsg;
    };

  
} // namespace BfBootDRI

#endif
