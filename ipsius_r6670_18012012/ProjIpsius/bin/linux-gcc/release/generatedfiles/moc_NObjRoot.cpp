/****************************************************************************
** Meta object code from reading C++ file 'NObjRoot.h'
**
** Created: Tue Jan 24 17:47:56 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Domain/NObjRoot.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjRoot.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Domain__NObjRoot[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       8,  129, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      23,   18,   17,   17, 0x02,
      56,   17,   17,   17, 0x02,
      72,   17,   17,   17, 0x02,
      88,   17,   17,   17, 0x02,
     109,   17,   17,   17, 0x02,
     118,   17,   17,   17, 0x02,
     165,  136,   17,   17, 0x02,
     225,  204,   17,   17, 0x22,
     270,  260,   17,   17, 0x22,
     300,   17,   17,   17, 0x02,
     337,  319,   17,   17, 0x02,
     370,  364,   17,   17, 0x02,
     391,   18,   17,   17, 0x02,
     438,  427,   17,   17, 0x02,
     461,   17,   17,   17, 0x02,
     479,  427,   17,   17, 0x02,
     500,   17,   17,   17, 0x02,
     517,   18,   17,   17, 0x02,
     552,  548,   17,   17, 0x02,
     597,  574,   17,   17, 0x02,
     658,  645,   17,   17, 0x22,
     698,   17,   17,   17, 0x02,
     711,   18,   17,   17, 0x02,

 // properties: name, type, flags
     749,  741, 0x0a095001,
     759,  741, 0x0a095001,
     772,  767, 0x01095103,
     782,  741, 0x0a095001,
     791,  741, 0x0a095001,
     800,  741, 0x0a095001,
     817,  767, 0x01095103,
     839,  767, 0x01095103,

       0        // eod
};

static const char qt_meta_stringdata_Domain__NObjRoot[] = {
    "Domain::NObjRoot\0\0pCmd\0"
    "TestStackTrace(DRI::ICmdOutput*)\0"
    "TestAssertion()\0TestException()\0"
    "TestAsyncException()\0TestAV()\0"
    "TestMsgOverload()\0pAsyncCmd,canAborted,timeOut\0"
    "TestAsyncCmd(DRI::IAsyncCmd*,bool,int)\0"
    "pAsyncCmd,canAborted\0"
    "TestAsyncCmd(DRI::IAsyncCmd*,bool)\0"
    "pAsyncCmd\0TestAsyncCmd(DRI::IAsyncCmd*)\0"
    "TestQtWarningMsg()\0pAsyncCmd,timeOut\0"
    "Sleep(DRI::IAsyncCmd*,int)\0depth\0"
    "SetMsgProfiling(int)\0"
    "PrintMsgProfiling(DRI::ICmdOutput*)\0"
    "intervalMs\0SetMaxMessageTime(int)\0"
    "TestMsgWatchdog()\0StartNetMonitor(int)\0"
    "StopNetMonitor()\0SessionsList(DRI::ICmdOutput*)\0"
    "val\0HaltOnQtWarning(bool)\0"
    "pCmd,scpName,refOutput\0"
    "RunTestScript(DRI::ICmdOutput*,QString,QString)\0"
    "pCmd,scpName\0RunTestScript(DRI::ICmdOutput*,QString)\0"
    "DomainExit()\0ListThreads(DRI::ICmdOutput*)\0"
    "QString\0BuildInfo\0Started\0bool\0HeapTrace\0"
    "HeapInfo\0Sessions\0NetMonitorStatus\0"
    "ExitOnLastSessionDrop\0TraceSessions\0"
};

const QMetaObject Domain::NObjRoot::staticMetaObject = {
    { &NamedObject::staticMetaObject, qt_meta_stringdata_Domain__NObjRoot,
      qt_meta_data_Domain__NObjRoot, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Domain::NObjRoot::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Domain::NObjRoot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Domain::NObjRoot::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Domain__NObjRoot))
        return static_cast<void*>(const_cast< NObjRoot*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjRoot*>(this));
    return NamedObject::qt_metacast(_clname);
}

int Domain::NObjRoot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NamedObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: TestStackTrace((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 1: TestAssertion(); break;
        case 2: TestException(); break;
        case 3: TestAsyncException(); break;
        case 4: TestAV(); break;
        case 5: TestMsgOverload(); break;
        case 6: TestAsyncCmd((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 7: TestAsyncCmd((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 8: TestAsyncCmd((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        case 9: TestQtWarningMsg(); break;
        case 10: Sleep((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 11: SetMsgProfiling((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: PrintMsgProfiling((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 13: SetMaxMessageTime((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: TestMsgWatchdog(); break;
        case 15: StartNetMonitor((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: StopNetMonitor(); break;
        case 17: SessionsList((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 18: HaltOnQtWarning((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 19: RunTestScript((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 20: RunTestScript((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 21: DomainExit(); break;
        case 22: ListThreads((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 23;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = BuildInfo(); break;
        case 1: *reinterpret_cast< QString*>(_v) = m_startDateTime; break;
        case 2: *reinterpret_cast< bool*>(_v) = getHeapTrace(); break;
        case 3: *reinterpret_cast< QString*>(_v) = getHeapInfo(); break;
        case 4: *reinterpret_cast< QString*>(_v) = getSessionsInfo(); break;
        case 5: *reinterpret_cast< QString*>(_v) = NetMonitorStatus(); break;
        case 6: *reinterpret_cast< bool*>(_v) = getExitOnLastSessionDrop(); break;
        case 7: *reinterpret_cast< bool*>(_v) = getTraceSessions(); break;
        }
        _id -= 8;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 2: setHeapTrace(*reinterpret_cast< bool*>(_v)); break;
        case 6: setExitOnLastSessionDrop(*reinterpret_cast< bool*>(_v)); break;
        case 7: setTraceSessions(*reinterpret_cast< bool*>(_v)); break;
        }
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
	DRI::NamedObjectTypeRegister<Domain::NObjRoot> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Domain::NObjRoot> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
