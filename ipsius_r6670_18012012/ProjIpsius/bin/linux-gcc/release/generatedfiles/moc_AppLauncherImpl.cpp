/****************************************************************************
** Meta object code from reading C++ file 'AppLauncherImpl.h'
**
** Created: Tue Jan 24 17:47:39 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IpsiusService/AppLauncherImpl.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AppLauncherImpl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IpsiusService__AppThreadLauncher[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_IpsiusService__AppThreadLauncher[] = {
    "IpsiusService::AppThreadLauncher\0"
};

const QMetaObject IpsiusService::AppThreadLauncher::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_IpsiusService__AppThreadLauncher,
      qt_meta_data_IpsiusService__AppThreadLauncher, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IpsiusService::AppThreadLauncher::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IpsiusService::AppThreadLauncher::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IpsiusService::AppThreadLauncher::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IpsiusService__AppThreadLauncher))
        return static_cast<void*>(const_cast< AppThreadLauncher*>(this));
    return QThread::qt_metacast(_clname);
}

int IpsiusService::AppThreadLauncher::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_IpsiusService__AppLauncherImpl[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      40,   32,   31,   31, 0x05,

 // slots: signature, parameters, type, tag, flags
      69,   31,   31,   31, 0x08,
      83,   31,   31,   31, 0x08,
      94,   32,   31,   31, 0x08,
     135,  122,   31,   31, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_IpsiusService__AppLauncherImpl[] = {
    "IpsiusService::AppLauncherImpl\0\0fRunGui\0"
    "RunGuiSig(RunInMainThreadFn)\0OnRunDomain()\0"
    "OnFinish()\0OnRunGui(RunInMainThreadFn)\0"
    "fRunGui,lock\0"
    "RunAndUlock(RunInMainThreadFn,boost::shared_ptr<Utils::ThreadSyncEvent"
    ">)\0"
};

const QMetaObject IpsiusService::AppLauncherImpl::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_IpsiusService__AppLauncherImpl,
      qt_meta_data_IpsiusService__AppLauncherImpl, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IpsiusService::AppLauncherImpl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IpsiusService::AppLauncherImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IpsiusService::AppLauncherImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IpsiusService__AppLauncherImpl))
        return static_cast<void*>(const_cast< AppLauncherImpl*>(this));
    return QObject::qt_metacast(_clname);
}

int IpsiusService::AppLauncherImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: RunGuiSig((*reinterpret_cast< RunInMainThreadFn(*)>(_a[1]))); break;
        case 1: OnRunDomain(); break;
        case 2: OnFinish(); break;
        case 3: OnRunGui((*reinterpret_cast< RunInMainThreadFn(*)>(_a[1]))); break;
        case 4: RunAndUlock((*reinterpret_cast< RunInMainThreadFn(*)>(_a[1])),(*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void IpsiusService::AppLauncherImpl::RunGuiSig(RunInMainThreadFn _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IpsiusService::AppThreadLauncher> GRegQObj_0;
	Utils::QObjFactoryRegister<IpsiusService::AppLauncherImpl> GRegQObj_1;
}; // end namespace 
QT_END_MOC_NAMESPACE
