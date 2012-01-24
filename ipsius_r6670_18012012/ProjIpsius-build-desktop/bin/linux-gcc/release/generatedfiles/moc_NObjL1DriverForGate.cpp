/****************************************************************************
** Meta object code from reading C++ file 'NObjL1DriverForGate.h'
**
** Created: Tue Jan 24 14:57:15 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Dss1ToSip/NObjL1DriverForGate.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjL1DriverForGate.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dss1ToSip__NObjL1DriverForGate[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       3,   14, // properties
       1,   23, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      31,   31, 0x0009500b,
      46,   41, 0x01095001,
      61,   41, 0x01095001,

 // enums: name, flags, count, data
      31, 0x0,    4,   27,

 // enum data: key, value
      77, uint(Dss1ToSip::NObjL1DriverForGate::State),
      83, uint(Dss1ToSip::NObjL1DriverForGate::All),
      87, uint(Dss1ToSip::NObjL1DriverForGate::Short),
      93, uint(Dss1ToSip::NObjL1DriverForGate::Data),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__NObjL1DriverForGate[] = {
    "Dss1ToSip::NObjL1DriverForGate\0TraceType\0"
    "bool\0HardwareActive\0ActivatedByUser\0"
    "State\0All\0Short\0Data\0"
};

const QMetaObject Dss1ToSip::NObjL1DriverForGate::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__NObjL1DriverForGate,
      qt_meta_data_Dss1ToSip__NObjL1DriverForGate, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::NObjL1DriverForGate::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::NObjL1DriverForGate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::NObjL1DriverForGate::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__NObjL1DriverForGate))
        return static_cast<void*>(const_cast< NObjL1DriverForGate*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjL1DriverForGate*>(this));
    if (!strcmp(_clname, "ISDN::IL2ToDriver"))
        return static_cast< ISDN::IL2ToDriver*>(const_cast< NObjL1DriverForGate*>(this));
    if (!strcmp(_clname, "ObjLink::IObjectLinksHost"))
        return static_cast< ObjLink::IObjectLinksHost*>(const_cast< NObjL1DriverForGate*>(this));
    if (!strcmp(_clname, "ObjLink::IObjectLinkOwner"))
        return static_cast< ObjLink::IObjectLinkOwner*>(const_cast< NObjL1DriverForGate*>(this));
    if (!strcmp(_clname, "IHardwareToL1"))
        return static_cast< IHardwareToL1*>(const_cast< NObjL1DriverForGate*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Dss1ToSip::NObjL1DriverForGate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< TraceType*>(_v) = GetTraceType(); break;
        case 1: *reinterpret_cast< bool*>(_v) = HardwareActive(); break;
        case 2: *reinterpret_cast< bool*>(_v) = ActivatedByUser(); break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: SetTraceType(*reinterpret_cast< TraceType*>(_v)); break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 3;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<Dss1ToSip::NObjL1DriverForGate> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Dss1ToSip::NObjL1DriverForGate> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
