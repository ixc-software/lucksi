/****************************************************************************
** Meta object code from reading C++ file 'NObjCheckGenProfile.h'
**
** Created: Tue Jan 24 14:58:36 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/TestRtpWithSip/NObjCheckGenProfile.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjCheckGenProfile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TestRtpWithSip__NObjCheckGenProfile[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       6,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      41,   36, 0x01095003,
      53,   49, 0x02095003,
      66,   49, 0x02095003,
      82,   49, 0x02095003,
      87,   49, 0x02095003,
      93,   36, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_TestRtpWithSip__NObjCheckGenProfile[] = {
    "TestRtpWithSip::NObjCheckGenProfile\0"
    "bool\0Enabled\0int\0AnalisSample\0"
    "CountSkipSample\0Freq\0Power\0UseAlaw\0"
};

const QMetaObject TestRtpWithSip::NObjCheckGenProfile::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_TestRtpWithSip__NObjCheckGenProfile,
      qt_meta_data_TestRtpWithSip__NObjCheckGenProfile, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TestRtpWithSip::NObjCheckGenProfile::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TestRtpWithSip::NObjCheckGenProfile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TestRtpWithSip::NObjCheckGenProfile::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TestRtpWithSip__NObjCheckGenProfile))
        return static_cast<void*>(const_cast< NObjCheckGenProfile*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjCheckGenProfile*>(this));
    if (!strcmp(_clname, "CheckGenProfile"))
        return static_cast< CheckGenProfile*>(const_cast< NObjCheckGenProfile*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int TestRtpWithSip::NObjCheckGenProfile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = IsEnabled(); break;
        case 1: *reinterpret_cast< int*>(_v) = m_countAnalisSample; break;
        case 2: *reinterpret_cast< int*>(_v) = m_countSkipSample; break;
        case 3: *reinterpret_cast< int*>(_v) = m_freq; break;
        case 4: *reinterpret_cast< int*>(_v) = m_power; break;
        case 5: *reinterpret_cast< bool*>(_v) = m_useAlaw; break;
        }
        _id -= 6;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_turnOn = *reinterpret_cast< bool*>(_v); break;
        case 1: m_countAnalisSample = *reinterpret_cast< int*>(_v); break;
        case 2: m_countSkipSample = *reinterpret_cast< int*>(_v); break;
        case 3: m_freq = *reinterpret_cast< int*>(_v); break;
        case 4: m_power = *reinterpret_cast< int*>(_v); break;
        case 5: m_useAlaw = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 6;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 6;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<TestRtpWithSip::NObjCheckGenProfile> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<TestRtpWithSip::NObjCheckGenProfile> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
