/****************************************************************************
** Meta object code from reading C++ file 'Dss1Def.h'
**
** Created: Tue Jan 24 14:57:03 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Dss1ToSip/Dss1Def.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Dss1Def.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dss1ToSip__TypeOfNumber[] = {

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
      24, 0x0,    6,   18,

 // enum data: key, value
      30, uint(Dss1ToSip::TypeOfNumber::Unknown),
      38, uint(Dss1ToSip::TypeOfNumber::International),
      52, uint(Dss1ToSip::TypeOfNumber::National),
      61, uint(Dss1ToSip::TypeOfNumber::NetSpecific),
      73, uint(Dss1ToSip::TypeOfNumber::Subscriber),
      84, uint(Dss1ToSip::TypeOfNumber::Abbreviated),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__TypeOfNumber[] = {
    "Dss1ToSip::TypeOfNumber\0Value\0Unknown\0"
    "International\0National\0NetSpecific\0"
    "Subscriber\0Abbreviated\0"
};

const QMetaObject Dss1ToSip::TypeOfNumber::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__TypeOfNumber,
      qt_meta_data_Dss1ToSip__TypeOfNumber, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::TypeOfNumber::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::TypeOfNumber::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::TypeOfNumber::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__TypeOfNumber))
        return static_cast<void*>(const_cast< TypeOfNumber*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< TypeOfNumber*>(this));
    return QObject::qt_metacast(_clname);
}

int Dss1ToSip::TypeOfNumber::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_Dss1ToSip__NumberingPlan[] = {

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
      25, 0x0,    6,   18,

 // enum data: key, value
      31, uint(Dss1ToSip::NumberingPlan::Unknown),
      39, uint(Dss1ToSip::NumberingPlan::ISDN),
      44, uint(Dss1ToSip::NumberingPlan::Data),
      49, uint(Dss1ToSip::NumberingPlan::Telex),
      55, uint(Dss1ToSip::NumberingPlan::National),
      64, uint(Dss1ToSip::NumberingPlan::Private),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__NumberingPlan[] = {
    "Dss1ToSip::NumberingPlan\0Value\0Unknown\0"
    "ISDN\0Data\0Telex\0National\0Private\0"
};

const QMetaObject Dss1ToSip::NumberingPlan::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__NumberingPlan,
      qt_meta_data_Dss1ToSip__NumberingPlan, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::NumberingPlan::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::NumberingPlan::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::NumberingPlan::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__NumberingPlan))
        return static_cast<void*>(const_cast< NumberingPlan*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< NumberingPlan*>(this));
    return QObject::qt_metacast(_clname);
}

int Dss1ToSip::NumberingPlan::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_Dss1ToSip__PresentInd[] = {

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
      22, 0x0,    4,   18,

 // enum data: key, value
      28, uint(Dss1ToSip::PresentInd::Allowed),
      36, uint(Dss1ToSip::PresentInd::Restricted),
      47, uint(Dss1ToSip::PresentInd::NotAvailable),
      60, uint(Dss1ToSip::PresentInd::PesenIndReserved),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__PresentInd[] = {
    "Dss1ToSip::PresentInd\0Value\0Allowed\0"
    "Restricted\0NotAvailable\0PesenIndReserved\0"
};

const QMetaObject Dss1ToSip::PresentInd::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__PresentInd,
      qt_meta_data_Dss1ToSip__PresentInd, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::PresentInd::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::PresentInd::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::PresentInd::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__PresentInd))
        return static_cast<void*>(const_cast< PresentInd*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< PresentInd*>(this));
    return QObject::qt_metacast(_clname);
}

int Dss1ToSip::PresentInd::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_Dss1ToSip__ScreeningInd[] = {

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
      24, 0x0,    4,   18,

 // enum data: key, value
      30, uint(Dss1ToSip::ScreeningInd::UserProv_NotScreened),
      51, uint(Dss1ToSip::ScreeningInd::UserProv_Passed),
      67, uint(Dss1ToSip::ScreeningInd::UserProv_Failed),
      83, uint(Dss1ToSip::ScreeningInd::NetProv),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__ScreeningInd[] = {
    "Dss1ToSip::ScreeningInd\0Value\0"
    "UserProv_NotScreened\0UserProv_Passed\0"
    "UserProv_Failed\0NetProv\0"
};

const QMetaObject Dss1ToSip::ScreeningInd::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__ScreeningInd,
      qt_meta_data_Dss1ToSip__ScreeningInd, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::ScreeningInd::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::ScreeningInd::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::ScreeningInd::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__ScreeningInd))
        return static_cast<void*>(const_cast< ScreeningInd*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< ScreeningInd*>(this));
    return QObject::qt_metacast(_clname);
}

int Dss1ToSip::ScreeningInd::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_Dss1ToSip__HardType[] = {

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
      20, 0x0,    3,   18,

 // enum data: key, value
      26, uint(Dss1ToSip::HardType::ErrorHardType),
      40, uint(Dss1ToSip::HardType::T1),
      43, uint(Dss1ToSip::HardType::E1),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__HardType[] = {
    "Dss1ToSip::HardType\0Value\0ErrorHardType\0"
    "T1\0E1\0"
};

const QMetaObject Dss1ToSip::HardType::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__HardType,
      qt_meta_data_Dss1ToSip__HardType, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::HardType::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::HardType::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::HardType::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__HardType))
        return static_cast<void*>(const_cast< HardType*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< HardType*>(this));
    return QObject::qt_metacast(_clname);
}

int Dss1ToSip::HardType::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Dss1ToSip::TypeOfNumber> GRegQObj_0;
	Utils::QObjFactoryRegister<Dss1ToSip::NumberingPlan> GRegQObj_1;
	Utils::QObjFactoryRegister<Dss1ToSip::PresentInd> GRegQObj_2;
	Utils::QObjFactoryRegister<Dss1ToSip::ScreeningInd> GRegQObj_3;
	Utils::QObjFactoryRegister<Dss1ToSip::HardType> GRegQObj_4;
}; // end namespace 
QT_END_MOC_NAMESPACE
