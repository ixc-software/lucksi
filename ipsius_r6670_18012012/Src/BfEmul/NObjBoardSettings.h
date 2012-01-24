#ifndef NOBJBOARDSETTINGS_H
#define NOBJBOARDSETTINGS_H

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "BfBootCore/ServerTuneParametrs.h"

namespace BfEmul
{
    // todo перенести в bfBootCli
    class NObjBoardSettings
        : public Domain::NamedObject,
        public DRI::INonCreatable
    {
        Q_OBJECT;

    // fields
    private:
        boost::scoped_ptr<BfBootCore::DefaultParam> m_defaultParams;
        BfBootCore::UserParams m_userParams;
        bool m_locked;

    // DRI interface
    public:
        NObjBoardSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(pParent, name, pParent), m_locked(false)
        {            
        }
        
        Q_PROPERTY(int HwType READ getType);        
        Q_PROPERTY(int HwNum READ getNum);        
        Q_PROPERTY(QString HwMac READ getMac);        
        Q_INVOKABLE void SetDefaultParams(int hwType, int hwNum, QString mac);
        
        Q_PROPERTY(QString UdpLogAddr READ getLogAddr);               
        Q_INVOKABLE void SetUdpLogAddr(QString val, int port);
        Q_PROPERTY(bool UseTimestamp READ getUseTimestamp WRITE setUseTimestamp);
        Q_PROPERTY(int CountToStore READ getCountToStore WRITE setCountToStore);              
        Q_PROPERTY(QString Network READ getNetwork);        
        Q_INVOKABLE void SetNetwork(bool use, QString ip, QString gateway, QString mask);
        Q_PROPERTY(QString UserPwd READ getUserPwd WRITE setUserPwd);
        Q_PROPERTY(int WaitLoginMsec READ getWaitLoginMsec WRITE setWaitLoginMsec); // прочитает -1, если не выставленно
        Q_PROPERTY(int CmpPort WRITE setCmp READ getCmp);                

    // Owner interface
    public:
        void LockChanges(bool lock);
        void AssertUnlocked();
        BfBootCore::DefaultParam& getParams();
        bool Validate() const;

    // for dri property
    private:        
        int getType() { return m_defaultParams ? m_defaultParams->HwType : -1; }
        int getNum() { return m_defaultParams ? m_defaultParams->HwNumber : -1; }
        QString getMac() { return m_defaultParams ? m_defaultParams->Mac.c_str() : ""; }
        QString getLogAddr(); 
        void setUseTimestamp(bool use);
        bool getUseTimestamp() const;
        void setCountToStore(int count);
        int getCountToStore() const;
        QString getNetwork() const;
        QString getUserPwd();
        void setUserPwd(QString pwd = "");
        void setWaitLoginMsec(int msec);
        int getWaitLoginMsec() const;
        void setCmp(int port);
        int getCmp() const;
    };
} // namespace BfEmul

#endif
