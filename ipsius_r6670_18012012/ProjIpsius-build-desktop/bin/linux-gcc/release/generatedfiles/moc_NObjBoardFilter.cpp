/****************************************************************************
** Meta object code from reading C++ file 'NObjBoardFilter.h'
**
** Created: Tue Jan 24 14:56:16 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IpsiusService/NObjBoardFilter.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjBoardFilter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IpsiusService__NObjBoardFilter[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       2,   29, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      42,   32,   31,   31, 0x02,
      59,   32,   31,   31, 0x02,
      76,   31,   31,   31, 0x02,

 // properties: name, type, flags
      92,   84, 0x0a095001,
     100,   84, 0x0a095001,

       0        // eod
};

static const char qt_meta_stringdata_IpsiusService__NObjBoardFilter[] = {
    "IpsiusService::NObjBoardFilter\0\0"
    "rangeList\0Include(QString)\0Exclude(QString)\0"
    "Clear()\0QString\0Exclude\0Include\0"
};

const QMetaObject IpsiusService::NObjBoardFilter::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_IpsiusService__NObjBoardFilter,
      qt_meta_data_IpsiusService__NObjBoardFilter, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IpsiusService::NObjBoardFilter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IpsiusService::NObjBoardFilter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IpsiusService::NObjBoardFilter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IpsiusService__NObjBoardFilter))
        return static_cast<void*>(const_cast< NObjBoardFilter*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjBoardFilter*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int IpsiusService::NObjBoardFilter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Include((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: Exclude((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: Clear(); break;
        default: ;
        }
        _id -= 3;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = getExclude(); break;
        case 1: *reinterpret_cast< QString*>(_v) = getInclude(); break;
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
	DRI::NamedObjectTypeRegister<IpsiusService::NObjBoardFilter> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IpsiusService::NObjBoardFilter> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
