/****************************************************************************
** Meta object code from reading C++ file 'NObjLogSettings.h'
**
** Created: Tue Jan 24 17:48:00 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Domain/NObjLogSettings.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjLogSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Domain__NObjLogSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       6,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      29,   24, 0x01095003,
      43,   39, 0x02095003,
      55,   39, 0x02095003,
      69,   39, 0x02095003,
      80,   39, 0x02095003,
      96,   88, 0x0a095003,

       0        // eod
};

static const char qt_meta_stringdata_Domain__NObjLogSettings[] = {
    "Domain::NObjLogSettings\0bool\0Timestamp\0"
    "int\0CountToSync\0TimeoutToSync\0MaxRecords\0"
    "CleanTo\0QString\0PathSeparator\0"
};

const QMetaObject Domain::NObjLogSettings::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Domain__NObjLogSettings,
      qt_meta_data_Domain__NObjLogSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Domain::NObjLogSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Domain::NObjLogSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Domain::NObjLogSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Domain__NObjLogSettings))
        return static_cast<void*>(const_cast< NObjLogSettings*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjLogSettings*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Domain::NObjLogSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = TimestampInd(); break;
        case 1: *reinterpret_cast< int*>(_v) = CountToSync(); break;
        case 2: *reinterpret_cast< int*>(_v) = TimeoutToSync(); break;
        case 3: *reinterpret_cast< int*>(_v) = StoreRecordsLimit(); break;
        case 4: *reinterpret_cast< int*>(_v) = StoreCleanTo(); break;
        case 5: *reinterpret_cast< QString*>(_v) = PathSeparator(); break;
        }
        _id -= 6;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: TimestampInd(*reinterpret_cast< bool*>(_v)); break;
        case 1: CountToSync(*reinterpret_cast< int*>(_v)); break;
        case 2: TimeoutToSync(*reinterpret_cast< int*>(_v)); break;
        case 3: StoreRecordsLimit(*reinterpret_cast< int*>(_v)); break;
        case 4: StoreCleanTo(*reinterpret_cast< int*>(_v)); break;
        case 5: PathSeparator(*reinterpret_cast< QString*>(_v)); break;
        }
        _id -= 6;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 6;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<Domain::NObjLogSettings> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Domain::NObjLogSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
