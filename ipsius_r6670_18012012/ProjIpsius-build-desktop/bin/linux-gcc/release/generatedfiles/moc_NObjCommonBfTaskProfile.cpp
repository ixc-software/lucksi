/****************************************************************************
** Meta object code from reading C++ file 'NObjCommonBfTaskProfile.h'
**
** Created: Tue Jan 24 14:55:58 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfBootDri/NObjCommonBfTaskProfile.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjCommonBfTaskProfile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfBootDRI__NObjCommonBfTaskProfile[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       5,   44, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      51,   36,   35,   35, 0x02,
      96,   86,   35,   35, 0x22,
     146,  127,   35,   35, 0x02,
     208,  194,   35,   35, 0x22,
     280,  252,   35,   35, 0x02,
     351,  328,   35,   35, 0x22,

 // properties: name, type, flags
     403,  395, 0x0a095003,
     420,  416, 0x02095003,
     428,  395, 0x0a095003,
     442,  437, 0x01095003,
     454,  437, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_BfBootDRI__NObjCommonBfTaskProfile[] = {
    "BfBootDRI::NObjCommonBfTaskProfile\0\0"
    "pAsyncCmd,msec\0ObtainCbpPort(DRI::IAsyncCmd*,int)\0"
    "pAsyncCmd\0ObtainCbpPort(DRI::IAsyncCmd*)\0"
    "pAsyncCmd,mac,msec\0"
    "ObtainAddressByMac(DRI::IAsyncCmd*,QString,int)\0"
    "pAsyncCmd,mac\0ObtainAddressByMac(DRI::IAsyncCmd*,QString)\0"
    "pAsyncCmd,hwType,hwNum,msec\0"
    "ObtainAddressByNum(DRI::IAsyncCmd*,int,int,int)\0"
    "pAsyncCmd,hwType,hwNum\0"
    "ObtainAddressByNum(DRI::IAsyncCmd*,int,int)\0"
    "QString\0BoardAddress\0int\0CbpPort\0"
    "BoardPwd\0bool\0TraceClient\0TraceServer\0"
};

const QMetaObject BfBootDRI::NObjCommonBfTaskProfile::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfBootDRI__NObjCommonBfTaskProfile,
      qt_meta_data_BfBootDRI__NObjCommonBfTaskProfile, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfBootDRI::NObjCommonBfTaskProfile::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfBootDRI::NObjCommonBfTaskProfile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfBootDRI::NObjCommonBfTaskProfile::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfBootDRI__NObjCommonBfTaskProfile))
        return static_cast<void*>(const_cast< NObjCommonBfTaskProfile*>(this));
    if (!strcmp(_clname, "BfBootCli::ITaskManagerReport"))
        return static_cast< BfBootCli::ITaskManagerReport*>(const_cast< NObjCommonBfTaskProfile*>(this));
    if (!strcmp(_clname, "Utils::ILogForProgressBar"))
        return static_cast< Utils::ILogForProgressBar*>(const_cast< NObjCommonBfTaskProfile*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfBootDRI::NObjCommonBfTaskProfile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: ObtainCbpPort((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: ObtainCbpPort((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        case 2: ObtainAddressByMac((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 3: ObtainAddressByMac((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 4: ObtainAddressByNum((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 5: ObtainAddressByNum((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 6;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = m_addr; break;
        case 1: *reinterpret_cast< int*>(_v) = m_cbpPort; break;
        case 2: *reinterpret_cast< QString*>(_v) = ReadPwd(); break;
        case 3: *reinterpret_cast< bool*>(_v) = m_traceClient; break;
        case 4: *reinterpret_cast< bool*>(_v) = m_traceServer; break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_addr = *reinterpret_cast< QString*>(_v); break;
        case 1: m_cbpPort = *reinterpret_cast< int*>(_v); break;
        case 2: m_boardPwd = *reinterpret_cast< QString*>(_v); break;
        case 3: m_traceClient = *reinterpret_cast< bool*>(_v); break;
        case 4: m_traceServer = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 5;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<BfBootDRI::NObjCommonBfTaskProfile> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfBootDRI::NObjCommonBfTaskProfile> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
