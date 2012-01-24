/****************************************************************************
** Meta object code from reading C++ file 'NObjNetworkSettings.h'
**
** Created: Tue Jan 24 14:55:54 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfEmul/NObjNetworkSettings.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjNetworkSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfEmul__NObjNetworkSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       5,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      33,   28, 0x01095003,
      46,   38, 0x0a095003,
      49,   38, 0x0a095003,
      57,   38, 0x0a095003,
      62,   38, 0x0a095003,

       0        // eod
};

static const char qt_meta_stringdata_BfEmul__NObjNetworkSettings[] = {
    "BfEmul::NObjNetworkSettings\0bool\0DHCP\0"
    "QString\0Ip\0Gateway\0Mask\0Mac\0"
};

const QMetaObject BfEmul::NObjNetworkSettings::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfEmul__NObjNetworkSettings,
      qt_meta_data_BfEmul__NObjNetworkSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfEmul::NObjNetworkSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfEmul::NObjNetworkSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfEmul::NObjNetworkSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfEmul__NObjNetworkSettings))
        return static_cast<void*>(const_cast< NObjNetworkSettings*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjNetworkSettings*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfEmul::NObjNetworkSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = DHCP(); break;
        case 1: *reinterpret_cast< QString*>(_v) = m_ip; break;
        case 2: *reinterpret_cast< QString*>(_v) = m_gateway; break;
        case 3: *reinterpret_cast< QString*>(_v) = m_mask; break;
        case 4: *reinterpret_cast< QString*>(_v) = m_mac; break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_useDHCP = *reinterpret_cast< bool*>(_v); break;
        case 1: Ip(*reinterpret_cast< QString*>(_v)); break;
        case 2: Gateway(*reinterpret_cast< QString*>(_v)); break;
        case 3: Mask(*reinterpret_cast< QString*>(_v)); break;
        case 4: m_mac = *reinterpret_cast< QString*>(_v); break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 5;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<BfEmul::NObjNetworkSettings> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfEmul::NObjNetworkSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
