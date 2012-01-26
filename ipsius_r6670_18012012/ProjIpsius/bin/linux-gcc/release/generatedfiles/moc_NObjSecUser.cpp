/****************************************************************************
** Meta object code from reading C++ file 'NObjSecUser.h'
**
** Created: Tue Jan 24 17:48:15 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/DriSec/NObjSecUser.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSecUser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DriSec__NObjSecUser[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       1,   29, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x02,
      36,   30,   20,   20, 0x02,
      55,   30,   20,   20, 0x02,

 // properties: name, type, flags
      84,   76, 0x0a095001,

       0        // eod
};

static const char qt_meta_stringdata_DriSec__NObjSecUser[] = {
    "DriSec::NObjSecUser\0\0Delete()\0value\0"
    "SetLoginLimit(int)\0SetPassword(QString)\0"
    "QString\0Info\0"
};

const QMetaObject DriSec::NObjSecUser::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_DriSec__NObjSecUser,
      qt_meta_data_DriSec__NObjSecUser, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DriSec::NObjSecUser::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DriSec::NObjSecUser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DriSec::NObjSecUser::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DriSec__NObjSecUser))
        return static_cast<void*>(const_cast< NObjSecUser*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSecUser*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int DriSec::NObjSecUser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Delete(); break;
        case 1: SetLoginLimit((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: SetPassword((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
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

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<DriSec::NObjSecUser> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<DriSec::NObjSecUser> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
