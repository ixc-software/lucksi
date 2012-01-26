/****************************************************************************
** Meta object code from reading C++ file 'NObjCmpConnection.h'
**
** Created: Tue Jan 24 17:49:02 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iCmpExt/NObjCmpConnection.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjCmpConnection.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iCmpExt__NObjCmpConnection[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       9,   34, // properties
       1,   61, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      38,   28,   27,   27, 0x02,
      65,   28,   27,   27, 0x02,
     115,   90,   27,   27, 0x02,
     174,  153,   27,   27, 0x02,

 // properties: name, type, flags
     218,  210, 0x0a095001,
     228,  210, 0x0a095001,
     238,  234, 0x02095001,
     254,  210, 0x0a095001,
     269,  210, 0x0a095001,
     286,  210, 0x0a095001,
     335,  301, 0x0009500b,
     351,  234, 0x02095003,
     371,  234, 0x02095003,

 // enums: name, flags, count, data
     392, 0x0,    4,   65,

 // enum data: key, value
     401, uint(iCmpExt::NObjCmpConnection::Disabled),
     410, uint(iCmpExt::NObjCmpConnection::Allocating),
     421, uint(iCmpExt::NObjCmpConnection::Connecting),
     432, uint(iCmpExt::NObjCmpConnection::Active),

       0        // eod
};

static const char qt_meta_stringdata_iCmpExt__NObjCmpConnection[] = {
    "iCmpExt::NObjCmpConnection\0\0pAsyncCmd\0"
    "StateInfo(DRI::IAsyncCmd*)\0"
    "TdmInfo(DRI::IAsyncCmd*)\0"
    "pAsyncCmd,chNum,lastCall\0"
    "ChannelInfo(DRI::IAsyncCmd*,int,bool)\0"
    "pAsyncCmd,confHandle\0"
    "ConferenceInfo(DRI::IAsyncCmd*,int)\0"
    "QString\0NameBoard\0State\0int\0CountActivation\0"
    "LastActivation\0LastDeactivation\0"
    "LastBoardError\0iCmpExt::ChDataCaptureMode::Value\0"
    "DataCaptureMode\0MaxDataCaptureCount\0"
    "CurrDataCaptureCount\0CmpState\0Disabled\0"
    "Allocating\0Connecting\0Active\0"
};

#ifdef Q_NO_DATA_RELOCATION
static const QMetaObjectAccessor qt_meta_extradata_iCmpExt__NObjCmpConnection[] = {
        iCmpExt::ChDataCaptureMode::getStaticMetaObject,
#else
static const QMetaObject *qt_meta_extradata_iCmpExt__NObjCmpConnection[] = {
        &iCmpExt::ChDataCaptureMode::staticMetaObject,
#endif //Q_NO_DATA_RELOCATION
    0
};

static const QMetaObjectExtraData qt_meta_extradata2_iCmpExt__NObjCmpConnection = {
    qt_meta_extradata_iCmpExt__NObjCmpConnection, 0 
};

const QMetaObject iCmpExt::NObjCmpConnection::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iCmpExt__NObjCmpConnection,
      qt_meta_data_iCmpExt__NObjCmpConnection, &qt_meta_extradata2_iCmpExt__NObjCmpConnection }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iCmpExt::NObjCmpConnection::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iCmpExt::NObjCmpConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iCmpExt::NObjCmpConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iCmpExt__NObjCmpConnection))
        return static_cast<void*>(const_cast< NObjCmpConnection*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjCmpConnection*>(this));
    if (!strcmp(_clname, "IpsiusService::IAllocBoard"))
        return static_cast< IpsiusService::IAllocBoard*>(const_cast< NObjCmpConnection*>(this));
    if (!strcmp(_clname, "ICmpConnection"))
        return static_cast< ICmpConnection*>(const_cast< NObjCmpConnection*>(this));
    if (!strcmp(_clname, "SBProtoExt::ISbpConnectionEvents"))
        return static_cast< SBProtoExt::ISbpConnectionEvents*>(const_cast< NObjCmpConnection*>(this));
    if (!strcmp(_clname, "iCmp::IPcSideEventRecv"))
        return static_cast< iCmp::IPcSideEventRecv*>(const_cast< NObjCmpConnection*>(this));
    if (!strcmp(_clname, "ICmpChannelCreator"))
        return static_cast< ICmpChannelCreator*>(const_cast< NObjCmpConnection*>(this));
    if (!strcmp(_clname, "ICmpManager"))
        return static_cast< ICmpManager*>(const_cast< NObjCmpConnection*>(this));
    if (!strcmp(_clname, "IChDataCaptureSender"))
        return static_cast< IChDataCaptureSender*>(const_cast< NObjCmpConnection*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iCmpExt::NObjCmpConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: StateInfo((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        case 1: TdmInfo((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        case 2: ChannelInfo((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 3: ConferenceInfo((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 4;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = DriNameBoard(); break;
        case 1: *reinterpret_cast< QString*>(_v) = GetState(); break;
        case 2: *reinterpret_cast< int*>(_v) = m_countActivation; break;
        case 3: *reinterpret_cast< QString*>(_v) = LastActivation(); break;
        case 4: *reinterpret_cast< QString*>(_v) = LastDeactivation(); break;
        case 5: *reinterpret_cast< QString*>(_v) = m_lastBoardError; break;
        case 6: *reinterpret_cast< iCmpExt::ChDataCaptureMode::Value*>(_v) = m_dataCaptureMode; break;
        case 7: *reinterpret_cast< int*>(_v) = m_maxDataCaptureCount; break;
        case 8: *reinterpret_cast< int*>(_v) = m_currDataCaptureCount; break;
        }
        _id -= 9;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 6: m_dataCaptureMode = *reinterpret_cast< iCmpExt::ChDataCaptureMode::Value*>(_v); break;
        case 7: m_maxDataCaptureCount = *reinterpret_cast< int*>(_v); break;
        case 8: m_currDataCaptureCount = *reinterpret_cast< int*>(_v); break;
        }
        _id -= 9;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 9;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<iCmpExt::NObjCmpConnection> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iCmpExt::NObjCmpConnection> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
