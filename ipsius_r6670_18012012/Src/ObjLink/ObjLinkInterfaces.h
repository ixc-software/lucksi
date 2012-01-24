#ifndef __OBJECTLINKINTERFACES__
#define __OBJECTLINKINTERFACES__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "Domain/IDomain.h"
#include "iCore/MsgThread.h"
#include "ObjectLinkError.h"

namespace ObjLink
{
    using Utils::SafeRef;
    using boost::shared_ptr;

    // маркер интерфейсов, который будут использоваться вместе с ObjectLink
    class IObjectLinkInterface : public Utils::IBasicInterface
    {
    };

    // ---------------------------------------------------

    // промежуточный (прокси) интерфейс для выхода на IObjectLinkOwner
    class IObjectLinkOwnerNotify : public virtual Domain::IDomain
    {
    public:
        virtual void OnObjectLinkConnect() = 0;
        virtual void OnObjectLinkDisconnect() = 0;
        virtual void OnObjectLinkError(boost::shared_ptr<ObjectLinkError> error) = 0;
    };

    // события для владельца ObjectLink
    class IObjectLinkOwner : public virtual Domain::IDomain
    {
    public:
        virtual void OnObjectLinkConnect(ILinkKeyID &linkID) = 0;
        virtual void OnObjectLinkDisconnect(ILinkKeyID &linkID) = 0;
        virtual void OnObjectLinkError(boost::shared_ptr<ObjectLinkError> error) = 0;
    };

    // ---------------------------------------------------

    // события сервер -> ObjectLink
    class IServerToObjectLink : public Utils::IBasicInterface
    {
    public:
        virtual void ObjectLinkConnectConf() = 0;
        virtual void ObjectLinkDisconnect() = 0;
    };

    // интерфейс сервера
    class IObjectLinksServer : public Utils::IBasicInterface
    {
    public:
        virtual void ObjectLinkConnectReq(SafeRef<IServerToObjectLink> &link) = 0;
        virtual void ObjectLinkDisconnect(SafeRef<IServerToObjectLink> &link) = 0;
        virtual void ObjectLinkSendMsg(SafeRef<IServerToObjectLink> &link, iCore::MsgBase *pMsg) = 0;
    };

    // сервер (объект для подключения ObjectLink)
    class IObjectLinksHost : public Utils::IBasicInterface
    {
    public:
        virtual SafeRef<IObjectLinksServer> getObjectLinksServer() = 0;
    };

    // ---------------------------------------------------

    /* Интерфейс, по которому сгенерированный для обслуживания локального интерфейса код,
     общается с ObjectLinkLocal 

     Отказаться от метода ConnectedForProxy не получиться, т.к. создание 
     сообщения и его отправка через SendMsgToDestinationObject не срабатывает,
     если сервер был удален. Причина -- dynamic_cast, используемый при создании
     сообщения для выведения типа MsgObjectBase.
     */
    class ILocalProxyHost : public Utils::IBasicInterface
    {
    public:
        virtual iCore::MsgThread& getMsgThreadForProxy() = 0;
        virtual Utils::IBasicInterface* getDestinationInterfaceForProxy() = 0;
        virtual bool ConnectedForProxy() = 0;  
        virtual void SendMsgToDestinationObject(iCore::MsgBase *pMsg) = 0;
    };

    template<class TIntf>
    TIntf* LocalProxyHostToInterface(ILocalProxyHost &host)
    {
        Utils::IBasicInterface *pI = host.getDestinationInterfaceForProxy();
        ESS_ASSERT(pI != 0);

        TIntf *pTyped = dynamic_cast<TIntf*>(pI);
        ESS_ASSERT(pTyped != 0);
        return pTyped;
    }

		
}  // namespace ObjLink

#endif



