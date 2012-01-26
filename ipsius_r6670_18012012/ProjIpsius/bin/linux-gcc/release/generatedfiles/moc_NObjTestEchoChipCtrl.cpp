/****************************************************************************
** Meta object code from reading C++ file 'NObjTestEchoChipCtrl.h'
**
** Created: Tue Jan 24 17:50:24 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/TestRealEcho/NObjTestEchoChipCtrl.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjTestEchoChipCtrl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TestRealEcho__NObjEchoDataInit[] = {

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
      36,   31, 0x01095103,
      52,   44, 0x0a095103,
      64,   31, 0x01095103,

       0        // eod
};

static const char qt_meta_stringdata_TestRealEcho__NObjEchoDataInit[] = {
    "TestRealEcho::NObjEchoDataInit\0bool\0"
    "UseAlaw\0QString\0VoiceChMask\0T1Mode\0"
};

const QMetaObject TestRealEcho::NObjEchoDataInit::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_TestRealEcho__NObjEchoDataInit,
      qt_meta_data_TestRealEcho__NObjEchoDataInit, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TestRealEcho::NObjEchoDataInit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TestRealEcho::NObjEchoDataInit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TestRealEcho::NObjEchoDataInit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TestRealEcho__NObjEchoDataInit))
        return static_cast<void*>(const_cast< NObjEchoDataInit*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjEchoDataInit*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int TestRealEcho::NObjEchoDataInit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = getUseAlaw(); break;
        case 1: *reinterpret_cast< QString*>(_v) = getVoiceChMask(); break;
        case 2: *reinterpret_cast< bool*>(_v) = getT1Mode(); break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setUseAlaw(*reinterpret_cast< bool*>(_v)); break;
        case 1: setVoiceChMask(*reinterpret_cast< QString*>(_v)); break;
        case 2: setT1Mode(*reinterpret_cast< bool*>(_v)); break;
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
static const uint qt_meta_data_TestRealEcho__NObjTestEchoChipCtrl[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       4,   19, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      54,   36,   35,   35, 0x02,

 // properties: name, type, flags
      83,   79, 0x02095003,
      87,   79, 0x02095003,
      96,   79, 0x02095003,
     110,  105, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_TestRealEcho__NObjTestEchoChipCtrl[] = {
    "TestRealEcho::NObjTestEchoChipCtrl\0\0"
    "pAsyncCmd,timeout\0Run(DRI::IAsyncCmd*,int)\0"
    "int\0COM\0BodeRate\0EchoTaps\0bool\0"
    "TestErrorReport\0"
};

const QMetaObject TestRealEcho::NObjTestEchoChipCtrl::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_TestRealEcho__NObjTestEchoChipCtrl,
      qt_meta_data_TestRealEcho__NObjTestEchoChipCtrl, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TestRealEcho::NObjTestEchoChipCtrl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TestRealEcho::NObjTestEchoChipCtrl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TestRealEcho::NObjTestEchoChipCtrl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TestRealEcho__NObjTestEchoChipCtrl))
        return static_cast<void*>(const_cast< NObjTestEchoChipCtrl*>(this));
    if (!strcmp(_clname, "IEchoChipCtrlOwner"))
        return static_cast< IEchoChipCtrlOwner*>(const_cast< NObjTestEchoChipCtrl*>(this));
    if (!strcmp(_clname, "EchoApp::IMainRecvSide"))
        return static_cast< EchoApp::IMainRecvSide*>(const_cast< NObjTestEchoChipCtrl*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int TestRealEcho::NObjTestEchoChipCtrl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Run((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = m_com; break;
        case 1: *reinterpret_cast< int*>(_v) = m_boudRate; break;
        case 2: *reinterpret_cast< int*>(_v) = m_echoTapsForAll; break;
        case 3: *reinterpret_cast< bool*>(_v) = m_testErrorReport; break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_com = *reinterpret_cast< int*>(_v); break;
        case 1: m_boudRate = *reinterpret_cast< int*>(_v); break;
        case 2: m_echoTapsForAll = *reinterpret_cast< int*>(_v); break;
        case 3: m_testErrorReport = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<TestRealEcho::NObjEchoDataInit> GReg_0;
	DRI::NamedObjectTypeRegister<TestRealEcho::NObjTestEchoChipCtrl> GReg_1;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<TestRealEcho::NObjEchoDataInit> GRegQObj_0;
	Utils::QObjFactoryRegister<TestRealEcho::NObjTestEchoChipCtrl> GRegQObj_1;
}; // end namespace 
QT_END_MOC_NAMESPACE
