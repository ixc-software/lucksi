/****************************************************************************
** Meta object code from reading C++ file 'NObjDomainAutorun.h'
**
** Created: Tue Jan 24 14:56:39 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Domain/NObjDomainAutorun.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjDomainAutorun.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Domain__NObjDomainAutorun[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       1,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      31,   26, 0x01095103,

       0        // eod
};

static const char qt_meta_stringdata_Domain__NObjDomainAutorun[] = {
    "Domain::NObjDomainAutorun\0bool\0"
    "TraceToCout\0"
};

const QMetaObject Domain::NObjDomainAutorun::staticMetaObject = {
    { &NamedObject::staticMetaObject, qt_meta_stringdata_Domain__NObjDomainAutorun,
      qt_meta_data_Domain__NObjDomainAutorun, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Domain::NObjDomainAutorun::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Domain::NObjDomainAutorun::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Domain::NObjDomainAutorun::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Domain__NObjDomainAutorun))
        return static_cast<void*>(const_cast< NObjDomainAutorun*>(this));
    if (!strcmp(_clname, "Utils::SafeRefServer"))
        return static_cast< Utils::SafeRefServer*>(const_cast< NObjDomainAutorun*>(this));
    if (!strcmp(_clname, "DRI::IFileExecuteDone"))
        return static_cast< DRI::IFileExecuteDone*>(const_cast< NObjDomainAutorun*>(this));
    if (!strcmp(_clname, "DRI::IFileDRICoutTracer"))
        return static_cast< DRI::IFileDRICoutTracer*>(const_cast< NObjDomainAutorun*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjDomainAutorun*>(this));
    return NamedObject::qt_metacast(_clname);
}

int Domain::NObjDomainAutorun::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NamedObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = TraceToCout(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setTraceToCout(*reinterpret_cast< bool*>(_v)); break;
        }
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
	DRI::NamedObjectTypeRegister<Domain::NObjDomainAutorun> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Domain::NObjDomainAutorun> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
