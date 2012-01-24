#include "stdafx.h"
#include "NObjBoardSettings.h"

namespace 
{
    template<class T>
    int ReadIntProp(const Utils::InitProp<T>& prop)
    {
        return prop.IsInited() ? prop : -1;
    }
} // namespace 

namespace BfEmul
{
    Q_INVOKABLE void NObjBoardSettings::SetDefaultParams( int hwType, int hwNum, QString mac )
    {
        AssertUnlocked();
        m_defaultParams.reset(
            new BfBootCore::DefaultParam(hwType, hwNum, mac.toStdString())
            );
    }

    Q_INVOKABLE void NObjBoardSettings::SetUdpLogAddr( QString val, int port )
    {
        AssertUnlocked();
        m_userParams.UdpLogHost = val.toStdString();
        m_userParams.UdpLogPort = port;
    }

    Q_INVOKABLE void NObjBoardSettings::SetNetwork( bool use, QString ip, QString gateway, QString mask )
    {
        AssertUnlocked();
        if (use)
        {
            m_userParams.Network = E1App::BoardAddresSettings();
            return;
        }               

        m_userParams.Network = 
            E1App::BoardAddresSettings(ip.toStdString(), gateway.toStdString(), mask.toStdString());

        if (!m_userParams.Network.get().IsValid()) ThrowRuntimeException("Invalid network settings");
    }

    // ------------------------------------------------------------------------------------
    // Owner interface

    void NObjBoardSettings::LockChanges( bool lock )
    {
        if (m_locked == lock) return;
        m_locked = lock;        

        if (!lock) return PropertyWriteEventClear();
        else       return PropertyWriteEventAbort("Change settings locked.");
    }

    void NObjBoardSettings::AssertUnlocked()
    {
        if (m_locked) ThrowRuntimeException("Change settings locked.");
    }

    BfBootCore::DefaultParam& NObjBoardSettings::getParams()
    {
        if (!m_defaultParams) ThrowRuntimeException("SetDefaultParams first.");

        m_defaultParams->OptionalSettings = m_userParams;
        return *m_defaultParams;
    }

    bool NObjBoardSettings::Validate() const
    {
        return m_defaultParams;
    }

    // ------------------------------------------------------------------------------------
    // for dri property

    QString NObjBoardSettings::getLogAddr()
    {
        if (!m_userParams.UdpLogHost.IsInited() || !m_userParams.UdpLogPort.IsInited()) return "";
        return QString("%1:%2")
            .arg(m_userParams.UdpLogHost.get().c_str())
            .arg(m_userParams.UdpLogPort.get());
    }

    void NObjBoardSettings::setUseTimestamp( bool use )
    {
        m_userParams.UseTimestamp = use;
    }

    void NObjBoardSettings::setCountToStore( int count )
    {
        m_userParams.CountToStore = count;
    }

    int NObjBoardSettings::getCountToStore() const
    {
        return ReadIntProp(m_userParams.CountToStore);
    }

    QString NObjBoardSettings::getNetwork() const
    {
        if (!m_userParams.Network.IsInited()) return "";
        return QString("DHCP: %1 Ip: %2 Gateway: %3 Mask: %4")
            .arg(m_userParams.Network.get().DHCP())
            .arg(m_userParams.Network.get().IP().c_str())
            .arg(m_userParams.Network.get().Gateway().c_str())
            .arg(m_userParams.Network.get().Mask().c_str());
    }

    QString NObjBoardSettings::getUserPwd()
    {
        return m_userParams.UserPwd.IsInited() ? m_userParams.UserPwd.get().c_str() : "";
    }

    void NObjBoardSettings::setUserPwd( QString pwd /*= ""*/ )
    {
        m_userParams.UserPwd = pwd.toStdString();
    }

    void NObjBoardSettings::setWaitLoginMsec( int msec )
    {
        m_userParams.WaitLoginMsec = msec;
    }

    int NObjBoardSettings::getWaitLoginMsec() const
    {
        return ReadIntProp(m_userParams.WaitLoginMsec);
    }

    void NObjBoardSettings::setCmp( int port )
    {
        m_userParams.CmpPort = port;
    }

    int NObjBoardSettings::getCmp() const
    {
        return ReadIntProp(m_userParams.CmpPort);
    }

    bool NObjBoardSettings::getUseTimestamp() const
    {
        return m_userParams.UseTimestamp.IsInited() ? m_userParams.UseTimestamp : false;
    }

    
} // namespace BfEmul

