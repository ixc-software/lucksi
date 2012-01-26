/****************************************************************************
** Meta object code from reading C++ file 'NObjSimulLiveSettings.h'
**
** Created: Tue Jan 24 17:49:47 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IsdnTest/NObjSimulLiveSettings.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSimulLiveSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IsdnTest__NObjSimulLiveSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       7,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      36,   32, 0x02095003,
      65,   32, 0x02095003,
      78,   32, 0x02095003,
      94,   32, 0x02095003,
     110,   32, 0x02095003,
     122,   32, 0x02095003,
     134,   32, 0x02095003,

       0        // eod
};

static const char qt_meta_stringdata_IsdnTest__NObjSimulLiveSettings[] = {
    "IsdnTest::NObjSimulLiveSettings\0int\0"
    "PrcentDropedCallInConnection\0TestDuration\0"
    "MinCallInterval\0MaxCallInterval\0"
    "MinTalkTime\0MaxTalkTime\0OutStatistic\0"
};

const QMetaObject IsdnTest::NObjSimulLiveSettings::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_IsdnTest__NObjSimulLiveSettings,
      qt_meta_data_IsdnTest__NObjSimulLiveSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IsdnTest::NObjSimulLiveSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IsdnTest::NObjSimulLiveSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IsdnTest::NObjSimulLiveSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IsdnTest__NObjSimulLiveSettings))
        return static_cast<void*>(const_cast< NObjSimulLiveSettings*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSimulLiveSettings*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int IsdnTest::NObjSimulLiveSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = m_PrcentDropedCallInConnection; break;
        case 1: *reinterpret_cast< int*>(_v) = m_TestDuration; break;
        case 2: *reinterpret_cast< int*>(_v) = m_MinCallInterval; break;
        case 3: *reinterpret_cast< int*>(_v) = m_MaxCallInterval; break;
        case 4: *reinterpret_cast< int*>(_v) = m_MinTalkTime; break;
        case 5: *reinterpret_cast< int*>(_v) = m_MaxTalkTime; break;
        case 6: *reinterpret_cast< int*>(_v) = m_OutStatistic; break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_PrcentDropedCallInConnection = *reinterpret_cast< int*>(_v); break;
        case 1: m_TestDuration = *reinterpret_cast< int*>(_v); break;
        case 2: m_MinCallInterval = *reinterpret_cast< int*>(_v); break;
        case 3: m_MaxCallInterval = *reinterpret_cast< int*>(_v); break;
        case 4: m_MinTalkTime = *reinterpret_cast< int*>(_v); break;
        case 5: m_MaxTalkTime = *reinterpret_cast< int*>(_v); break;
        case 6: m_OutStatistic = *reinterpret_cast< int*>(_v); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<IsdnTest::NObjSimulLiveSettings> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IsdnTest::NObjSimulLiveSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
