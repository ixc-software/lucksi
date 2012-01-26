/****************************************************************************
** Meta object code from reading C++ file 'NObjReg.h'
**
** Created: Tue Jan 24 17:48:24 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iReg/NObjReg.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjReg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iReg__NObjReg[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       4,   19, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x02,

 // properties: name, type, flags
      37,   29, 0x0a095003,
      46,   41, 0x01095003,
      55,   41, 0x01095003,
      65,   41, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_iReg__NObjReg[] = {
    "iReg::NObjReg\0\0RunSelfTest()\0QString\0"
    "Dir\0bool\0RegCalls\0RegEvents\0RegTelnet\0"
};

const QMetaObject iReg::NObjReg::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iReg__NObjReg,
      qt_meta_data_iReg__NObjReg, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iReg::NObjReg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iReg::NObjReg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iReg::NObjReg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iReg__NObjReg))
        return static_cast<void*>(const_cast< NObjReg*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjReg*>(this));
    if (!strcmp(_clname, "ISysReg"))
        return static_cast< ISysReg*>(const_cast< NObjReg*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iReg::NObjReg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: RunSelfTest(); break;
        default: ;
        }
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = m_dir; break;
        case 1: *reinterpret_cast< bool*>(_v) = GetRegCalls(); break;
        case 2: *reinterpret_cast< bool*>(_v) = GetRegEvents(); break;
        case 3: *reinterpret_cast< bool*>(_v) = m_regTelnet; break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: SetDir(*reinterpret_cast< QString*>(_v)); break;
        case 1: SetRegCalls(*reinterpret_cast< bool*>(_v)); break;
        case 2: SetRegEvents(*reinterpret_cast< bool*>(_v)); break;
        case 3: SetRegTelnet(*reinterpret_cast< bool*>(_v)); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<iReg::NObjReg> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iReg::NObjReg> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
