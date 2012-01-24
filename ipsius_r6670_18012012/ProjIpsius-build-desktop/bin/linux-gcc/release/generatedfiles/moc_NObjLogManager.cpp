/****************************************************************************
** Meta object code from reading C++ file 'NObjLogManager.h'
**
** Created: Tue Jan 24 14:56:33 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Domain/NObjLogManager.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjLogManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Domain__NObjLogManager[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
      13,   34, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x02,
      34,   32,   23,   23, 0x02,
      56,   47,   23,   23, 0x02,
      84,   23,   23,   23, 0x22,

 // properties: name, type, flags
     110,  105, 0x01095003,
     122,  105, 0x01095003,
     142,  134, 0x0a095003,
     151,  105, 0x01095003,
     168,  105, 0x01095003,
     179,  134, 0x0a095003,
     191,  105, 0x01095003,
     205,  201, 0x02095003,
     217,  201, 0x02095003,
     231,  201, 0x02095003,
     242,  201, 0x02095003,
     250,  134, 0x0a095003,
     264,  134, 0x0a095001,

       0        // eod
};

static const char qt_meta_stringdata_Domain__NObjLogManager[] = {
    "Domain::NObjLogManager\0\0Clear()\0s\0"
    "Log(QString)\0fileName\0DumpSessionsToFile(QString)\0"
    "DumpSessionsToFile()\0bool\0TraceToCout\0"
    "TraceToFile\0QString\0FileName\0"
    "DirectSaveToFile\0TraceToUdp\0DstUdpTrace\0"
    "Timestamp\0int\0CountToSync\0TimeoutToSync\0"
    "MaxRecords\0CleanTo\0PathSeparator\0Stat\0"
};

const QMetaObject Domain::NObjLogManager::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Domain__NObjLogManager,
      qt_meta_data_Domain__NObjLogManager, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Domain::NObjLogManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Domain::NObjLogManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Domain::NObjLogManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Domain__NObjLogManager))
        return static_cast<void*>(const_cast< NObjLogManager*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjLogManager*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Domain::NObjLogManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Clear(); break;
        case 1: Log((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: DumpSessionsToFile((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: DumpSessionsToFile(); break;
        default: ;
        }
        _id -= 4;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = TraceToCout(); break;
        case 1: *reinterpret_cast< bool*>(_v) = TraceToFile(); break;
        case 2: *reinterpret_cast< QString*>(_v) = FileName(); break;
        case 3: *reinterpret_cast< bool*>(_v) = DirectSaveToFile(); break;
        case 4: *reinterpret_cast< bool*>(_v) = TraceToUdp(); break;
        case 5: *reinterpret_cast< QString*>(_v) = DstUdpTrace(); break;
        case 6: *reinterpret_cast< bool*>(_v) = TimestampInd(); break;
        case 7: *reinterpret_cast< int*>(_v) = CountToSync(); break;
        case 8: *reinterpret_cast< int*>(_v) = TimeoutToSync(); break;
        case 9: *reinterpret_cast< int*>(_v) = StoreRecordsLimit(); break;
        case 10: *reinterpret_cast< int*>(_v) = StoreCleanTo(); break;
        case 11: *reinterpret_cast< QString*>(_v) = PathSeparator(); break;
        case 12: *reinterpret_cast< QString*>(_v) = Stat(); break;
        }
        _id -= 13;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: TraceToCout(*reinterpret_cast< bool*>(_v)); break;
        case 1: TraceToFile(*reinterpret_cast< bool*>(_v)); break;
        case 2: FileName(*reinterpret_cast< QString*>(_v)); break;
        case 3: DirectSaveToFile(*reinterpret_cast< bool*>(_v)); break;
        case 4: TraceToUdp(*reinterpret_cast< bool*>(_v)); break;
        case 5: DstUdpTrace(*reinterpret_cast< QString*>(_v)); break;
        case 6: TimestampInd(*reinterpret_cast< bool*>(_v)); break;
        case 7: CountToSync(*reinterpret_cast< int*>(_v)); break;
        case 8: TimeoutToSync(*reinterpret_cast< int*>(_v)); break;
        case 9: StoreRecordsLimit(*reinterpret_cast< int*>(_v)); break;
        case 10: StoreCleanTo(*reinterpret_cast< int*>(_v)); break;
        case 11: PathSeparator(*reinterpret_cast< QString*>(_v)); break;
        }
        _id -= 13;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 13;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 13;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 13;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 13;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 13;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 13;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<Domain::NObjLogManager> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Domain::NObjLogManager> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
