/****************************************************************************
** Meta object code from reading C++ file 'NObjSipAccessPoint.h'
**
** Created: Tue Jan 24 17:49:57 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/NObjSipAccessPoint.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSipAccessPoint.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__NObjSipAccessPoint[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
      14,   24, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      26,   25,   25,   25, 0x02,
      49,   25,   25,   25, 0x02,

 // properties: name, type, flags
      78,   73, 0x01095003,
     120,   86, 0x00095009,
     134,  126, 0x0a095003,
     146,  126, 0x0a095003,
     157,  126, 0x0a095003,
     166,   73, 0x01095001,
     189,   73, 0x01095003,
     212,  208, 0x02095003,
     232,  208, 0x02095003,
     257,   73, 0x01095003,
     290,  126, 0x0a095001,
     341,  315, 0x0009500b,
     356,  126, 0x0a095003,
     370,  208, 0x02095003,

       0        // eod
};

static const char qt_meta_stringdata_iSip__NObjSipAccessPoint[] = {
    "iSip::NObjSipAccessPoint\0\0"
    "SetAsDefaultReceiver()\0ClearCallsDestination()\0"
    "bool\0Enabled\0iSip::SipRegistrationState::Value\0"
    "State\0QString\0RegisterUri\0ExternalIp\0"
    "Password\0DefaultForIncomingCall\0"
    "EnableRegistration\0int\0RegistrationTimeSec\0"
    "RegistrationRetryTimeSec\0"
    "EnableOutCallWithoutRegistration\0"
    "IncomingCallsDestination\0"
    "iSip::ForwardingType::Val\0ForwardingType\0"
    "ForwardingUri\0NoAnswerTimeoutSec\0"
};

#ifdef Q_NO_DATA_RELOCATION
static const QMetaObjectAccessor qt_meta_extradata_iSip__NObjSipAccessPoint[] = {
        iSip::SipRegistrationState::getStaticMetaObject,
    iSip::ForwardingType::getStaticMetaObject,
#else
static const QMetaObject *qt_meta_extradata_iSip__NObjSipAccessPoint[] = {
        &iSip::SipRegistrationState::staticMetaObject,
    &iSip::ForwardingType::staticMetaObject,
#endif //Q_NO_DATA_RELOCATION
    0
};

static const QMetaObjectExtraData qt_meta_extradata2_iSip__NObjSipAccessPoint = {
    qt_meta_extradata_iSip__NObjSipAccessPoint, 0 
};

const QMetaObject iSip::NObjSipAccessPoint::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iSip__NObjSipAccessPoint,
      qt_meta_data_iSip__NObjSipAccessPoint, &qt_meta_extradata2_iSip__NObjSipAccessPoint }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::NObjSipAccessPoint::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::NObjSipAccessPoint::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::NObjSipAccessPoint::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__NObjSipAccessPoint))
        return static_cast<void*>(const_cast< NObjSipAccessPoint*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSipAccessPoint*>(this));
    if (!strcmp(_clname, "ISipAccessPointEventsExt"))
        return static_cast< ISipAccessPointEventsExt*>(const_cast< NObjSipAccessPoint*>(this));
    if (!strcmp(_clname, "ISipAccessPoint"))
        return static_cast< ISipAccessPoint*>(const_cast< NObjSipAccessPoint*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iSip::NObjSipAccessPoint::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SetAsDefaultReceiver(); break;
        case 1: ClearCallsDestination(); break;
        default: ;
        }
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_isEnabled; break;
        case 1: *reinterpret_cast< iSip::SipRegistrationState::Value*>(_v) = GetState(); break;
        case 2: *reinterpret_cast< QString*>(_v) = RegisterUri(); break;
        case 3: *reinterpret_cast< QString*>(_v) = ExternalIp(); break;
        case 4: *reinterpret_cast< QString*>(_v) = Password(); break;
        case 5: *reinterpret_cast< bool*>(_v) = m_isDefaultReceiver; break;
        case 6: *reinterpret_cast< bool*>(_v) = EnableRegistration(); break;
        case 7: *reinterpret_cast< int*>(_v) = RegistrationTimeSec(); break;
        case 8: *reinterpret_cast< int*>(_v) = RegistrationRetryTimeSec(); break;
        case 9: *reinterpret_cast< bool*>(_v) = m_enableOutCallWithoutRegistration; break;
        case 10: *reinterpret_cast< QString*>(_v) = IncomingCallsDestination(); break;
        case 11: *reinterpret_cast< iSip::ForwardingType::Val*>(_v) = GetForwardingType(); break;
        case 12: *reinterpret_cast< QString*>(_v) = ForwardingUri(); break;
        case 13: *reinterpret_cast< int*>(_v) = NoAnswerTimeoutSec(); break;
        }
        _id -= 14;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: Enable(*reinterpret_cast< bool*>(_v)); break;
        case 2: RegisterUri(*reinterpret_cast< QString*>(_v)); break;
        case 3: ExternalIp(*reinterpret_cast< QString*>(_v)); break;
        case 4: Password(*reinterpret_cast< QString*>(_v)); break;
        case 6: EnableRegistration(*reinterpret_cast< bool*>(_v)); break;
        case 7: RegistrationTimeSec(*reinterpret_cast< int*>(_v)); break;
        case 8: RegistrationRetryTimeSec(*reinterpret_cast< int*>(_v)); break;
        case 9: m_enableOutCallWithoutRegistration = *reinterpret_cast< bool*>(_v); break;
        case 11: SetForwardingType(*reinterpret_cast< iSip::ForwardingType::Val*>(_v)); break;
        case 12: ForwardingUri(*reinterpret_cast< QString*>(_v)); break;
        case 13: NoAnswerTimeoutSec(*reinterpret_cast< int*>(_v)); break;
        }
        _id -= 14;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 14;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<iSip::NObjSipAccessPoint> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iSip::NObjSipAccessPoint> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
