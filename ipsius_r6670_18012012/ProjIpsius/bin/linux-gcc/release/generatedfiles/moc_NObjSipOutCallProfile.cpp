/****************************************************************************
** Meta object code from reading C++ file 'NObjSipOutCallProfile.h'
**
** Created: Tue Jan 24 17:50:03 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/NObjSipOutCallProfile.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSipOutCallProfile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__NObjSipOutCallProfile[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       3,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      33,   28, 0x01095003,
      56,   28, 0x01095003,
      76,   28, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_iSip__NObjSipOutCallProfile[] = {
    "iSip::NObjSipOutCallProfile\0bool\0"
    "UseSourceNameInContact\0UseSourceNameInFrom\0"
    "UseSourceNameAsDisplay\0"
};

const QMetaObject iSip::NObjSipOutCallProfile::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iSip__NObjSipOutCallProfile,
      qt_meta_data_iSip__NObjSipOutCallProfile, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::NObjSipOutCallProfile::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::NObjSipOutCallProfile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::NObjSipOutCallProfile::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__NObjSipOutCallProfile))
        return static_cast<void*>(const_cast< NObjSipOutCallProfile*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSipOutCallProfile*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iSip::NObjSipOutCallProfile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_useSourceNameInContact; break;
        case 1: *reinterpret_cast< bool*>(_v) = m_useSourceNameInFrom; break;
        case 2: *reinterpret_cast< bool*>(_v) = m_useSourceNameAsDisplay; break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: UseSourceNameInContact(*reinterpret_cast< bool*>(_v)); break;
        case 1: UseSourceNameInFrom(*reinterpret_cast< bool*>(_v)); break;
        case 2: m_useSourceNameAsDisplay = *reinterpret_cast< bool*>(_v); break;
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
	DRI::NamedObjectTypeRegister<iSip::NObjSipOutCallProfile> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iSip::NObjSipOutCallProfile> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
