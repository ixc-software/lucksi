/****************************************************************************
** Meta object code from reading C++ file 'UdpSocketImpl.h'
**
** Created: Tue Jan 24 14:56:58 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iNet/UdpSocketImpl.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UdpSocketImpl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iNet__UdpSocket__Impl[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      33,   23,   22,   22, 0x05,
     107,  102,   22,   22, 0x05,
     159,   22,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     215,   23,   22,   22, 0x08,
     282,  102,   22,   22, 0x08,
     332,   22,   22,   22, 0x08,
     362,  350,   22,   22, 0x08,
     403,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_iNet__UdpSocket__Impl[] = {
    "iNet::UdpSocket::Impl\0\0lock,host\0"
    "BindSignal(boost::shared_ptr<Utils::ThreadSyncEvent>,Utils::HostInf)\0"
    "data\0SendDataSignal(boost::shared_ptr<iNet::SocketData>)\0"
    "DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent>)\0"
    "BindSlot(boost::shared_ptr<Utils::ThreadSyncEvent>,Utils::HostInf)\0"
    "SendDataSlot(boost::shared_ptr<iNet::SocketData>)\0"
    "ReceiveDataSlot()\0socketError\0"
    "ErrorOccur(QAbstractSocket::SocketError)\0"
    "DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent>)\0"
};

const QMetaObject iNet::UdpSocket::Impl::staticMetaObject = {
    { &QUdpSocket::staticMetaObject, qt_meta_stringdata_iNet__UdpSocket__Impl,
      qt_meta_data_iNet__UdpSocket__Impl, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iNet::UdpSocket::Impl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iNet::UdpSocket::Impl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iNet::UdpSocket::Impl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iNet__UdpSocket__Impl))
        return static_cast<void*>(const_cast< Impl*>(this));
    return QUdpSocket::qt_metacast(_clname);
}

int iNet::UdpSocket::Impl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QUdpSocket::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: BindSignal((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1])),(*reinterpret_cast< const Utils::HostInf(*)>(_a[2]))); break;
        case 1: SendDataSignal((*reinterpret_cast< boost::shared_ptr<iNet::SocketData>(*)>(_a[1]))); break;
        case 2: DeleteSignal((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1]))); break;
        case 3: BindSlot((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1])),(*reinterpret_cast< const Utils::HostInf(*)>(_a[2]))); break;
        case 4: SendDataSlot((*reinterpret_cast< boost::shared_ptr<iNet::SocketData>(*)>(_a[1]))); break;
        case 5: ReceiveDataSlot(); break;
        case 6: ErrorOccur((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 7: DeleteSlot((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void iNet::UdpSocket::Impl::BindSignal(boost::shared_ptr<Utils::ThreadSyncEvent> _t1, const Utils::HostInf _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void iNet::UdpSocket::Impl::SendDataSignal(boost::shared_ptr<iNet::SocketData> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void iNet::UdpSocket::Impl::DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
