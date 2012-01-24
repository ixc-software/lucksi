/****************************************************************************
** Meta object code from reading C++ file 'NObjSecRoot.h'
**
** Created: Tue Jan 24 14:56:47 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/DriSec/NObjSecRoot.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSecRoot.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DriSec__NObjSecSpecial[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_DriSec__NObjSecSpecial[] = {
    "DriSec::NObjSecSpecial\0"
};

const QMetaObject DriSec::NObjSecSpecial::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_DriSec__NObjSecSpecial,
      qt_meta_data_DriSec__NObjSecSpecial, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DriSec::NObjSecSpecial::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DriSec::NObjSecSpecial::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DriSec::NObjSecSpecial::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DriSec__NObjSecSpecial))
        return static_cast<void*>(const_cast< NObjSecSpecial*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSecSpecial*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int DriSec::NObjSecSpecial::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_DriSec__NObjSecProfiles[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      72,   25,   24,   24, 0x02,
     144,  111,   24,   24, 0x22,
     202,  178,   24,   24, 0x22,
     242,  228,   24,   24, 0x22,

       0        // eod
};

static const char qt_meta_stringdata_DriSec__NObjSecProfiles[] = {
    "DriSec::NObjSecProfiles\0\0"
    "name,rootMode,allowList,denyList,blockSecurity\0"
    "Add(QString,Mode,QString,QString,bool)\0"
    "name,rootMode,allowList,denyList\0"
    "Add(QString,Mode,QString,QString)\0"
    "name,rootMode,allowList\0"
    "Add(QString,Mode,QString)\0name,rootMode\0"
    "Add(QString,Mode)\0"
};

const QMetaObject DriSec::NObjSecProfiles::staticMetaObject = {
    { &NObjProfilesBase::staticMetaObject, qt_meta_stringdata_DriSec__NObjSecProfiles,
      qt_meta_data_DriSec__NObjSecProfiles, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DriSec::NObjSecProfiles::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DriSec::NObjSecProfiles::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DriSec::NObjSecProfiles::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DriSec__NObjSecProfiles))
        return static_cast<void*>(const_cast< NObjSecProfiles*>(this));
    return NObjProfilesBase::qt_metacast(_clname);
}

int DriSec::NObjSecProfiles::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NObjProfilesBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Add((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< Mode(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 1: Add((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< Mode(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 2: Add((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< Mode(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 3: Add((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< Mode(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
static const uint qt_meta_data_DriSec__NObjSecUsers[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      61,   22,   21,   21, 0x02,
     136,  102,   21,   21, 0x22,
     192,  169,   21,   21, 0x22,

       0        // eod
};

static const char qt_meta_stringdata_DriSec__NObjSecUsers[] = {
    "DriSec::NObjSecUsers\0\0"
    "login,password,profile,loginLimit,desc\0"
    "Add(QString,QString,QString,int,QString)\0"
    "login,password,profile,loginLimit\0"
    "Add(QString,QString,QString,int)\0"
    "login,password,profile\0"
    "Add(QString,QString,QString)\0"
};

const QMetaObject DriSec::NObjSecUsers::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_DriSec__NObjSecUsers,
      qt_meta_data_DriSec__NObjSecUsers, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DriSec::NObjSecUsers::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DriSec::NObjSecUsers::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DriSec::NObjSecUsers::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DriSec__NObjSecUsers))
        return static_cast<void*>(const_cast< NObjSecUsers*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSecUsers*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int DriSec::NObjSecUsers::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Add((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5]))); break;
        case 1: Add((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 2: Add((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_DriSec__NObjSecRoot[] = {

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
      42,   21,   20,   20, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_DriSec__NObjSecRoot[] = {
    "DriSec::NObjSecRoot\0\0pContext,profileName\0"
    "SwitchSecProfile(DRI::ISessionCmdContext*,QString)\0"
};

const QMetaObject DriSec::NObjSecRoot::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_DriSec__NObjSecRoot,
      qt_meta_data_DriSec__NObjSecRoot, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DriSec::NObjSecRoot::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DriSec::NObjSecRoot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DriSec::NObjSecRoot::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DriSec__NObjSecRoot))
        return static_cast<void*>(const_cast< NObjSecRoot*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSecRoot*>(this));
    if (!strcmp(_clname, "ISecContext"))
        return static_cast< ISecContext*>(const_cast< NObjSecRoot*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int DriSec::NObjSecRoot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SwitchSecProfile((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
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
	DRI::NamedObjectTypeRegister<DriSec::NObjSecSpecial> GReg_0;
	DRI::NamedObjectTypeRegister<DriSec::NObjSecProfiles> GReg_1;
	DRI::NamedObjectTypeRegister<DriSec::NObjSecUsers> GReg_2;
	DRI::NamedObjectTypeRegister<DriSec::NObjSecRoot> GReg_3;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<DriSec::NObjSecSpecial> GRegQObj_0;
	Utils::QObjFactoryRegister<DriSec::NObjSecProfiles> GRegQObj_1;
	Utils::QObjFactoryRegister<DriSec::NObjSecUsers> GRegQObj_2;
	Utils::QObjFactoryRegister<DriSec::NObjSecRoot> GRegQObj_3;
}; // end namespace 
QT_END_MOC_NAMESPACE
