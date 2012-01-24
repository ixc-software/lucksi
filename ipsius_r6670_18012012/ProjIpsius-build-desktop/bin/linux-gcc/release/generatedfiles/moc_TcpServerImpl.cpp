/****************************************************************************
** Meta object code from reading C++ file 'TcpServerImpl.h'
**
** Created: Tue Jan 24 14:56:56 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iNet/TcpServerImpl.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TcpServerImpl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iNet__TcpServer__Impl[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      33,   23,   22,   22, 0x05,
     104,   22,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     160,   23,   22,   22, 0x08,
     229,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_iNet__TcpServer__Impl[] = {
    "iNet::TcpServer::Impl\0\0lock,host\0"
    "ListenSignal(boost::shared_ptr<Utils::ThreadSyncEvent>,Utils::HostInf)\0"
    "DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent>)\0"
    "ListenSlot(boost::shared_ptr<Utils::ThreadSyncEvent>,Utils::HostInf)\0"
    "DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent>)\0"
};

const QMetaObject iNet::TcpServer::Impl::staticMetaObject = {
    { &QTcpServer::staticMetaObject, qt_meta_stringdata_iNet__TcpServer__Impl,
      qt_meta_data_iNet__TcpServer__Impl, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iNet::TcpServer::Impl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iNet::TcpServer::Impl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iNet::TcpServer::Impl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iNet__TcpServer__Impl))
        return static_cast<void*>(const_cast< Impl*>(this));
    return QTcpServer::qt_metacast(_clname);
}

int iNet::TcpServer::Impl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpServer::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: ListenSignal((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1])),(*reinterpret_cast< const Utils::HostInf(*)>(_a[2]))); break;
        case 1: DeleteSignal((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1]))); break;
        case 2: ListenSlot((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1])),(*reinterpret_cast< const Utils::HostInf(*)>(_a[2]))); break;
        case 3: DeleteSlot((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void iNet::TcpServer::Impl::ListenSignal(boost::shared_ptr<Utils::ThreadSyncEvent> _t1, const Utils::HostInf _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void iNet::TcpServer::Impl::DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
