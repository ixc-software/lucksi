/****************************************************************************
** Meta object code from reading C++ file 'NObjBfTaskEraseDefault.h'
**
** Created: Tue Jan 24 17:47:27 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfBootDri/NObjBfTaskEraseDefault.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjBfTaskEraseDefault.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfBootDRI__NObjBfTaskEraseDefault[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      43,   35,   34,   34, 0x02,
      67,   57,   34,   34, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_BfBootDRI__NObjBfTaskEraseDefault[] = {
    "BfBootDRI::NObjBfTaskEraseDefault\0\0"
    "profile\0Init(QString)\0pAsyncCmd\0"
    "Run(DRI::IAsyncCmd*)\0"
};

const QMetaObject BfBootDRI::NObjBfTaskEraseDefault::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfBootDRI__NObjBfTaskEraseDefault,
      qt_meta_data_BfBootDRI__NObjBfTaskEraseDefault, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfBootDRI::NObjBfTaskEraseDefault::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfBootDRI::NObjBfTaskEraseDefault::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfBootDRI::NObjBfTaskEraseDefault::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfBootDRI__NObjBfTaskEraseDefault))
        return static_cast<void*>(const_cast< NObjBfTaskEraseDefault*>(this));
    if (!strcmp(_clname, "ITask"))
        return static_cast< ITask*>(const_cast< NObjBfTaskEraseDefault*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfBootDRI::NObjBfTaskEraseDefault::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Init((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: Run((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<BfBootDRI::NObjBfTaskEraseDefault> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfBootDRI::NObjBfTaskEraseDefault> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
