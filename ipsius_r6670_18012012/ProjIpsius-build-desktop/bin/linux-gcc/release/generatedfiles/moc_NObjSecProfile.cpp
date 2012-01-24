/****************************************************************************
** Meta object code from reading C++ file 'NObjSecProfile.h'
**
** Created: Tue Jan 24 14:56:44 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/DriSec/NObjSecProfile.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSecProfile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DriSec__NObjSecProfile[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      46,   24,   23,   23, 0x02,
      83,   74,   23,   23, 0x22,
     145,  106,   23,   23, 0x02,
     235,  205,   23,   23, 0x22,
     307,  287,   23,   23, 0x22,
     357,  351,   23,   23, 0x02,
     372,   23,   23,   23, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_DriSec__NObjSecProfile[] = {
    "DriSec::NObjSecProfile\0\0pContext,withSysItems\0"
    "List(DRI::ICmdOutput*,bool)\0pContext\0"
    "List(DRI::ICmdOutput*)\0"
    "path,recursive,mode,allowList,denyList\0"
    "AddRec(QString,bool,NObjProfilesBase::Mode,QString,QString)\0"
    "path,recursive,mode,allowList\0"
    "AddRec(QString,bool,NObjProfilesBase::Mode,QString)\0"
    "path,recursive,mode\0"
    "AddRec(QString,bool,NObjProfilesBase::Mode)\0"
    "index\0DeleteRec(int)\0Delete()\0"
};

const QMetaObject DriSec::NObjSecProfile::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_DriSec__NObjSecProfile,
      qt_meta_data_DriSec__NObjSecProfile, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DriSec::NObjSecProfile::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DriSec::NObjSecProfile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DriSec::NObjSecProfile::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DriSec__NObjSecProfile))
        return static_cast<void*>(const_cast< NObjSecProfile*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSecProfile*>(this));
    if (!strcmp(_clname, "ISecurity"))
        return static_cast< ISecurity*>(const_cast< NObjSecProfile*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int DriSec::NObjSecProfile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: List((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: List((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 2: AddRec((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< NObjProfilesBase::Mode(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5]))); break;
        case 3: AddRec((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< NObjProfilesBase::Mode(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 4: AddRec((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< NObjProfilesBase::Mode(*)>(_a[3]))); break;
        case 5: DeleteRec((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: Delete(); break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<DriSec::NObjSecProfile> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<DriSec::NObjSecProfile> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
