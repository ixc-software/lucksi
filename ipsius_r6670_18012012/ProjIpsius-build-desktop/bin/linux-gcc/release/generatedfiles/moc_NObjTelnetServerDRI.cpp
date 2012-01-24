/****************************************************************************
** Meta object code from reading C++ file 'NObjTelnetServerDRI.h'
**
** Created: Tue Jan 24 14:56:42 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/DRI/NObjTelnetServerDRI.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjTelnetServerDRI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DRI__NObjTelnetServerSessionDRI[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       1,   24, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      33,   32,   32,   32, 0x02,
      49,   41,   32,   32, 0x02,

 // properties: name, type, flags
      84,   76, 0x0a095001,

       0        // eod
};

static const char qt_meta_stringdata_DRI__NObjTelnetServerSessionDRI[] = {
    "DRI::NObjTelnetServerSessionDRI\0\0"
    "Close()\0pOutput\0GetStats(DRI::ICmdOutput*)\0"
    "QString\0Info\0"
};

const QMetaObject DRI::NObjTelnetServerSessionDRI::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_DRI__NObjTelnetServerSessionDRI,
      qt_meta_data_DRI__NObjTelnetServerSessionDRI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DRI::NObjTelnetServerSessionDRI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DRI::NObjTelnetServerSessionDRI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DRI::NObjTelnetServerSessionDRI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DRI__NObjTelnetServerSessionDRI))
        return static_cast<void*>(const_cast< NObjTelnetServerSessionDRI*>(this));
    if (!strcmp(_clname, "INonCreatable"))
        return static_cast< INonCreatable*>(const_cast< NObjTelnetServerSessionDRI*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int DRI::NObjTelnetServerSessionDRI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Close(); break;
        case 1: GetStats((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = GetInfo(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
static const uint qt_meta_data_DRI__NObjTelnetServerDRI[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      26,   25,   25,   25, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_DRI__NObjTelnetServerDRI[] = {
    "DRI::NObjTelnetServerDRI\0\0CloseAllSessions()\0"
};

const QMetaObject DRI::NObjTelnetServerDRI::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_DRI__NObjTelnetServerDRI,
      qt_meta_data_DRI__NObjTelnetServerDRI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DRI::NObjTelnetServerDRI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DRI::NObjTelnetServerDRI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DRI::NObjTelnetServerDRI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DRI__NObjTelnetServerDRI))
        return static_cast<void*>(const_cast< NObjTelnetServerDRI*>(this));
    if (!strcmp(_clname, "Telnet::ITelnetServerEvents"))
        return static_cast< Telnet::ITelnetServerEvents*>(const_cast< NObjTelnetServerDRI*>(this));
    if (!strcmp(_clname, "ITelnetSessionDRIToServer"))
        return static_cast< ITelnetSessionDRIToServer*>(const_cast< NObjTelnetServerDRI*>(this));
    if (!strcmp(_clname, "INonCreatable"))
        return static_cast< INonCreatable*>(const_cast< NObjTelnetServerDRI*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int DRI::NObjTelnetServerDRI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: CloseAllSessions(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<DRI::NObjTelnetServerSessionDRI> GReg_0;
	DRI::NamedObjectTypeRegister<DRI::NObjTelnetServerDRI> GReg_1;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<DRI::NObjTelnetServerSessionDRI> GRegQObj_0;
	Utils::QObjFactoryRegister<DRI::NObjTelnetServerDRI> GRegQObj_1;
}; // end namespace 
QT_END_MOC_NAMESPACE
