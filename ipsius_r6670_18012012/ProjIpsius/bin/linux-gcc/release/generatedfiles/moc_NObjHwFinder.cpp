/****************************************************************************
** Meta object code from reading C++ file 'NObjHwFinder.h'
**
** Created: Tue Jan 24 17:47:49 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IpsiusService/NObjHwFinder.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjHwFinder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IpsiusService__NObjHwFinder[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
      12,   74, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      47,   29,   28,   28, 0x02,
      88,   83,   28,   28, 0x22,
     134,  119,   28,   28, 0x02,
     159,  153,   28,   28, 0x02,
     176,   83,   28,   28, 0x02,
     206,   28,   28,   28, 0x02,
     227,  214,   28,   28, 0x02,
     261,   83,   28,   28, 0x22,
     311,  290,   28,   28, 0x02,
     349,  339,   28,   28, 0x22,
     373,  290,   28,   28, 0x02,
     404,  339,   28,   28, 0x22,

 // properties: name, type, flags
     436,  431, 0x01095003,
     452,  444, 0x0a095003,
     465,  431, 0x01095003,
     481,  477, 0x02095003,
     501,  431, 0x01095003,
     522,  477, 0x02095001,
     543,  477, 0x02095001,
     568,  477, 0x02095001,
     582,  431, 0x01095003,
     594,  431, 0x01095003,
     608,  431, 0x01095003,
     628,  431, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_IpsiusService__NObjHwFinder[] = {
    "IpsiusService::NObjHwFinder\0\0"
    "pCmd,withBadItems\0ListFirmware(DRI::ICmdOutput*,bool)\0"
    "pCmd\0ListFirmware(DRI::ICmdOutput*)\0"
    "hwNumber,alias\0Alias(int,QString)\0"
    "alias\0Unalias(QString)\0"
    "ListAliases(DRI::ICmdOutput*)\0Clean()\0"
    "pCmd,briefly\0ListBoards(DRI::ICmdOutput*,bool)\0"
    "ListBoards(DRI::ICmdOutput*)\0"
    "boardAddr,udpAppPort\0AddDirectBoard(QString,int)\0"
    "boardAddr\0AddDirectBoard(QString)\0"
    "RemoveDirectBoard(QString,int)\0"
    "RemoveDirectBoard(QString)\0bool\0Enabled\0"
    "QString\0FirmwarePath\0AllowUpdate\0int\0"
    "BroadcastListenPort\0TraceUpdateProcedure\0"
    "ToalBroadcastCounter\0FilteredBroadcastCounter\0"
    "WrongMsgCount\0OnlyLocalIP\0OnlyDirectDev\0"
    "TraceUpdateProgress\0RollbackMode\0"
};

const QMetaObject IpsiusService::NObjHwFinder::staticMetaObject = {
    { &NamedObject::staticMetaObject, qt_meta_stringdata_IpsiusService__NObjHwFinder,
      qt_meta_data_IpsiusService__NObjHwFinder, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IpsiusService::NObjHwFinder::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IpsiusService::NObjHwFinder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IpsiusService::NObjHwFinder::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IpsiusService__NObjHwFinder))
        return static_cast<void*>(const_cast< NObjHwFinder*>(this));
    if (!strcmp(_clname, "BfBootCli::IBroadcastReceiverToOwner"))
        return static_cast< BfBootCli::IBroadcastReceiverToOwner*>(const_cast< NObjHwFinder*>(this));
    if (!strcmp(_clname, "IHwBoardOwner"))
        return static_cast< IHwBoardOwner*>(const_cast< NObjHwFinder*>(this));
    return NamedObject::qt_metacast(_clname);
}

int IpsiusService::NObjHwFinder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NamedObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: ListFirmware((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: ListFirmware((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 2: Alias((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 3: Unalias((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: ListAliases((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 5: Clean(); break;
        case 6: ListBoards((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 7: ListBoards((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 8: AddDirectBoard((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: AddDirectBoard((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 10: RemoveDirectBoard((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 11: RemoveDirectBoard((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 12;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_enabled; break;
        case 1: *reinterpret_cast< QString*>(_v) = m_firmwarePath; break;
        case 2: *reinterpret_cast< bool*>(_v) = m_allowUpdate; break;
        case 3: *reinterpret_cast< int*>(_v) = m_broadcastListenPort; break;
        case 4: *reinterpret_cast< bool*>(_v) = m_traceUpdate; break;
        case 5: *reinterpret_cast< int*>(_v) = m_totalBroadcastCounter; break;
        case 6: *reinterpret_cast< int*>(_v) = m_filteredBroadcastCounter; break;
        case 7: *reinterpret_cast< int*>(_v) = getWrongMsgCount(); break;
        case 8: *reinterpret_cast< bool*>(_v) = m_onlyLocalIp; break;
        case 9: *reinterpret_cast< bool*>(_v) = m_onlyDirectDevice; break;
        case 10: *reinterpret_cast< bool*>(_v) = m_traceUpdate; break;
        case 11: *reinterpret_cast< bool*>(_v) = m_rollbackMode; break;
        }
        _id -= 12;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: SetEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 1: SetFirmwarePath(*reinterpret_cast< QString*>(_v)); break;
        case 2: AllowUpdate(*reinterpret_cast< bool*>(_v)); break;
        case 3: SetBroadcastListenPort(*reinterpret_cast< int*>(_v)); break;
        case 4: m_traceUpdate = *reinterpret_cast< bool*>(_v); break;
        case 8: m_onlyLocalIp = *reinterpret_cast< bool*>(_v); break;
        case 9: SetOnlyDirectDevice(*reinterpret_cast< bool*>(_v)); break;
        case 10: m_traceUpdate = *reinterpret_cast< bool*>(_v); break;
        case 11: m_rollbackMode = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 12;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 12;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<IpsiusService::NObjHwFinder> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IpsiusService::NObjHwFinder> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
