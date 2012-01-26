/****************************************************************************
** Meta object code from reading C++ file 'SecUtils.h'
**
** Created: Tue Jan 24 17:48:16 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/DriSec/SecUtils.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SecUtils.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DriSec__NObjProfilesBase[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       1,   14, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // enums: name, flags, count, data
      25, 0x0,    3,   18,

 // enum data: key, value
      30, uint(DriSec::NObjProfilesBase::acDeny),
      37, uint(DriSec::NObjProfilesBase::acReadOnly),
      48, uint(DriSec::NObjProfilesBase::acFull),

       0        // eod
};

static const char qt_meta_stringdata_DriSec__NObjProfilesBase[] = {
    "DriSec::NObjProfilesBase\0Mode\0acDeny\0"
    "acReadOnly\0acFull\0"
};

const QMetaObject DriSec::NObjProfilesBase::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_DriSec__NObjProfilesBase,
      qt_meta_data_DriSec__NObjProfilesBase, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DriSec::NObjProfilesBase::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DriSec::NObjProfilesBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DriSec::NObjProfilesBase::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DriSec__NObjProfilesBase))
        return static_cast<void*>(const_cast< NObjProfilesBase*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjProfilesBase*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int DriSec::NObjProfilesBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<DriSec::NObjProfilesBase> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<DriSec::NObjProfilesBase> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
