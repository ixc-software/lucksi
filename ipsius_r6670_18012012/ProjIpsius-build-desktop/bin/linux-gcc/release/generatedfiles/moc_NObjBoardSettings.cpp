/****************************************************************************
** Meta object code from reading C++ file 'NObjBoardSettings.h'
**
** Created: Tue Jan 24 14:55:50 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfEmul/NObjBoardSettings.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjBoardSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfEmul__NObjBoardSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
      10,   29, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      44,   27,   26,   26, 0x02,
      87,   78,   26,   26, 0x02,
     134,  114,   26,   26, 0x02,

 // properties: name, type, flags
     179,  175, 0x02095001,
     186,  175, 0x02095001,
     200,  192, 0x0a095001,
     206,  192, 0x0a095001,
     222,  217, 0x01095103,
     235,  175, 0x02095103,
     248,  192, 0x0a095001,
     256,  192, 0x0a095103,
     264,  175, 0x02095103,
     278,  175, 0x02095003,

       0        // eod
};

static const char qt_meta_stringdata_BfEmul__NObjBoardSettings[] = {
    "BfEmul::NObjBoardSettings\0\0hwType,hwNum,mac\0"
    "SetDefaultParams(int,int,QString)\0"
    "val,port\0SetUdpLogAddr(QString,int)\0"
    "use,ip,gateway,mask\0"
    "SetNetwork(bool,QString,QString,QString)\0"
    "int\0HwType\0HwNum\0QString\0HwMac\0"
    "UdpLogAddr\0bool\0UseTimestamp\0CountToStore\0"
    "Network\0UserPwd\0WaitLoginMsec\0CmpPort\0"
};

const QMetaObject BfEmul::NObjBoardSettings::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfEmul__NObjBoardSettings,
      qt_meta_data_BfEmul__NObjBoardSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfEmul::NObjBoardSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfEmul::NObjBoardSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfEmul::NObjBoardSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfEmul__NObjBoardSettings))
        return static_cast<void*>(const_cast< NObjBoardSettings*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjBoardSettings*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfEmul::NObjBoardSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SetDefaultParams((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 1: SetUdpLogAddr((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: SetNetwork((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        default: ;
        }
        _id -= 3;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = getType(); break;
        case 1: *reinterpret_cast< int*>(_v) = getNum(); break;
        case 2: *reinterpret_cast< QString*>(_v) = getMac(); break;
        case 3: *reinterpret_cast< QString*>(_v) = getLogAddr(); break;
        case 4: *reinterpret_cast< bool*>(_v) = getUseTimestamp(); break;
        case 5: *reinterpret_cast< int*>(_v) = getCountToStore(); break;
        case 6: *reinterpret_cast< QString*>(_v) = getNetwork(); break;
        case 7: *reinterpret_cast< QString*>(_v) = getUserPwd(); break;
        case 8: *reinterpret_cast< int*>(_v) = getWaitLoginMsec(); break;
        case 9: *reinterpret_cast< int*>(_v) = getCmp(); break;
        }
        _id -= 10;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 4: setUseTimestamp(*reinterpret_cast< bool*>(_v)); break;
        case 5: setCountToStore(*reinterpret_cast< int*>(_v)); break;
        case 7: setUserPwd(*reinterpret_cast< QString*>(_v)); break;
        case 8: setWaitLoginMsec(*reinterpret_cast< int*>(_v)); break;
        case 9: setCmp(*reinterpret_cast< int*>(_v)); break;
        }
        _id -= 10;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 10;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<BfEmul::NObjBoardSettings> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfEmul::NObjBoardSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
