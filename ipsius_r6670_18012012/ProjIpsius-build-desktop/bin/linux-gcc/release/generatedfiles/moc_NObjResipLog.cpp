/****************************************************************************
** Meta object code from reading C++ file 'NObjResipLog.h'
**
** Created: Tue Jan 24 14:58:24 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/NObjResipLog.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjResipLog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__NObjResipLog[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       8,   19, // properties
       2,   43, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      34,   20,   19,   19, 0x02,

 // properties: name, type, flags
      78,   68, 0x00095009,
      82,   68, 0x00095009,
      86,   68, 0x00095009,
      95,   68, 0x00095009,
      99,   68, 0x00095009,
     103,   68, 0x00095009,
     115,   68, 0x00095009,
     125,   68, 0x00095009,

 // enums: name, flags, count, data
      68, 0x0,    9,   51,
     131, 0x0,    9,   69,

 // enum data: key, value
     141, uint(iSip::NObjResipLog::None),
     146, uint(iSip::NObjResipLog::Critical),
     155, uint(iSip::NObjResipLog::Error),
     161, uint(iSip::NObjResipLog::Warning),
     169, uint(iSip::NObjResipLog::Info),
     174, uint(iSip::NObjResipLog::Debug),
     180, uint(iSip::NObjResipLog::Stack),
     186, uint(iSip::NObjResipLog::StdErr),
     193, uint(iSip::NObjResipLog::Bogus),
     199, uint(iSip::NObjResipLog::All),
      78, uint(iSip::NObjResipLog::Dns),
      82, uint(iSip::NObjResipLog::Dum),
      86, uint(iSip::NObjResipLog::Presence),
      95, uint(iSip::NObjResipLog::Sdp),
      99, uint(iSip::NObjResipLog::Sip),
     103, uint(iSip::NObjResipLog::Transaction),
     115, uint(iSip::NObjResipLog::Transport),
     125, uint(iSip::NObjResipLog::Stats),

       0        // eod
};

static const char qt_meta_stringdata_iSip__NObjResipLog[] = {
    "iSip::NObjResipLog\0\0susystem,type\0"
    "SetTraceType(Subsystem,TraceType)\0"
    "TraceType\0Dns\0Dum\0Presence\0Sdp\0Sip\0"
    "Transaction\0Transport\0Stats\0Subsystem\0"
    "None\0Critical\0Error\0Warning\0Info\0Debug\0"
    "Stack\0StdErr\0Bogus\0All\0"
};

const QMetaObject iSip::NObjResipLog::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iSip__NObjResipLog,
      qt_meta_data_iSip__NObjResipLog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::NObjResipLog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::NObjResipLog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::NObjResipLog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__NObjResipLog))
        return static_cast<void*>(const_cast< NObjResipLog*>(this));
    if (!strcmp(_clname, "resip::ExternalLogger"))
        return static_cast< resip::ExternalLogger*>(const_cast< NObjResipLog*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iSip::NObjResipLog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SetTraceType((*reinterpret_cast< Subsystem(*)>(_a[1])),(*reinterpret_cast< TraceType(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< TraceType*>(_v) = m_dnsTraceType; break;
        case 1: *reinterpret_cast< TraceType*>(_v) = m_dumTraceType; break;
        case 2: *reinterpret_cast< TraceType*>(_v) = m_presenceTraceType; break;
        case 3: *reinterpret_cast< TraceType*>(_v) = m_sdpTraceType; break;
        case 4: *reinterpret_cast< TraceType*>(_v) = m_sipTraceType; break;
        case 5: *reinterpret_cast< TraceType*>(_v) = m_transactionTraceType; break;
        case 6: *reinterpret_cast< TraceType*>(_v) = m_transportTraceType; break;
        case 7: *reinterpret_cast< TraceType*>(_v) = m_statsTraceType; break;
        }
        _id -= 8;
    } else if (_c == QMetaObject::WriteProperty) {
        _id -= 8;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 8;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 8;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<iSip::NObjResipLog> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iSip::NObjResipLog> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
