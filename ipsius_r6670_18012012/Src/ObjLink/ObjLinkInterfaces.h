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

    // ������ �����������, ������� ����� �������������� ������ � ObjectLink
    class IObjectLinkInterface : public Utils::IBasicInterface
    {
    };

    // ---------------------------------------------------

    // ������������� (������) ��������� ��� ������ �� IObjectLinkOwner
    class IObjectLinkOwnerNotify : public virtual Domain::IDomain
    {
    public:
        virtual void OnObjectLinkConnect() = 0;
        virtual void OnObjectLinkDisconnect() = 0;
        virtual void OnObjectLinkError(boost::shared_ptr<ObjectLinkError> error) = 0;
    };

    // ������� ��� ��������� ObjectLink
    class IObjectLinkOwner : public virtual Domain::IDomain
    {
    public:
        virtual void OnObjectLinkConnect(ILinkKeyID &linkID) = 0;
        virtual void OnObjectLinkDisconnect(ILinkKeyID &linkID) = 0;
        virtual void OnObjectLinkError(boost::shared_ptr<ObjectLinkError> error) = 0;
    };

    // ---------------------------------------------------

    // ������� ������ -> ObjectLink
    class IServerToObjectLink : public Utils::IBasicInterface
    {
    public:
        virtual void ObjectLinkConnectConf() = 0;
        virtual void ObjectLinkDisconnect() = 0;
    };

    // ��������� �������
    class IObjectLinksServer : public Utils::IBasicInterface
    {
    public:
        virtual void ObjectLinkConnectReq(SafeRef<IServerToObjectLink> &link) = 0;
        virtual void ObjectLinkDisconnect(SafeRef<IServerToObjectLink> &link) = 0;
        virtual void ObjectLinkSendMsg(SafeRef<IServerToObjectLink> &link, iCore::MsgBase *pMsg) = 0;
    };

    // ������ (������ ��� ����������� ObjectLink)
    class IObjectLinksHost : public Utils::IBasicInterface
    {
    public:
        virtual SafeRef<IObjectLinksServer> getObjectLinksServer() = 0;
    };

    // ---------------------------------------------------

    /* ���������, �� �������� ��������������� ��� ������������ ���������� ���������� ���,
     �������� � ObjectLinkLocal 

     ���������� �� ������ ConnectedForProxy �� ����������, �.�. �������� 
     ��������� � ��� �������� ����� SendMsgToDestinationObject �� �����������,
     ���� ������ ��� ������. ������� -- dynamic_cast, ������������ ��� ��������
     ��������� ��� ��������� ���� MsgObjectBase.
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



