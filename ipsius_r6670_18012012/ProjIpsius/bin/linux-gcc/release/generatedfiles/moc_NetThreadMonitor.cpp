/****************************************************************************
** Meta object code from reading C++ file 'NetThreadMonitor.h'
**
** Created: Tue Jan 24 17:48:23 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iNet/NetThreadMonitor.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NetThreadMonitor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iNet__NetThreadMonitor[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      32,   24,   23,   23, 0x05,
      54,   23,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      72,   23,   23,   23, 0x08,
      84,   24,   23,   23, 0x08,
     104,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_iNet__NetThreadMonitor[] = {
    "iNet::NetThreadMonitor\0\0timeout\0"
    "StartTimerSignal(int)\0StopTimerSignal()\0"
    "TimerDone()\0StartTimerSlot(int)\0"
    "StopTimerSlot()\0"
};

const QMetaObject iNet::NetThreadMonitor::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_iNet__NetThreadMonitor,
      qt_meta_data_iNet__NetThreadMonitor, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iNet::NetThreadMonitor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iNet::NetThreadMonitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iNet::NetThreadMonitor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iNet__NetThreadMonitor))
        return static_cast<void*>(const_cast< NetThreadMonitor*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< NetThreadMonitor*>(this));
    return QObject::qt_metacast(_clname);
}

int iNet::NetThreadMonitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: StartTimerSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: StopTimerSignal(); break;
        case 2: TimerDone(); break;
        case 3: StartTimerSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: StopTimerSlot(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void iNet::NetThreadMonitor::StartTimerSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void iNet::NetThreadMonitor::StopTimerSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iNet::NetThreadMonitor> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
