#ifndef __MSGOBJECTBASE__
#define __MSGOBJECTBASE__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "ICheckObject.h"

namespace iCore
{
    class MsgBase;
    class ITimerManager;

    class IMsgObject : public Utils::IBasicInterface
    {
    public:
        virtual void PutMsgInQueue(MsgBase *pMsg) = 0;
    };

    /*
        ������� ����� ��� MsgObject
        ������� ������������� � ����� ������������ ������������, � ���� 
        ����� ������� ����������, ������������ � ������� MsgBase � ����������� ��������
    */
    class MsgObjectBase : 
        public ICheckObject, 
        public virtual IMsgObject,
        public virtual Utils::SafeRefServer
    {
    public:

        MsgObjectBase(ICheckGenerator &g) : ICheckObject(g)
        {
        }

        virtual ITimerManager& GetTimerManager() = 0;
        virtual bool getDestroyRequested() const = 0;
    };
	
} // namespace iCore

#endif
