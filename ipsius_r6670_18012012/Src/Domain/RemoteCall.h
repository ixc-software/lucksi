#ifndef __REMOTECALL__

#define __REMOTECALL__

#include "stdafx.h"
#include "Utils/CRC32.h"
#include "RemoteCallIO.h"
#include "DomainRPC.h"

namespace Domain
{

    typedef int RpcMethodID;
    typedef Utils::TCRC RpcMethodCRC;

    /*
        RPC вызов для отравки, содержит двоичный поток RemoteCallIO, в который записывается
        вся необходимая информация о вызове, которая потом передается через DomainRPC
    */
    class RemoteCall
    {
        RemoteCallIO m_io;
        DomainRPC &m_DRCP;

    public:
        RemoteCall(ObjectRemoteHandle &remoteID, RpcMethodID methodID, RpcMethodCRC methodCRC)
            : m_DRCP(remoteID.GetDRCP())
        {
            m_io << remoteID.getID();
            m_io << methodID << methodCRC;
        }

        ~RemoteCall()
        {
            // send!
            m_DRCP.Send(m_io);
        }

        RemoteCallIO& io()
        {
            return m_io;
        }

    };

    // -------------------------------------------

    // Полученный RPC вызов
    class RecvRemoteCall
    {
        RemoteCallIO &m_io;

        ObjectDescriptor    m_desc;
        RpcMethodID         m_methodID; 
        RpcMethodCRC        m_methodCRC;

    public:
        RecvRemoteCall(RemoteCallIO &io) : m_io(io)
        {
            m_io >> m_desc >> m_methodID >> m_methodCRC;
        }

        RemoteCallIO& io()
        {
            return m_io;
        }

        ObjectDescriptor    getObjectDescriptor() const { return m_desc; }
        RpcMethodID         getRpcMethodID() const      { return m_methodID; }
        RpcMethodCRC        getRpcMethodCRC() const     { return m_methodCRC; }

    };

}  // namespace Domain


#endif
