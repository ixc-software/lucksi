#include "stdafx.h"
#include "ProxyTest_Interface.h"

/*
    Такой файл должен генерироваться автоматически!

*/

// --------------------------------------------

namespace
{

    // интерфейс, для которого сгенерирован код
    typedef CallModel::ITestInterface InterfaceType;  

    // генерированный класс локального прокси для ITestInterface
    class LocalProxy 
        : public Domain::ProxyIntfLocal<CallModel::ITestInterface>, 
          public InterfaceType
    {
    public:

        // InterfaceType implementation
        void Ping()  
        {
            if (!CheckConnected()) return;

            getInterface()->Ping();
        }

        void SetName(boost::shared_ptr<QString> name)
        {
            if (!CheckConnected()) return;

            getInterface()->SetName(name);
        }
        
        void SetNameVal(QString name)
        {
            getInterface()->SetNameVal(name);
        }


    };

    // ------------------------------------------------------------

    // идентификаторы методов
    enum MethodID
    {
        ID_Ping,
        ID_SetName,
        ID_SetNameVal,
    };

    // CRC методов
    enum MethodCRC
    {
        CRC_Ping        = 0x5670EE15,
        CRC_SetName     = 0x406700FF,
        CRC_SetNameVal  = 0x45110050,
    };

    // ------------------------------------------------------------

    // генерированный класс удаленного прокси
    class RemoteProxy 
        : public Domain::ProxyIntfRemote,
          public InterfaceType
    {
    public:

        // InterfaceType implementation
        void Ping()  
        {
            if (!CheckConnected()) return;

            Domain::RemoteCall rc(getRemoteHandle(), ID_Ping, CRC_Ping);
        }

        void SetName(boost::shared_ptr<QString> name)
        {
            if (!CheckConnected()) return;

            Domain::RemoteCall rc(getRemoteHandle(), ID_SetName, CRC_SetName);
            rc.io() << (*name);
        }

        void SetNameVal(QString name)
        {
            if (!CheckConnected()) return;

            Domain::RemoteCall rc(getRemoteHandle(), ID_SetName, CRC_SetName);
            rc.io() << name;
        }

    };

    // ------------------------------------------------------------

    // удаленный прокси, серверная часть
    class RemoteProxyHandle : public Domain::ProxyRemoteServerT<InterfaceType>
    {
    public:

        RemoteProxyHandle(Domain::IProxyServer *pSrv)
            : Domain::ProxyRemoteServerT<InterfaceType>(pSrv)
        {
            // ...
        }

        void ProcessCall(Domain::RecvRemoteCall &call)  // override
        {
            Domain::RpcMethodID id = call.getRpcMethodID();
            Domain::RpcMethodCRC crc = call.getRpcMethodCRC();

            // Ping()
            if (id == ID_Ping)
            {
                if (crc != CRC_Ping) 
                {
                    OnMethodBadCRC("Ping", ID_Ping, CRC_Ping, crc);
                    return;
                }

                // param
                // ...nope

                // call
                getInterface()->Ping();
                return;
            }

            // SetName(boost::shared_ptr<QString> name)
            if (id == ID_SetName)
            {
                if (crc != CRC_SetName) 
                {
                    OnMethodBadCRC("SetName", ID_SetName, CRC_SetName, crc);
                    return;
                }

                // param
                boost::shared_ptr<QString> param(new QString());
                call.io() >> (*param);

                // call
                getInterface()->SetName(param);
                return;
            }

            // SetNameVal(QString name)
            if (id == ID_SetNameVal)
            {
                if (crc != CRC_SetNameVal) 
                {
                    OnMethodBadCRC("SetNameVal", ID_SetNameVal, CRC_SetNameVal, crc);
                    return;
                }

                // param
                QString param;
                call.io() >> param;

                // call
                getInterface()->SetNameVal(param);
                return;
            }

            // Bad method
            OnBadMethodID(id);
        }

    };

    // ----------------------------------------------------------------

    Domain::RegisterMeta<InterfaceType, LocalProxy, RemoteProxy, RemoteProxyHandle> RM(0);
}


