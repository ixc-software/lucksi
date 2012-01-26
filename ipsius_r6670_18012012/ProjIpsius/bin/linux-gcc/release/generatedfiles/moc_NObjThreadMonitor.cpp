/****************************************************************************
** Meta object code from reading C++ file 'NObjThreadMonitor.h'
**
** Created: Tue Jan 24 17:48:02 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Domain/NObjThreadMonitor.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjThreadMonitor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Domain__NObjThreadMonitor[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       6,   49, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      36,   27,   26,   26, 0x02,
      60,   26,   26,   26, 0x02,
     109,   83,   26,   26, 0x02,
     133,   26,   26,   26, 0x02,
     178,  152,   26,   26, 0x02,
     207,   26,   26,   26, 0x02,
     231,   26,   26,   26, 0x02,

 // properties: name, type, flags
     243,  239, 0x02095001,
     259,  239, 0x02095001,
     276,  239, 0x02095001,
     288,  239, 0x02095001,
     301,  239, 0x02095001,
     322,  314, 0x0a095001,

       0        // eod
};

static const char qt_meta_stringdata_Domain__NObjThreadMonitor[] = {
    "Domain::NObjThreadMonitor\0\0msgCount\0"
    "SetMaxMsgQueueSize(int)\0ResetMaxMsgQueueSize()\0"
    "timerLagCount,maxTimerLag\0"
    "SetMaxTimerLag(int,int)\0ResetMaxTimerLag()\0"
    "msgGrowCount,msgGrowLimit\0"
    "SetMsgGrowDetection(int,int)\0"
    "ResetMsgGrowDetection()\0Reset()\0int\0"
    "MaxMsgQueueSize\0MaxTimerLagCount\0"
    "MaxTimerLag\0MsgGrowCount\0MsgGrowLimit\0"
    "QString\0Statistic\0"
};

const QMetaObject Domain::NObjThreadMonitor::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Domain__NObjThreadMonitor,
      qt_meta_data_Domain__NObjThreadMonitor, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Domain::NObjThreadMonitor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Domain::NObjThreadMonitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Domain::NObjThreadMonitor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Domain__NObjThreadMonitor))
        return static_cast<void*>(const_cast< NObjThreadMonitor*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjThreadMonitor*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Domain::NObjThreadMonitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SetMaxMsgQueueSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: ResetMaxMsgQueueSize(); break;
        case 2: SetMaxTimerLag((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: ResetMaxTimerLag(); break;
        case 4: SetMsgGrowDetection((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: ResetMsgGrowDetection(); break;
        case 6: Reset(); break;
        default: ;
        }
        _id -= 7;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = MaxMsgQueueSize(); break;
        case 1: *reinterpret_cast< int*>(_v) = MaxTimerLagCount(); break;
        case 2: *reinterpret_cast< int*>(_v) = MaxTimerLag(); break;
        case 3: *reinterpret_cast< int*>(_v) = MsgGrowCount(); break;
        case 4: *reinterpret_cast< int*>(_v) = MsgGrowLimit(); break;
        case 5: *reinterpret_cast< QString*>(_v) = Statistic(); break;
        }
        _id -= 6;
    } else if (_c == QMetaObject::WriteProperty) {
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
	DRI::NamedObjectTypeRegister<Domain::NObjThreadMonitor> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Domain::NObjThreadMonitor> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
