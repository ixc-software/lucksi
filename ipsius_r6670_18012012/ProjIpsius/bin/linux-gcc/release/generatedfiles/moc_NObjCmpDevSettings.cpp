/****************************************************************************
** Meta object code from reading C++ file 'NObjCmpDevSettings.h'
**
** Created: Tue Jan 24 17:49:06 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iCmpExt/NObjCmpDevSettings.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjCmpDevSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iCmpExt__NObjCmpDevSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
      17,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      32,   28, 0x02095003,
      50,   45, 0x01095003,
      58,   45, 0x01095003,
      80,   45, 0x01095003,
      94,   28, 0x02095003,
     112,   28, 0x02095003,
     132,   28, 0x02095003,
     150,   28, 0x02095003,
     170,   28, 0x02095003,
     184,   28, 0x02095003,
     200,   28, 0x02095003,
     221,   28, 0x02095003,
     242,   28, 0x02095003,
     260,   28, 0x02095003,
     274,   28, 0x02095003,
     287,   28, 0x02095003,
     296,   45, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_iCmpExt__NObjCmpDevSettings[] = {
    "iCmpExt::NObjCmpDevSettings\0int\0"
    "StartRtpPort\0bool\0UseAlaw\0"
    "TestBodyBaseException\0ThrowFromInit\0"
    "Sport0BlocksCount\0Sport0BlockCapacity\0"
    "Sport1BlocksCount\0Sport1BlockCapacity\0"
    "VoiceSportNum\0VoiceSportChNum\0"
    "RtpMinBufferingDepth\0RtpMaxBufferingDepth\0"
    "RtpUpScalePercent\0RtpQueueDepth\0"
    "RtpDropCount\0EchoTape\0UseInternalEcho\0"
};

const QMetaObject iCmpExt::NObjCmpDevSettings::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iCmpExt__NObjCmpDevSettings,
      qt_meta_data_iCmpExt__NObjCmpDevSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iCmpExt::NObjCmpDevSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iCmpExt::NObjCmpDevSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iCmpExt::NObjCmpDevSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iCmpExt__NObjCmpDevSettings))
        return static_cast<void*>(const_cast< NObjCmpDevSettings*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjCmpDevSettings*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iCmpExt::NObjCmpDevSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = StartRtpPort(); break;
        case 1: *reinterpret_cast< bool*>(_v) = UseAlaw(); break;
        case 2: *reinterpret_cast< bool*>(_v) = TestBodyBaseException(); break;
        case 3: *reinterpret_cast< bool*>(_v) = ThrowFromInit(); break;
        case 4: *reinterpret_cast< int*>(_v) = Sport0BlocksCount(); break;
        case 5: *reinterpret_cast< int*>(_v) = Sport0BlockCapacity(); break;
        case 6: *reinterpret_cast< int*>(_v) = Sport1BlocksCount(); break;
        case 7: *reinterpret_cast< int*>(_v) = Sport1BlockCapacity(); break;
        case 8: *reinterpret_cast< int*>(_v) = VoiceSportNum(); break;
        case 9: *reinterpret_cast< int*>(_v) = VoiceSportChNum(); break;
        case 10: *reinterpret_cast< int*>(_v) = RtpMinBufferingDepth(); break;
        case 11: *reinterpret_cast< int*>(_v) = RtpMaxBufferingDepth(); break;
        case 12: *reinterpret_cast< int*>(_v) = RtpUpScalePercent(); break;
        case 13: *reinterpret_cast< int*>(_v) = RtpQueueDepth(); break;
        case 14: *reinterpret_cast< int*>(_v) = RtpDropCount(); break;
        case 15: *reinterpret_cast< int*>(_v) = m_echoTape; break;
        case 16: *reinterpret_cast< bool*>(_v) = m_innerEcho; break;
        }
        _id -= 17;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: StartRtpPort(*reinterpret_cast< int*>(_v)); break;
        case 1: UseAlaw(*reinterpret_cast< bool*>(_v)); break;
        case 2: TestBodyBaseException(*reinterpret_cast< bool*>(_v)); break;
        case 3: ThrowFromInit(*reinterpret_cast< bool*>(_v)); break;
        case 4: Sport0BlocksCount(*reinterpret_cast< int*>(_v)); break;
        case 5: Sport0BlockCapacity(*reinterpret_cast< int*>(_v)); break;
        case 6: Sport1BlocksCount(*reinterpret_cast< int*>(_v)); break;
        case 7: Sport1BlockCapacity(*reinterpret_cast< int*>(_v)); break;
        case 8: VoiceSportNum(*reinterpret_cast< int*>(_v)); break;
        case 9: VoiceSportChNum(*reinterpret_cast< int*>(_v)); break;
        case 10: RtpMinBufferingDepth(*reinterpret_cast< int*>(_v)); break;
        case 11: RtpMaxBufferingDepth(*reinterpret_cast< int*>(_v)); break;
        case 12: RtpUpScalePercent(*reinterpret_cast< int*>(_v)); break;
        case 13: RtpQueueDepth(*reinterpret_cast< int*>(_v)); break;
        case 14: RtpDropCount(*reinterpret_cast< int*>(_v)); break;
        case 15: m_echoTape = *reinterpret_cast< int*>(_v); break;
        case 16: m_innerEcho = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 17;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 17;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 17;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 17;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 17;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 17;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 17;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<iCmpExt::NObjCmpDevSettings> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iCmpExt::NObjCmpDevSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
