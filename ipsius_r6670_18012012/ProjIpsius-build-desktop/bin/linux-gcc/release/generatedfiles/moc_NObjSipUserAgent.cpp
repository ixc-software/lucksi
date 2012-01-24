/****************************************************************************
** Meta object code from reading C++ file 'NObjSipUserAgent.h'
**
** Created: Tue Jan 24 14:58:22 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/NObjSipUserAgent.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSipUserAgent.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__NObjSipUserAgent[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       4,   54, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      58,   24,   23,   23, 0x02,
     180,  149,   23,   23, 0x22,
     256,  237,   23,   23, 0x22,
     314,  305,   23,   23, 0x22,
     387,  359,   23,   23, 0x02,
     444,  427,   23,   23, 0x22,
     481,  476,   23,   23, 0x02,
     516,  508,   23,   23, 0x02,

 // properties: name, type, flags
     557,  552, 0x01095003,
     591,  565, 0x0009500b,
     612,  604, 0x0a095001,
     630,  626, 0x02095003,

       0        // eod
};

static const char qt_meta_stringdata_iSip__NObjSipUserAgent[] = {
    "iSip::NObjSipUserAgent\0\0"
    "protocol,localPort,ipInterface,ip\0"
    "SetupTransport(iSip::SipTransportInfo::Type,int,QString,iSip::SipTrans"
    "portInfo::IpVersion)\0"
    "protocol,localPort,ipInterface\0"
    "SetupTransport(iSip::SipTransportInfo::Type,int,QString)\0"
    "protocol,localPort\0"
    "SetupTransport(iSip::SipTransportInfo::Type,int)\0"
    "protocol\0SetupTransport(iSip::SipTransportInfo::Type)\0"
    "name,registerUri,m_password\0"
    "AddAccessPoint(QString,QString,QString)\0"
    "name,registerUri\0AddAccessPoint(QString,QString)\0"
    "name\0DeleteAccessPoint(QString)\0pOutput\0"
    "DumProcessingStat(DRI::ICmdOutput*)\0"
    "bool\0Enabled\0iSip::SipUtils::PrintMode\0"
    "ResipLogMode\0QString\0TransportInfo\0"
    "int\0ForceShutdownTimeoutMs\0"
};

#ifdef Q_NO_DATA_RELOCATION
static const QMetaObjectAccessor qt_meta_extradata_iSip__NObjSipUserAgent[] = {
        iSip::SipUtils::getStaticMetaObject,
#else
static const QMetaObject *qt_meta_extradata_iSip__NObjSipUserAgent[] = {
        &iSip::SipUtils::staticMetaObject,
#endif //Q_NO_DATA_RELOCATION
    0
};

static const QMetaObjectExtraData qt_meta_extradata2_iSip__NObjSipUserAgent = {
    qt_meta_extradata_iSip__NObjSipUserAgent, 0 
};

const QMetaObject iSip::NObjSipUserAgent::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iSip__NObjSipUserAgent,
      qt_meta_data_iSip__NObjSipUserAgent, &qt_meta_extradata2_iSip__NObjSipUserAgent }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::NObjSipUserAgent::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::NObjSipUserAgent::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::NObjSipUserAgent::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__NObjSipUserAgent))
        return static_cast<void*>(const_cast< NObjSipUserAgent*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iSip::NObjSipUserAgent::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SetupTransport((*reinterpret_cast< iSip::SipTransportInfo::Type(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< iSip::SipTransportInfo::IpVersion(*)>(_a[4]))); break;
        case 1: SetupTransport((*reinterpret_cast< iSip::SipTransportInfo::Type(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 2: SetupTransport((*reinterpret_cast< iSip::SipTransportInfo::Type(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: SetupTransport((*reinterpret_cast< iSip::SipTransportInfo::Type(*)>(_a[1]))); break;
        case 4: AddAccessPoint((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 5: AddAccessPoint((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 6: DeleteAccessPoint((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: DumProcessingStat((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 8;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_isEnabled; break;
        case 1: *reinterpret_cast< iSip::SipUtils::PrintMode*>(_v) = m_resipLogMode; break;
        case 2: *reinterpret_cast< QString*>(_v) = TransportInfo(); break;
        case 3: *reinterpret_cast< int*>(_v) = ForceShutdownTimeoutMs(); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: Enable(*reinterpret_cast< bool*>(_v)); break;
        case 1: m_resipLogMode = *reinterpret_cast< iSip::SipUtils::PrintMode*>(_v); break;
        case 3: ForceShutdownTimeoutMs(*reinterpret_cast< int*>(_v)); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<iSip::NObjSipUserAgent> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iSip::NObjSipUserAgent> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
