/****************************************************************************
** Meta object code from reading C++ file 'NObjBroadcastReceiver.h'
**
** Created: Tue Jan 24 17:47:23 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfBootDri/NObjBroadcastReceiver.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjBroadcastReceiver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfBootDRI__NObjBroadcastReceiver[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       7,   29, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      63,   34,   33,   33, 0x02,
     123,  102,   33,   33, 0x22,
     168,  158,   33,   33, 0x02,

 // properties: name, type, flags
     211,  207, 0x02095003,
     220,  207, 0x02095003,
     227,  207, 0x02095003,
     235,  207, 0x02095003,
     251,  243, 0x0a095103,
     255,  207, 0x02095003,
     266,  243, 0x0a095003,

       0        // eod
};

static const char qt_meta_stringdata_BfBootDRI__NObjBroadcastReceiver[] = {
    "BfBootDRI::NObjBroadcastReceiver\0\0"
    "pAsyncCmd,resetAfter,maxMsec\0"
    "StartWaiting(DRI::IAsyncCmd*,bool,int)\0"
    "pAsyncCmd,resetAfter\0"
    "StartWaiting(DRI::IAsyncCmd*,bool)\0"
    "pAsyncCmd\0ViewLastWaitingResult(DRI::IAsyncCmd*)\0"
    "int\0HwNumber\0HwType\0CbpPort\0CmpPort\0"
    "QString\0Mac\0SoftNumber\0SrcAddress\0"
};

const QMetaObject BfBootDRI::NObjBroadcastReceiver::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfBootDRI__NObjBroadcastReceiver,
      qt_meta_data_BfBootDRI__NObjBroadcastReceiver, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfBootDRI::NObjBroadcastReceiver::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfBootDRI::NObjBroadcastReceiver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfBootDRI::NObjBroadcastReceiver::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfBootDRI__NObjBroadcastReceiver))
        return static_cast<void*>(const_cast< NObjBroadcastReceiver*>(this));
    if (!strcmp(_clname, "BfBootCli::IBroadcastReceiverToOwner"))
        return static_cast< BfBootCli::IBroadcastReceiverToOwner*>(const_cast< NObjBroadcastReceiver*>(this));
    if (!strcmp(_clname, "BfBootCli::BroadcastFilter"))
        return static_cast< BfBootCli::BroadcastFilter*>(const_cast< NObjBroadcastReceiver*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfBootDRI::NObjBroadcastReceiver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: StartWaiting((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: StartWaiting((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: ViewLastWaitingResult((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = getNum(); break;
        case 1: *reinterpret_cast< int*>(_v) = getType(); break;
        case 2: *reinterpret_cast< int*>(_v) = getCbp(); break;
        case 3: *reinterpret_cast< int*>(_v) = getCmp(); break;
        case 4: *reinterpret_cast< QString*>(_v) = getMac(); break;
        case 5: *reinterpret_cast< int*>(_v) = getRel(); break;
        case 6: *reinterpret_cast< QString*>(_v) = getAddr(); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_hwNum = *reinterpret_cast< int*>(_v); break;
        case 1: m_hwType = *reinterpret_cast< int*>(_v); break;
        case 2: m_cbpPort = *reinterpret_cast< int*>(_v); break;
        case 3: m_cmpPort = *reinterpret_cast< int*>(_v); break;
        case 4: setMac(*reinterpret_cast< QString*>(_v)); break;
        case 5: m_softNum = *reinterpret_cast< int*>(_v); break;
        case 6: m_srcAddress = *reinterpret_cast< QString*>(_v); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<BfBootDRI::NObjBroadcastReceiver> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfBootDRI::NObjBroadcastReceiver> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
