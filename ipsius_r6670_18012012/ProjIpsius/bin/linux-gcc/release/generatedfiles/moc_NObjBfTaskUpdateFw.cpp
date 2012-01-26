/****************************************************************************
** Meta object code from reading C++ file 'NObjBfTaskUpdateFw.h'
**
** Created: Tue Jan 24 17:47:35 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfBootDri/NObjBfTaskUpdateFw.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjBfTaskUpdateFw.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfBootDRI__NObjBfTaskUpdateFw[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       1,   29, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      39,   31,   30,   30, 0x02,
      62,   53,   30,   30, 0x02,
      93,   83,   30,   30, 0x02,

 // properties: name, type, flags
     119,  114, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_BfBootDRI__NObjBfTaskUpdateFw[] = {
    "BfBootDRI::NObjBfTaskUpdateFw\0\0profile\0"
    "Init(QString)\0fileName\0SetFirmware(QString)\0"
    "pAsyncCmd\0Run(DRI::IAsyncCmd*)\0bool\0"
    "CheckAfter\0"
};

const QMetaObject BfBootDRI::NObjBfTaskUpdateFw::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfBootDRI__NObjBfTaskUpdateFw,
      qt_meta_data_BfBootDRI__NObjBfTaskUpdateFw, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfBootDRI::NObjBfTaskUpdateFw::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfBootDRI::NObjBfTaskUpdateFw::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfBootDRI::NObjBfTaskUpdateFw::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfBootDRI__NObjBfTaskUpdateFw))
        return static_cast<void*>(const_cast< NObjBfTaskUpdateFw*>(this));
    if (!strcmp(_clname, "ITask"))
        return static_cast< ITask*>(const_cast< NObjBfTaskUpdateFw*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfBootDRI::NObjBfTaskUpdateFw::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Init((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: SetFirmware((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: Run((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_checkAfter; break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_checkAfter = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<BfBootDRI::NObjBfTaskUpdateFw> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfBootDRI::NObjBfTaskUpdateFw> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
