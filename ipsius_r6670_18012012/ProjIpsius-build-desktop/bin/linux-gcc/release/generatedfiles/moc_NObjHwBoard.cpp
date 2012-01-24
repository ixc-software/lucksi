/****************************************************************************
** Meta object code from reading C++ file 'NObjHwBoard.h'
**
** Created: Tue Jan 24 14:56:19 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IpsiusService/NObjHwBoard.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjHwBoard.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IpsiusService__NObjHwBoard[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
      19,   19, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      41,   28,   27,   27, 0x02,

 // properties: name, type, flags
      77,   73, 0x02095001,
      86,   73, 0x02095001,
      99,   91, 0x0a095001,
     102,   91, 0x0a095001,
     106,   73, 0x02095001,
     117,   73, 0x02095001,
     125,   73, 0x02095001,
     137,   91, 0x0a095001,
     153,   73, 0x02095001,
     165,   91, 0x0a095001,
     181,   91, 0x0a095001,
     187,   91, 0x0a095001,
     197,   91, 0x0a095001,
     213,  203, 0x10095001,
     223,  203, 0x10095001,
     232,   73, 0x02095001,
     248,   73, 0x02095001,
     265,  203, 0x10095001,
     281,  276, 0x01095001,

       0        // eod
};

static const char qt_meta_stringdata_IpsiusService__NObjHwBoard[] = {
    "IpsiusService::NObjHwBoard\0\0pCmd,briefly\0"
    "ListInfo(DRI::ICmdOutput*,bool)\0int\0"
    "HwNumber\0HwID\0QString\0IP\0MAC\0BooterPort\0"
    "CmpPort\0BootRelease\0BootReleaseInfo\0"
    "SoftRelease\0SoftReleaseInfo\0State\0"
    "StateDesc\0Owner\0QDateTime\0FirstRecv\0"
    "LastRecv\0DiscoveredCount\0BroadcastCounter\0"
    "StateEnter\0bool\0Filtered\0"
};

const QMetaObject IpsiusService::NObjHwBoard::staticMetaObject = {
    { &NamedObject::staticMetaObject, qt_meta_stringdata_IpsiusService__NObjHwBoard,
      qt_meta_data_IpsiusService__NObjHwBoard, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IpsiusService::NObjHwBoard::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IpsiusService::NObjHwBoard::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IpsiusService::NObjHwBoard::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IpsiusService__NObjHwBoard))
        return static_cast<void*>(const_cast< NObjHwBoard*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjHwBoard*>(this));
    return NamedObject::qt_metacast(_clname);
}

int IpsiusService::NObjHwBoard::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NamedObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: ListInfo((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = getHwNumber(); break;
        case 1: *reinterpret_cast< int*>(_v) = getHwID(); break;
        case 2: *reinterpret_cast< QString*>(_v) = getIP(); break;
        case 3: *reinterpret_cast< QString*>(_v) = getMAC(); break;
        case 4: *reinterpret_cast< int*>(_v) = getBooterPort(); break;
        case 5: *reinterpret_cast< int*>(_v) = getCmpPort(); break;
        case 6: *reinterpret_cast< int*>(_v) = getBootRelease(); break;
        case 7: *reinterpret_cast< QString*>(_v) = getBootReleaseInfo(); break;
        case 8: *reinterpret_cast< int*>(_v) = getSoftRelease(); break;
        case 9: *reinterpret_cast< QString*>(_v) = getSoftReleaseInfo(); break;
        case 10: *reinterpret_cast< QString*>(_v) = getState(); break;
        case 11: *reinterpret_cast< QString*>(_v) = getStateDesc(); break;
        case 12: *reinterpret_cast< QString*>(_v) = getOwner(); break;
        case 13: *reinterpret_cast< QDateTime*>(_v) = getFirstRecv(); break;
        case 14: *reinterpret_cast< QDateTime*>(_v) = getLastRecv(); break;
        case 15: *reinterpret_cast< int*>(_v) = m_discovered; break;
        case 16: *reinterpret_cast< int*>(_v) = m_broadcastCounter; break;
        case 17: *reinterpret_cast< QDateTime*>(_v) = getStateEnter(); break;
        case 18: *reinterpret_cast< bool*>(_v) = IsFiltered(); break;
        }
        _id -= 19;
    } else if (_c == QMetaObject::WriteProperty) {
        _id -= 19;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 19;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 19;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 19;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 19;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 19;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 19;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<IpsiusService::NObjHwBoard> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IpsiusService::NObjHwBoard> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
