/****************************************************************************
** Meta object code from reading C++ file 'NObjSipAutotrainer.h'
**
** Created: Tue Jan 24 17:49:59 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/NObjSipAutotrainer.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSipAutotrainer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__NObjSipAutotrainer[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       3,   49, // properties
       1,   58, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      26,   25,   25,   25, 0x02,
      55,   46,   25,   25, 0x02,
      99,   94,   25,   25, 0x22,
     130,   25,   25,   25, 0x02,
     156,   46,   25,   25, 0x02,
     194,   94,   25,   25, 0x22,
     224,   25,   25,   25, 0x02,

 // properties: name, type, flags
     257,  249, 0x0a095003,
     266,  249, 0x0a095001,
     285,  249, 0x0a095001,

 // enums: name, flags, count, data
     303, 0x0,    8,   62,

 // enum data: key, value
     312, uint(iSip::NObjSipAutotrainer::Dial),
     317, uint(iSip::NObjSipAutotrainer::Wait),
     322, uint(iSip::NObjSipAutotrainer::Alerting),
     331, uint(iSip::NObjSipAutotrainer::Answer),
     338, uint(iSip::NObjSipAutotrainer::RedirectTo),
     349, uint(iSip::NObjSipAutotrainer::Hold),
     354, uint(iSip::NObjSipAutotrainer::Unhold),
     361, uint(iSip::NObjSipAutotrainer::Close),

       0        // eod
};

static const char qt_meta_stringdata_iSip__NObjSipAutotrainer[] = {
    "iSip::NObjSipAutotrainer\0\0StartOutgoingCall()\0"
    "type,par\0AddCommandForOutCall(Commands,QString)\0"
    "type\0AddCommandForOutCall(Commands)\0"
    "ClearCommandsForOutCall()\0"
    "AddCommandForInCall(Commands,QString)\0"
    "AddCommandForInCall(Commands)\0"
    "ClearCommandsForInCall()\0QString\0"
    "SipPoint\0CommandsForOutCall\0"
    "CommandsForInCall\0Commands\0Dial\0Wait\0"
    "Alerting\0Answer\0RedirectTo\0Hold\0Unhold\0"
    "Close\0"
};

const QMetaObject iSip::NObjSipAutotrainer::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iSip__NObjSipAutotrainer,
      qt_meta_data_iSip__NObjSipAutotrainer, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::NObjSipAutotrainer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::NObjSipAutotrainer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::NObjSipAutotrainer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__NObjSipAutotrainer))
        return static_cast<void*>(const_cast< NObjSipAutotrainer*>(this));
    if (!strcmp(_clname, "ISipAccessPointEvents"))
        return static_cast< ISipAccessPointEvents*>(const_cast< NObjSipAutotrainer*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iSip::NObjSipAutotrainer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: StartOutgoingCall(); break;
        case 1: AddCommandForOutCall((*reinterpret_cast< Commands(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 2: AddCommandForOutCall((*reinterpret_cast< Commands(*)>(_a[1]))); break;
        case 3: ClearCommandsForOutCall(); break;
        case 4: AddCommandForInCall((*reinterpret_cast< Commands(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 5: AddCommandForInCall((*reinterpret_cast< Commands(*)>(_a[1]))); break;
        case 6: ClearCommandsForInCall(); break;
        default: ;
        }
        _id -= 7;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = m_sipAccessPointName; break;
        case 1: *reinterpret_cast< QString*>(_v) = m_commandsOutStr; break;
        case 2: *reinterpret_cast< QString*>(_v) = m_commandsInStr; break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: SipPoint(*reinterpret_cast< QString*>(_v)); break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 3;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<iSip::NObjSipAutotrainer> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iSip::NObjSipAutotrainer> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
