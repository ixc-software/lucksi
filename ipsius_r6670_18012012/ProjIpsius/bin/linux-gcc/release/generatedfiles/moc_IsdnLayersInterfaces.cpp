/****************************************************************************
** Meta object code from reading C++ file 'IsdnLayersInterfaces.h'
**
** Created: Tue Jan 24 17:49:21 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/ISDN/IsdnLayersInterfaces.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IsdnLayersInterfaces.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ISDN__ForceMoc_IsdnLayersInterfaces[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_ISDN__ForceMoc_IsdnLayersInterfaces[] = {
    "ISDN::ForceMoc_IsdnLayersInterfaces\0"
};

const QMetaObject ISDN::ForceMoc_IsdnLayersInterfaces::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ISDN__ForceMoc_IsdnLayersInterfaces,
      qt_meta_data_ISDN__ForceMoc_IsdnLayersInterfaces, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ISDN::ForceMoc_IsdnLayersInterfaces::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ISDN::ForceMoc_IsdnLayersInterfaces::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ISDN::ForceMoc_IsdnLayersInterfaces::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ISDN__ForceMoc_IsdnLayersInterfaces))
        return static_cast<void*>(const_cast< ForceMoc_IsdnLayersInterfaces*>(this));
    return QObject::qt_metacast(_clname);
}

int ISDN::ForceMoc_IsdnLayersInterfaces::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}

// --------------------------------------------------- 
// Interface code 


namespace ISDN__IDriverToL2_New
{
    using boost::shared_ptr;
    using namespace ISDN;

    // CRC string '0xISDN::IDriverToL2/void DataInd(QVector<byte> packet)/void ActivateInd()/void DeactivateInd()/void ActivateConf()/void DeactivateConf()/void PullConf()/void PauseInd()/void PauseConf()UL'

    typedef  ISDN::IDriverToL2 TIntf;

    class LocalIntfProxy : 
        public iCore::MsgObject,
        public TIntf
    {
        typedef LocalIntfProxy T;

        ObjLink::ILocalProxyHost &m_host;
        TIntf *m_pI;

        //--------------

        void DataInd(QVector<byte> packet)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DataInd, packet) );
        }

        //--------------

        void ActivateInd()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ActivateInd) );
        }

        //--------------

        void DeactivateInd()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DeactivateInd) );
        }

        //--------------

        void ActivateConf()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ActivateConf) );
        }

        //--------------

        void DeactivateConf()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DeactivateConf) );
        }

        //--------------

        void PullConf()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::PullConf) );
        }

        //--------------

        void PauseInd()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::PauseInd) );
        }

        //--------------

        void PauseConf()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::PauseConf) );
        }

        //--------------

    public:

        LocalIntfProxy(ObjLink::ILocalProxyHost &host)
            : iCore::MsgObject(host.getMsgThreadForProxy()),
            m_host(host)
        {
            m_pI = ObjLink::LocalProxyHostToInterface<TIntf>(host);
        }
    
}; // End of local proxy

    ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy> RM(Utils::CRC32(0x1f323600UL));

}; // End of namespace

namespace ISDN__IL2ToDriver_New
{
    using boost::shared_ptr;
    using namespace ISDN;

    // CRC string '0xISDN::IL2ToDriver/void DataRequest(QVector<byte> packet)/void ActivateRequest()/void DeactivateRequest()/void PullRequest()/void PullInd(QVector<byte> packet)/void SetUpIntf(BinderToIDrvToL2 pIntfUp)UL'

    typedef  ISDN::IL2ToDriver TIntf;

    class LocalIntfProxy : 
        public iCore::MsgObject,
        public TIntf
    {
        typedef LocalIntfProxy T;

        ObjLink::ILocalProxyHost &m_host;
        TIntf *m_pI;

        //--------------

        void DataRequest(QVector<byte> packet)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DataRequest, packet) );
        }

        //--------------

        void ActivateRequest()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ActivateRequest) );
        }

        //--------------

        void DeactivateRequest()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DeactivateRequest) );
        }

        //--------------

        void PullRequest()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::PullRequest) );
        }

        //--------------

        void PullInd(QVector<byte> packet)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::PullInd, packet) );
        }

        //--------------

        void SetUpIntf(BinderToIDrvToL2 pIntfUp)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::SetUpIntf, pIntfUp) );
        }

        //--------------

    public:

        LocalIntfProxy(ObjLink::ILocalProxyHost &host)
            : iCore::MsgObject(host.getMsgThreadForProxy()),
            m_host(host)
        {
            m_pI = ObjLink::LocalProxyHostToInterface<TIntf>(host);
        }
    
}; // End of local proxy

    ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy> RM(Utils::CRC32(0xdf29052dUL));

}; // End of namespace

namespace ISDN__IL2ToL3_New
{
    using boost::shared_ptr;
    using namespace ISDN;

    // CRC string '0xISDN::IL2ToL3/void EstablishInd()/void EstablishConf()/void ReleaseConf()/void ReleaseInd()/void DataInd(QVector<byte> l2data)/void UDataInd(QVector<byte> l2data)/void FreeWinInd()/void SetDownIntf(BinderToIL3ToL2 binderToL2)/void ErrorInd(L2Error errorCode)UL'

    typedef  ISDN::IL2ToL3 TIntf;

    class LocalIntfProxy : 
        public iCore::MsgObject,
        public TIntf
    {
        typedef LocalIntfProxy T;

        ObjLink::ILocalProxyHost &m_host;
        TIntf *m_pI;

        //--------------

        void EstablishInd()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::EstablishInd) );
        }

        //--------------

        void EstablishConf()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::EstablishConf) );
        }

        //--------------

        void ReleaseConf()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ReleaseConf) );
        }

        //--------------

        void ReleaseInd()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ReleaseInd) );
        }

        //--------------

        void DataInd(QVector<byte> l2data)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DataInd, l2data) );
        }

        //--------------

        void UDataInd(QVector<byte> l2data)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::UDataInd, l2data) );
        }

        //--------------

        void FreeWinInd()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::FreeWinInd) );
        }

        //--------------

        void SetDownIntf(BinderToIL3ToL2 binderToL2)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::SetDownIntf, binderToL2) );
        }

        //--------------

        void ErrorInd(L2Error errorCode)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ErrorInd, errorCode) );
        }

        //--------------

    public:

        LocalIntfProxy(ObjLink::ILocalProxyHost &host)
            : iCore::MsgObject(host.getMsgThreadForProxy()),
            m_host(host)
        {
            m_pI = ObjLink::LocalProxyHostToInterface<TIntf>(host);
        }
    
}; // End of local proxy

    ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy> RM(Utils::CRC32(0xbad02e17UL));

}; // End of namespace

namespace ISDN__IL3ToL2_New
{
    using boost::shared_ptr;
    using namespace ISDN;

    // CRC string '0xISDN::IL3ToL2/void EstablishReq()/void ReleaseReq()/void DataReq(QVector<byte> l3data)/void UDataReq(QVector<byte> l3data)/void SetUpIntf(BinderToIL2ToL3 binderToL3)UL'

    typedef  ISDN::IL3ToL2 TIntf;

    class LocalIntfProxy : 
        public iCore::MsgObject,
        public TIntf
    {
        typedef LocalIntfProxy T;

        ObjLink::ILocalProxyHost &m_host;
        TIntf *m_pI;

        //--------------

        void EstablishReq()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::EstablishReq) );
        }

        //--------------

        void ReleaseReq()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ReleaseReq) );
        }

        //--------------

        void DataReq(QVector<byte> l3data)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DataReq, l3data) );
        }

        //--------------

        void UDataReq(QVector<byte> l3data)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::UDataReq, l3data) );
        }

        //--------------

        void SetUpIntf(BinderToIL2ToL3 binderToL3)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::SetUpIntf, binderToL3) );
        }

        //--------------

    public:

        LocalIntfProxy(ObjLink::ILocalProxyHost &host)
            : iCore::MsgObject(host.getMsgThreadForProxy()),
            m_host(host)
        {
            m_pI = ObjLink::LocalProxyHostToInterface<TIntf>(host);
        }
    
}; // End of local proxy

    ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy> RM(Utils::CRC32(0x58c1b4dcUL));

}; // End of namespace

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<ISDN::ForceMoc_IsdnLayersInterfaces> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
