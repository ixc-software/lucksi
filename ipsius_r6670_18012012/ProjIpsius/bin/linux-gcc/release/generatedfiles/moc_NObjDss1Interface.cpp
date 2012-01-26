/****************************************************************************
** Meta object code from reading C++ file 'NObjDss1Interface.h'
**
** Created: Tue Jan 24 17:48:34 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Dss1ToSip/NObjDss1Interface.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjDss1Interface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dss1ToSip__NObjDss1Interface[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
      10,   39, // properties
       2,   69, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      34,   30,   29,   29, 0x02,
      62,   48,   29,   29, 0x02,
      99,   91,   29,   29, 0x22,
     123,   91,   29,   29, 0x02,
     156,  146,   29,   29, 0x02,

 // properties: name, type, flags
     185,  181, 0x02095001,
     209,  201, 0x0a095001,
     215,  181, 0x02095001,
     224,  201, 0x0a095001,
     234,  201, 0x0a095001,
     244,  201, 0x0a095001,
     253,  201, 0x0a095001,
     260,  201, 0x0a095003,
     274,  201, 0x0a095003,
     292,  287, 0x01095003,

 // enums: name, flags, count, data
     301, 0x0,    1,   77,
     314, 0x0,    5,   79,

 // enum data: key, value
     330, uint(Dss1ToSip::NObjDss1Interface::Lifo),
     335, uint(Dss1ToSip::NObjDss1Interface::Inactive),
     344, uint(Dss1ToSip::NObjDss1Interface::WaitGlobalSetupResp),
     364, uint(Dss1ToSip::NObjDss1Interface::WaitE1InitResp),
     379, uint(Dss1ToSip::NObjDss1Interface::E1InActive),
     390, uint(Dss1ToSip::NObjDss1Interface::E1Active),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__NObjDss1Interface[] = {
    "Dss1ToSip::NObjDss1Interface\0\0par\0"
    "Enabled(bool)\0pOutput,brief\0"
    "Calls(DRI::ICmdOutput*,bool)\0pOutput\0"
    "Calls(DRI::ICmdOutput*)\0Info(DRI::ICmdOutput*)\0"
    "pAsyncCmd\0LiuInfo(DRI::IAsyncCmd*)\0"
    "int\0InterfaceNumber\0QString\0State\0"
    "DChannel\0BChannels\0FreeOutCh\0FreeInCh\0"
    "BusyCh\0OutBlockingCh\0InBlockingCh\0"
    "bool\0TestMode\0StrategyType\0ConnectionState\0"
    "Lifo\0Inactive\0WaitGlobalSetupResp\0"
    "WaitE1InitResp\0E1InActive\0E1Active\0"
};

const QMetaObject Dss1ToSip::NObjDss1Interface::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__NObjDss1Interface,
      qt_meta_data_Dss1ToSip__NObjDss1Interface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::NObjDss1Interface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::NObjDss1Interface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::NObjDss1Interface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__NObjDss1Interface))
        return static_cast<void*>(const_cast< NObjDss1Interface*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjDss1Interface*>(this));
    if (!strcmp(_clname, "ISDN::IIsdnIntf"))
        return static_cast< ISDN::IIsdnIntf*>(const_cast< NObjDss1Interface*>(this));
    if (!strcmp(_clname, "iCmpExt::ICmpConEvents"))
        return static_cast< iCmpExt::ICmpConEvents*>(const_cast< NObjDss1Interface*>(this));
    if (!strcmp(_clname, "iCmpExt::ICmpChannelCreator"))
        return static_cast< iCmpExt::ICmpChannelCreator*>(const_cast< NObjDss1Interface*>(this));
    if (!strcmp(_clname, "IL1ToHardware"))
        return static_cast< IL1ToHardware*>(const_cast< NObjDss1Interface*>(this));
    if (!strcmp(_clname, "IBoardInfoReq"))
        return static_cast< IBoardInfoReq*>(const_cast< NObjDss1Interface*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Dss1ToSip::NObjDss1Interface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Enabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: Calls((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: Calls((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 3: Info((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 4: LiuInfo((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = InterfaceNumber(); break;
        case 1: *reinterpret_cast< QString*>(_v) = GetStateStr(); break;
        case 2: *reinterpret_cast< int*>(_v) = DChannel(); break;
        case 3: *reinterpret_cast< QString*>(_v) = BChannels(); break;
        case 4: *reinterpret_cast< QString*>(_v) = FreeOutCh(); break;
        case 5: *reinterpret_cast< QString*>(_v) = FreeInCh(); break;
        case 6: *reinterpret_cast< QString*>(_v) = BusyCh(); break;
        case 7: *reinterpret_cast< QString*>(_v) = OutBlockingCh(); break;
        case 8: *reinterpret_cast< QString*>(_v) = InBlockingCh(); break;
        case 9: *reinterpret_cast< bool*>(_v) = m_testMode; break;
        }
        _id -= 10;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 7: OutBlockingCh(*reinterpret_cast< QString*>(_v)); break;
        case 8: InBlockingCh(*reinterpret_cast< QString*>(_v)); break;
        case 9: m_testMode = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 10;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 10;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<Dss1ToSip::NObjDss1Interface> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Dss1ToSip::NObjDss1Interface> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
