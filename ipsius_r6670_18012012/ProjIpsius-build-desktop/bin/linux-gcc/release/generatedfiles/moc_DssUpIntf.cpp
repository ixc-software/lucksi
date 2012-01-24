/****************************************************************************
** Meta object code from reading C++ file 'DssUpIntf.h'
**
** Created: Tue Jan 24 14:57:38 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/ISDN/DssUpIntf.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DssUpIntf.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ISDN__ForceMoc_DssUpIntf[] = {

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

static const char qt_meta_stringdata_ISDN__ForceMoc_DssUpIntf[] = {
    "ISDN::ForceMoc_DssUpIntf\0"
};

const QMetaObject ISDN::ForceMoc_DssUpIntf::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ISDN__ForceMoc_DssUpIntf,
      qt_meta_data_ISDN__ForceMoc_DssUpIntf, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ISDN::ForceMoc_DssUpIntf::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ISDN::ForceMoc_DssUpIntf::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ISDN::ForceMoc_DssUpIntf::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ISDN__ForceMoc_DssUpIntf))
        return static_cast<void*>(const_cast< ForceMoc_DssUpIntf*>(this));
    return QObject::qt_metacast(_clname);
}

int ISDN::ForceMoc_DssUpIntf::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}

// --------------------------------------------------- 
// Interface code 


namespace ISDN__IDssCall_New
{
    using boost::shared_ptr;
    using namespace ISDN;

    // CRC string '0xISDN::IDssCall/void HoldReq()/void HoldConf()/void RetriveReq()/void RetriveConf()/void AddressComplete()/void AddressIncomplete()/void SetLink(BinderToUserCall linkBinderToUserCall)/void Disconnent(boost::shared_ptr<const DssCause> pCause)/void Alerting()/void ConnectRsp()UL'

    typedef  ISDN::IDssCall TIntf;

    class LocalIntfProxy : 
        public iCore::MsgObject,
        public TIntf
    {
        typedef LocalIntfProxy T;

        ObjLink::ILocalProxyHost &m_host;
        TIntf *m_pI;

        //--------------

        void HoldReq()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::HoldReq) );
        }

        //--------------

        void HoldConf()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::HoldConf) );
        }

        //--------------

        void RetriveReq()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::RetriveReq) );
        }

        //--------------

        void RetriveConf()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::RetriveConf) );
        }

        //--------------

        void AddressComplete()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::AddressComplete) );
        }

        //--------------

        void AddressIncomplete()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::AddressIncomplete) );
        }

        //--------------

        void SetLink(BinderToUserCall linkBinderToUserCall)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::SetLink, linkBinderToUserCall) );
        }

        //--------------

        void Disconnent(boost::shared_ptr<const DssCause> pCause)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Disconnent, pCause) );
        }

        //--------------

        void Alerting()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Alerting) );
        }

        //--------------

        void ConnectRsp()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ConnectRsp) );
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

    ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy> RM(Utils::CRC32(0x36395ba9UL));

}; // End of namespace

namespace ISDN__IDssCallEvents_New
{
    using boost::shared_ptr;
    using namespace ISDN;

    // CRC string '0xISDN::IDssCallEvents/void HoldInd()/void RetriveInd()/void Alerting()/void ConnectConfirm()/void Connected()/void DssCallCreated(DssCallCreatedParametrs params)/void DssCallErrorMsg(boost::shared_ptr<const CallWarning> pWarning)/void MoreInfo(Info info)/void ProgressInd(IeConstants::ProgressDscr descr)/void Disconnent(boost::shared_ptr<const DssCause> pCause)UL'

    typedef  ISDN::IDssCallEvents TIntf;

    class LocalIntfProxy : 
        public iCore::MsgObject,
        public TIntf
    {
        typedef LocalIntfProxy T;

        ObjLink::ILocalProxyHost &m_host;
        TIntf *m_pI;

        //--------------

        void HoldInd()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::HoldInd) );
        }

        //--------------

        void RetriveInd()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::RetriveInd) );
        }

        //--------------

        void Alerting()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Alerting) );
        }

        //--------------

        void ConnectConfirm()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ConnectConfirm) );
        }

        //--------------

        void Connected()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Connected) );
        }

        //--------------

        void DssCallCreated(DssCallCreatedParametrs params)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DssCallCreated, params) );
        }

        //--------------

        void DssCallErrorMsg(boost::shared_ptr<const CallWarning> pWarning)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DssCallErrorMsg, pWarning) );
        }

        //--------------

        void MoreInfo(Info info)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::MoreInfo, info) );
        }

        //--------------

        void ProgressInd(IeConstants::ProgressDscr descr)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::ProgressInd, descr) );
        }

        //--------------

        void Disconnent(boost::shared_ptr<const DssCause> pCause)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Disconnent, pCause) );
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

    ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy> RM(Utils::CRC32(0x289ed212UL));

}; // End of namespace

namespace ISDN__ICallbackDss_New
{
    using boost::shared_ptr;
    using namespace ISDN;

    // CRC string '0xISDN::ICallbackDss/void Activated()/void Deactivated()/void IncomingCall(IncommingCallParametrs params)/void DssErrorMsg(shared_ptr<const DssWarning> pWarning)/void RestartComplete(CompleteInfo info)UL'

    typedef  ISDN::ICallbackDss TIntf;

    class LocalIntfProxy : 
        public iCore::MsgObject,
        public TIntf
    {
        typedef LocalIntfProxy T;

        ObjLink::ILocalProxyHost &m_host;
        TIntf *m_pI;

        //--------------

        void Activated()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Activated) );
        }

        //--------------

        void Deactivated()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Deactivated) );
        }

        //--------------

        void IncomingCall(IncommingCallParametrs params)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::IncomingCall, params) );
        }

        //--------------

        void DssErrorMsg(shared_ptr<const DssWarning> pWarning)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::DssErrorMsg, pWarning) );
        }

        //--------------

        void RestartComplete(CompleteInfo info)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::RestartComplete, info) );
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

    ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy> RM(Utils::CRC32(0xe34a38dbUL));

}; // End of namespace

namespace ISDN__ILayerDss_New
{
    using boost::shared_ptr;
    using namespace ISDN;

    // CRC string '0xISDN::ILayerDss/void Activate()/void Deactivate()/void CreateOutCall(CreateOutCallParametrs params)/void RestartReq()UL'

    typedef  ISDN::ILayerDss TIntf;

    class LocalIntfProxy : 
        public iCore::MsgObject,
        public TIntf
    {
        typedef LocalIntfProxy T;

        ObjLink::ILocalProxyHost &m_host;
        TIntf *m_pI;

        //--------------

        void Activate()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Activate) );
        }

        //--------------

        void Deactivate()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Deactivate) );
        }

        //--------------

        void CreateOutCall(CreateOutCallParametrs params)
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::CreateOutCall, params) );
        }

        //--------------

        void RestartReq()
        {
            if(!m_host.ConnectedForProxy()) return;
            m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::RestartReq) );
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

    ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy> RM(Utils::CRC32(0xe53879d3UL));

}; // End of namespace

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<ISDN::ForceMoc_DssUpIntf> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
