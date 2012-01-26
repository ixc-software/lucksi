/****************************************************************************
** Meta object code from reading C++ file 'NObjBfTaskSetParams.h'
**
** Created: Tue Jan 24 17:47:31 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfBootDri/NObjBfTaskSetParams.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjBfTaskSetParams.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfBootDRI__NObjBfTaskSetParams[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       2,   74, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      40,   32,   31,   31, 0x02,
      71,   54,   31,   31, 0x02,
     109,  105,   31,   31, 0x02,
     140,  135,   31,   31, 0x02,
     166,  162,   31,   31, 0x02,
     197,  191,   31,   31, 0x02,
     241,  221,   31,   31, 0x02,
     289,  285,   31,   31, 0x02,
     312,   31,   31,   31, 0x22,
     333,  328,   31,   31, 0x02,
     358,  135,   31,   31, 0x02,
     390,  380,   31,   31, 0x02,

 // properties: name, type, flags
     416,  411, 0x01095003,
     436,  411, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_BfBootDRI__NObjBfTaskSetParams[] = {
    "BfBootDRI::NObjBfTaskSetParams\0\0profile\0"
    "Init(QString)\0hwType,hwNum,mac\0"
    "SetFactoryParams(int,int,QString)\0val\0"
    "SetOptUdpLogAddr(QString)\0port\0"
    "SetOptUdpLogPort(int)\0use\0"
    "SetOptUseTimestamp(bool)\0count\0"
    "SetOptCountToStore(int)\0use,ip,gateway,mask\0"
    "SetOptNetwork(bool,QString,QString,QString)\0"
    "pwd\0SetOptUserPwd(QString)\0SetOptUserPwd()\0"
    "msec\0SetOptWaitLoginMsec(int)\0"
    "SetOptCmpAppPort(int)\0pAsyncCmd\0"
    "Run(DRI::IAsyncCmd*)\0bool\0ResetFactoryIfExist\0"
    "CheckAfter\0"
};

const QMetaObject BfBootDRI::NObjBfTaskSetParams::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfBootDRI__NObjBfTaskSetParams,
      qt_meta_data_BfBootDRI__NObjBfTaskSetParams, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfBootDRI::NObjBfTaskSetParams::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfBootDRI::NObjBfTaskSetParams::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfBootDRI::NObjBfTaskSetParams::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfBootDRI__NObjBfTaskSetParams))
        return static_cast<void*>(const_cast< NObjBfTaskSetParams*>(this));
    if (!strcmp(_clname, "ITask"))
        return static_cast< ITask*>(const_cast< NObjBfTaskSetParams*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfBootDRI::NObjBfTaskSetParams::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Init((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: SetFactoryParams((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 2: SetOptUdpLogAddr((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: SetOptUdpLogPort((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: SetOptUseTimestamp((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: SetOptCountToStore((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: SetOptNetwork((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 7: SetOptUserPwd((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: SetOptUserPwd(); break;
        case 9: SetOptWaitLoginMsec((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: SetOptCmpAppPort((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: Run((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 12;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_useAutoReset; break;
        case 1: *reinterpret_cast< bool*>(_v) = m_checkAfter; break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_useAutoReset = *reinterpret_cast< bool*>(_v); break;
        case 1: m_checkAfter = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<BfBootDRI::NObjBfTaskSetParams> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfBootDRI::NObjBfTaskSetParams> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
