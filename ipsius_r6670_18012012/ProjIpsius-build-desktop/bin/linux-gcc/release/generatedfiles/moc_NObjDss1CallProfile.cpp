/****************************************************************************
** Meta object code from reading C++ file 'NObjDss1CallProfile.h'
**
** Created: Tue Jan 24 14:57:08 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Dss1ToSip/NObjDss1CallProfile.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjDss1CallProfile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dss1ToSip__NObjDss1InCallProfile[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       3,   14, // properties
       2,   23, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      33,   33, 0x0009500b,
      49,   49, 0x0009500b,
      76,   68, 0x0a095003,

 // enums: name, flags, count, data
      33, 0x0,    2,   31,
      49, 0x0,    2,   35,

 // enum data: key, value
      88, uint(Dss1ToSip::NObjDss1InCallProfile::From),
      93, uint(Dss1ToSip::NObjDss1InCallProfile::Contact),
     101, uint(Dss1ToSip::NObjDss1InCallProfile::UsePilotNumber),
     116, uint(Dss1ToSip::NObjDss1InCallProfile::ReleaseCall),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__NObjDss1InCallProfile[] = {
    "Dss1ToSip::NObjDss1InCallProfile\0"
    "CallingAddrType\0CallingAddrNoDigit\0"
    "QString\0PilotNumber\0From\0Contact\0"
    "UsePilotNumber\0ReleaseCall\0"
};

const QMetaObject Dss1ToSip::NObjDss1InCallProfile::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__NObjDss1InCallProfile,
      qt_meta_data_Dss1ToSip__NObjDss1InCallProfile, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::NObjDss1InCallProfile::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::NObjDss1InCallProfile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::NObjDss1InCallProfile::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__NObjDss1InCallProfile))
        return static_cast<void*>(const_cast< NObjDss1InCallProfile*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjDss1InCallProfile*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Dss1ToSip::NObjDss1InCallProfile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< CallingAddrType*>(_v) = m_callingAddrType; break;
        case 1: *reinterpret_cast< CallingAddrNoDigit*>(_v) = m_callingAddrNoDigit; break;
        case 2: *reinterpret_cast< QString*>(_v) = PilotNumber(); break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_callingAddrType = *reinterpret_cast< CallingAddrType*>(_v); break;
        case 1: m_callingAddrNoDigit = *reinterpret_cast< CallingAddrNoDigit*>(_v); break;
        case 2: PilotNumber(*reinterpret_cast< QString*>(_v)); break;
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
	DRI::NamedObjectTypeRegister<Dss1ToSip::NObjDss1InCallProfile> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Dss1ToSip::NObjDss1InCallProfile> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
