/****************************************************************************
** Meta object code from reading C++ file 'NObjMiniFlasher.h'
**
** Created: Tue Jan 24 14:56:27 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/MiniFlasher/NObjMiniFlasher.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjMiniFlasher.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MiniFlasher__NObjMiniFlasher[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       4,   74, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      49,   30,   29,   29, 0x02,
     113,   83,   29,   29, 0x02,
     185,  159,   29,   29, 0x22,
     226,   30,   29,   29, 0x22,
     274,  259,   29,   29, 0x02,
     320,  310,   29,   29, 0x22,
     383,  348,   29,   29, 0x02,
     462,  434,   29,   29, 0x22,
     527,  505,   29,   29, 0x02,
     591,  576,   29,   29, 0x22,
     632,  310,   29,   29, 0x02,
     679,  659,   29,   29, 0x02,

 // properties: name, type, flags
     718,  714, 0x02095003,
     726,  714, 0x02095003,
     735,  714, 0x02095103,
     750,  714, 0x02095103,

       0        // eod
};

static const char qt_meta_stringdata_MiniFlasher__NObjMiniFlasher[] = {
    "MiniFlasher::NObjMiniFlasher\0\0"
    "pAsyncCmd,fileName\0UartBoot(DRI::IAsyncCmd*,QString)\0"
    "pAsyncCmd,fileName,offset,run\0"
    "LoadImg(DRI::IAsyncCmd*,QString,QString,bool)\0"
    "pAsyncCmd,fileName,offset\0"
    "LoadImg(DRI::IAsyncCmd*,QString,QString)\0"
    "LoadImg(DRI::IAsyncCmd*,QString)\0"
    "pAsyncCmd,size\0EraseFlash(DRI::IAsyncCmd*,QString)\0"
    "pAsyncCmd\0EraseFlash(DRI::IAsyncCmd*)\0"
    "pAsyncCmd,destFileName,size,offset\0"
    "FlashDump(DRI::IAsyncCmd*,QString,QString,QString)\0"
    "pAsyncCmd,destFileName,size\0"
    "FlashDump(DRI::IAsyncCmd*,QString,QString)\0"
    "pAsyncCmd,file,offset\0"
    "FlashDumpVerify(DRI::IAsyncCmd*,QString,QString)\0"
    "pAsyncCmd,file\0FlashDumpVerify(DRI::IAsyncCmd*,QString)\0"
    "FlashTest(DRI::IAsyncCmd*)\0"
    "pAsyncCmd,timeToRun\0"
    "UartBenchmark(DRI::IAsyncCmd*,int)\0"
    "int\0ComPort\0BaudRate\0ConnectTimeout\0"
    "ConnectRepeatInterval\0"
};

const QMetaObject MiniFlasher::NObjMiniFlasher::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_MiniFlasher__NObjMiniFlasher,
      qt_meta_data_MiniFlasher__NObjMiniFlasher, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MiniFlasher::NObjMiniFlasher::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MiniFlasher::NObjMiniFlasher::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MiniFlasher::NObjMiniFlasher::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MiniFlasher__NObjMiniFlasher))
        return static_cast<void*>(const_cast< NObjMiniFlasher*>(this));
    if (!strcmp(_clname, "IMfProcessEvents"))
        return static_cast< IMfProcessEvents*>(const_cast< NObjMiniFlasher*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int MiniFlasher::NObjMiniFlasher::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: UartBoot((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: LoadImg((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 2: LoadImg((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 3: LoadImg((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 4: EraseFlash((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: EraseFlash((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        case 6: FlashDump((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 7: FlashDump((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 8: FlashDumpVerify((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 9: FlashDumpVerify((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 10: FlashTest((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        case 11: UartBenchmark((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 12;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = m_comPort; break;
        case 1: *reinterpret_cast< int*>(_v) = m_baudRate; break;
        case 2: *reinterpret_cast< int*>(_v) = ConnectTimeout(); break;
        case 3: *reinterpret_cast< int*>(_v) = ConnectRepeatInterval(); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_comPort = *reinterpret_cast< int*>(_v); break;
        case 1: m_baudRate = *reinterpret_cast< int*>(_v); break;
        case 2: setConnectTimeout(*reinterpret_cast< int*>(_v)); break;
        case 3: setConnectRepeatInterval(*reinterpret_cast< int*>(_v)); break;
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
	DRI::NamedObjectTypeRegister<MiniFlasher::NObjMiniFlasher> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<MiniFlasher::NObjMiniFlasher> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
