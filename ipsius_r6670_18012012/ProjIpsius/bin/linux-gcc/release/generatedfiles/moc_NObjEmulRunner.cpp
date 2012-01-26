/****************************************************************************
** Meta object code from reading C++ file 'NObjEmulRunner.h'
**
** Created: Tue Jan 24 17:47:18 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfEmul/NObjEmulRunner.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjEmulRunner.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfEmul__NObjEmulRunner[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       2,   39, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      32,   24,   23,   23, 0x02,
      85,   54,   23,   23, 0x02,
     134,  117,   23,   23, 0x02,
     168,  158,   23,   23, 0x02,
     198,  158,   23,   23, 0x02,

 // properties: name, type, flags
     231,  227, 0x02095001,
     242,  227, 0x02095001,

       0        // eod
};

static const char qt_meta_stringdata_BfEmul__NObjEmulRunner[] = {
    "BfEmul::NObjEmulRunner\0\0min,max\0"
    "InitRtpRange(int,int)\0"
    "name,boardNumber1,boardNumber2\0"
    "CreateEmulPair(QString,int,int)\0"
    "name,boardNumber\0CreateEmul(QString,int)\0"
    "pAsyncCmd\0StartAllEmul(DRI::IAsyncCmd*)\0"
    "StopAllEmul(DRI::IAsyncCmd*)\0int\0"
    "MinRtpPort\0MaxRtpPort\0"
};

const QMetaObject BfEmul::NObjEmulRunner::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfEmul__NObjEmulRunner,
      qt_meta_data_BfEmul__NObjEmulRunner, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfEmul::NObjEmulRunner::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfEmul::NObjEmulRunner::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfEmul::NObjEmulRunner::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfEmul__NObjEmulRunner))
        return static_cast<void*>(const_cast< NObjEmulRunner*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfEmul::NObjEmulRunner::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: InitRtpRange((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: CreateEmulPair((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: CreateEmul((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: StartAllEmul((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        case 4: StopAllEmul((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = MinRtpPort(); break;
        case 1: *reinterpret_cast< int*>(_v) = MaxRtpPort(); break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::WriteProperty) {
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
	DRI::NamedObjectTypeRegister<BfEmul::NObjEmulRunner> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfEmul::NObjEmulRunner> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
