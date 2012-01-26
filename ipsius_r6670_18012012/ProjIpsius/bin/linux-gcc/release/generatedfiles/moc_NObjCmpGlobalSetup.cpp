/****************************************************************************
** Meta object code from reading C++ file 'NObjCmpGlobalSetup.h'
**
** Created: Tue Jan 24 17:49:10 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iCmpExt/NObjCmpGlobalSetup.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjCmpGlobalSetup.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iCmpExt__NObjCmpGlobalSetup[] = {

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
      33,   28, 0x01095003,
      53,   45, 0x0a095003,
      66,   28, 0x01095003,
      85,   28, 0x01095003,
     106,   28, 0x01095003,
     120,   28, 0x01095003,
     136,   28, 0x01095003,
     161,   28, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_iCmpExt__NObjCmpGlobalSetup[] = {
    "iCmpExt::NObjCmpGlobalSetup\0bool\0"
    "DevTraceInd\0QString\0DevTraceHost\0"
    "DevWatchdogEnabled\0DevResetOnFatalError\0"
    "SetupNullEcho\0AsyncDeleteBody\0"
    "AsyncDropConnOnException\0DoSafeRefTest\0"
};

const QMetaObject iCmpExt::NObjCmpGlobalSetup::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iCmpExt__NObjCmpGlobalSetup,
      qt_meta_data_iCmpExt__NObjCmpGlobalSetup, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iCmpExt::NObjCmpGlobalSetup::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iCmpExt::NObjCmpGlobalSetup::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iCmpExt::NObjCmpGlobalSetup::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iCmpExt__NObjCmpGlobalSetup))
        return static_cast<void*>(const_cast< NObjCmpGlobalSetup*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjCmpGlobalSetup*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iCmpExt::NObjCmpGlobalSetup::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_devTraceInd; break;
        case 1: *reinterpret_cast< QString*>(_v) = DevTraceHost(); break;
        case 2: *reinterpret_cast< bool*>(_v) = m_watchdogEnabled; break;
        case 3: *reinterpret_cast< bool*>(_v) = m_resetOnFatalError; break;
        case 4: *reinterpret_cast< bool*>(_v) = m_setupNullEcho; break;
        case 5: *reinterpret_cast< bool*>(_v) = m_asyncDeleteBody; break;
        case 6: *reinterpret_cast< bool*>(_v) = m_asyncDropConnOnException; break;
        case 7: *reinterpret_cast< bool*>(_v) = m_doSafeRefTest; break;
        }
        _id -= 8;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: DevTraceInd(*reinterpret_cast< bool*>(_v)); break;
        case 1: DevTraceHost(*reinterpret_cast< QString*>(_v)); break;
        case 2: WatchdogEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 3: RestartOnError(*reinterpret_cast< bool*>(_v)); break;
        case 4: m_setupNullEcho = *reinterpret_cast< bool*>(_v); break;
        case 5: m_asyncDeleteBody = *reinterpret_cast< bool*>(_v); break;
        case 6: m_asyncDropConnOnException = *reinterpret_cast< bool*>(_v); break;
        case 7: m_doSafeRefTest = *reinterpret_cast< bool*>(_v); break;
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
	DRI::NamedObjectTypeRegister<iCmpExt::NObjCmpGlobalSetup> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iCmpExt::NObjCmpGlobalSetup> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
