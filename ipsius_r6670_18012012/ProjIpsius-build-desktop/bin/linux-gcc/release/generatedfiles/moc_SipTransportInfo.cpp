/****************************************************************************
** Meta object code from reading C++ file 'SipTransportInfo.h'
**
** Created: Tue Jan 24 14:58:28 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/SipTransportInfo.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SipTransportInfo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__SipTransportInfo[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       2,   14, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // enums: name, flags, count, data
      23, 0x0,    3,   22,
      28, 0x0,    3,   28,

 // enum data: key, value
      38, uint(iSip::SipTransportInfo::UnknownTransport),
      55, uint(iSip::SipTransportInfo::Udp),
      59, uint(iSip::SipTransportInfo::Tcp),
      63, uint(iSip::SipTransportInfo::UnknownIpVersion),
      80, uint(iSip::SipTransportInfo::Ip4),
      84, uint(iSip::SipTransportInfo::Ip6),

       0        // eod
};

static const char qt_meta_stringdata_iSip__SipTransportInfo[] = {
    "iSip::SipTransportInfo\0Type\0IpVersion\0"
    "UnknownTransport\0Udp\0Tcp\0UnknownIpVersion\0"
    "Ip4\0Ip6\0"
};

const QMetaObject iSip::SipTransportInfo::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_iSip__SipTransportInfo,
      qt_meta_data_iSip__SipTransportInfo, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::SipTransportInfo::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::SipTransportInfo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::SipTransportInfo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__SipTransportInfo))
        return static_cast<void*>(const_cast< SipTransportInfo*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< SipTransportInfo*>(this));
    return QObject::qt_metacast(_clname);
}

int iSip::SipTransportInfo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iSip::SipTransportInfo> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
