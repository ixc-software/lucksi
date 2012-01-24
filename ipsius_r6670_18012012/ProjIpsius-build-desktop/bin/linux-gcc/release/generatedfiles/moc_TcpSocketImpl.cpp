/****************************************************************************
** Meta object code from reading C++ file 'TcpSocketImpl.h'
**
** Created: Tue Jan 24 14:56:57 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iNet/TcpSocketImpl.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TcpSocketImpl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iNet__TcpSocket__Impl[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   23,   22,   22, 0x05,
      60,   55,   22,   22, 0x05,
      96,   22,   22,   22, 0x05,
     123,   22,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     179,   55,   22,   22, 0x08,
     213,   22,   22,   22, 0x08,
     229,   23,   22,   22, 0x08,
     254,   22,   22,   22, 0x08,
     268,   22,   22,   22, 0x08,
     293,   22,   22,   22, 0x08,
     324,  312,   22,   22, 0x08,
     371,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_iNet__TcpSocket__Impl[] = {
    "iNet::TcpSocket::Impl\0\0data\0"
    "SendDataSignal(QByteArray)\0host\0"
    "ConnectToHostSignal(Utils::HostInf)\0"
    "DisconnectFromHostSignal()\0"
    "DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent>)\0"
    "ConnectToHostSlot(Utils::HostInf)\0"
    "ConnectedSlot()\0SendDataSlot(QByteArray)\0"
    "ReceiveData()\0DisconnectFromHostSlot()\0"
    "DisconnectedSlot()\0socketError\0"
    "SocketErrorOccur(QAbstractSocket::SocketError)\0"
    "DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent>)\0"
};

const QMetaObject iNet::TcpSocket::Impl::staticMetaObject = {
    { &QTcpSocket::staticMetaObject, qt_meta_stringdata_iNet__TcpSocket__Impl,
      qt_meta_data_iNet__TcpSocket__Impl, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iNet::TcpSocket::Impl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iNet::TcpSocket::Impl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iNet::TcpSocket::Impl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iNet__TcpSocket__Impl))
        return static_cast<void*>(const_cast< Impl*>(this));
    if (!strcmp(_clname, "ITcpSocket"))
        return static_cast< ITcpSocket*>(const_cast< Impl*>(this));
    return QTcpSocket::qt_metacast(_clname);
}

int iNet::TcpSocket::Impl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpSocket::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SendDataSignal((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 1: ConnectToHostSignal((*reinterpret_cast< Utils::HostInf(*)>(_a[1]))); break;
        case 2: DisconnectFromHostSignal(); break;
        case 3: DeleteSignal((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1]))); break;
        case 4: ConnectToHostSlot((*reinterpret_cast< const Utils::HostInf(*)>(_a[1]))); break;
        case 5: ConnectedSlot(); break;
        case 6: SendDataSlot((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 7: ReceiveData(); break;
        case 8: DisconnectFromHostSlot(); break;
        case 9: DisconnectedSlot(); break;
        case 10: SocketErrorOccur((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 11: DeleteSlot((*reinterpret_cast< boost::shared_ptr<Utils::ThreadSyncEvent>(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void iNet::TcpSocket::Impl::SendDataSignal(QByteArray _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void iNet::TcpSocket::Impl::ConnectToHostSignal(Utils::HostInf _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void iNet::TcpSocket::Impl::DisconnectFromHostSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void iNet::TcpSocket::Impl::DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
