/****************************************************************************
** Meta object code from reading C++ file 'NObjSbpSettings.h'
**
** Created: Tue Jan 24 14:56:25 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/SafeBiProtoExt/NObjSbpSettings.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSbpSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SBProtoExt__NObjSbpSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       8,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      32,   28, 0x02095001,
      40,   28, 0x02095001,
      54,   28, 0x02095003,
      70,   28, 0x02095003,
      88,   28, 0x02095103,
     100,   28, 0x02095003,
     121,   28, 0x02095003,
     135,   28, 0x02095003,

       0        // eod
};

static const char qt_meta_stringdata_SBProtoExt__NObjSbpSettings[] = {
    "SBProtoExt::NObjSbpSettings\0int\0Version\0"
    "PacketMaxSize\0ResponceTimeout\0"
    "ReceiveEndTimeout\0PoolTimeout\0"
    "TimeoutCheckInterval\0TraceCountPar\0"
    "TraceBinMaxBytes\0"
};

const QMetaObject SBProtoExt::NObjSbpSettings::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_SBProtoExt__NObjSbpSettings,
      qt_meta_data_SBProtoExt__NObjSbpSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SBProtoExt::NObjSbpSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SBProtoExt::NObjSbpSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SBProtoExt::NObjSbpSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SBProtoExt__NObjSbpSettings))
        return static_cast<void*>(const_cast< NObjSbpSettings*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSbpSettings*>(this));
    if (!strcmp(_clname, "SBProto::SbpSettings"))
        return static_cast< SBProto::SbpSettings*>(const_cast< NObjSbpSettings*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int SBProtoExt::NObjSbpSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = getCurrentProtoVersion(); break;
        case 1: *reinterpret_cast< int*>(_v) = getMaxSendSize(); break;
        case 2: *reinterpret_cast< int*>(_v) = getResponceTimeoutMs(); break;
        case 3: *reinterpret_cast< int*>(_v) = getReceiveEndTimeoutMs(); break;
        case 4: *reinterpret_cast< int*>(_v) = getPoolTimeout(); break;
        case 5: *reinterpret_cast< int*>(_v) = getTimeoutCheckIntervalMs(); break;
        case 6: *reinterpret_cast< int*>(_v) = CountPar(); break;
        case 7: *reinterpret_cast< int*>(_v) = BinMaxBytes(); break;
        }
        _id -= 8;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 2: setResponceTimeoutMs(*reinterpret_cast< int*>(_v)); break;
        case 3: setReceiveEndTimeoutMs(*reinterpret_cast< int*>(_v)); break;
        case 4: setPoolTimeout(*reinterpret_cast< int*>(_v)); break;
        case 5: setTimeoutCheckIntervalMs(*reinterpret_cast< int*>(_v)); break;
        case 6: CountPar(*reinterpret_cast< int*>(_v)); break;
        case 7: BinMaxBytes(*reinterpret_cast< int*>(_v)); break;
        }
        _id -= 8;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 8;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<SBProtoExt::NObjSbpSettings> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<SBProtoExt::NObjSbpSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
