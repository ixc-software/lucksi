/****************************************************************************
** Meta object code from reading C++ file 'NObjLapdTestSettings.h'
**
** Created: Tue Jan 24 14:57:57 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IsdnTest/NObjLapdTestSettings.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjLapdTestSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IsdnTest__NObjLapdTestSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       8,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      35,   31, 0x02095003,
      51,   31, 0x02095003,
      71,   66, 0x01095003,
      81,   31, 0x02095003,
      90,   31, 0x02095003,
     108,   31, 0x02095003,
     120,   31, 0x02095003,
     132,   31, 0x02095003,

       0        // eod
};

static const char qt_meta_stringdata_IsdnTest__NObjLapdTestSettings[] = {
    "IsdnTest::NObjLapdTestSettings\0int\0"
    "MaxDurationMsec\0MaxTestTimeout\0bool\0"
    "TraceTest\0DropRate\0DataExchangeCycle\0"
    "MaxIPackets\0MaxUPackets\0MaxIPacketsInDropMode\0"
};

const QMetaObject IsdnTest::NObjLapdTestSettings::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_IsdnTest__NObjLapdTestSettings,
      qt_meta_data_IsdnTest__NObjLapdTestSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IsdnTest::NObjLapdTestSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IsdnTest::NObjLapdTestSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IsdnTest::NObjLapdTestSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IsdnTest__NObjLapdTestSettings))
        return static_cast<void*>(const_cast< NObjLapdTestSettings*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjLapdTestSettings*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int IsdnTest::NObjLapdTestSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = m_MaxDurationMsec; break;
        case 1: *reinterpret_cast< int*>(_v) = m_MaxTestTimeout; break;
        case 2: *reinterpret_cast< bool*>(_v) = m_TracedTest; break;
        case 3: *reinterpret_cast< int*>(_v) = m_DropRate; break;
        case 4: *reinterpret_cast< int*>(_v) = m_DataExchangeCycle; break;
        case 5: *reinterpret_cast< int*>(_v) = m_MaxIPackets; break;
        case 6: *reinterpret_cast< int*>(_v) = m_MaxUPackets; break;
        case 7: *reinterpret_cast< int*>(_v) = m_MaxIPacketsInDropMode; break;
        }
        _id -= 8;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_MaxDurationMsec = *reinterpret_cast< int*>(_v); break;
        case 1: m_MaxTestTimeout = *reinterpret_cast< int*>(_v); break;
        case 2: m_TracedTest = *reinterpret_cast< bool*>(_v); break;
        case 3: m_DropRate = *reinterpret_cast< int*>(_v); break;
        case 4: m_DataExchangeCycle = *reinterpret_cast< int*>(_v); break;
        case 5: m_MaxIPackets = *reinterpret_cast< int*>(_v); break;
        case 6: m_MaxUPackets = *reinterpret_cast< int*>(_v); break;
        case 7: m_MaxIPacketsInDropMode = *reinterpret_cast< int*>(_v); break;
        }
        _id -= 8;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 8;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<IsdnTest::NObjLapdTestSettings> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IsdnTest::NObjLapdTestSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
