/****************************************************************************
** Meta object code from reading C++ file 'NObjBfTaskUpdateBooter.h'
**
** Created: Tue Jan 24 14:56:07 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfBootDri/NObjBfTaskUpdateBooter.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjBfTaskUpdateBooter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfBootDRI__NObjBfTaskUpdateBooter[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      43,   35,   34,   34, 0x02,
      62,   57,   34,   34, 0x02,
      94,   84,   34,   34, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_BfBootDRI__NObjBfTaskUpdateBooter[] = {
    "BfBootDRI::NObjBfTaskUpdateBooter\0\0"
    "profile\0Init(QString)\0file\0"
    "SetBootImage(QString)\0pAsyncCmd\0"
    "Run(DRI::IAsyncCmd*)\0"
};

const QMetaObject BfBootDRI::NObjBfTaskUpdateBooter::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfBootDRI__NObjBfTaskUpdateBooter,
      qt_meta_data_BfBootDRI__NObjBfTaskUpdateBooter, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfBootDRI::NObjBfTaskUpdateBooter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfBootDRI::NObjBfTaskUpdateBooter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfBootDRI::NObjBfTaskUpdateBooter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfBootDRI__NObjBfTaskUpdateBooter))
        return static_cast<void*>(const_cast< NObjBfTaskUpdateBooter*>(this));
    if (!strcmp(_clname, "ITask"))
        return static_cast< ITask*>(const_cast< NObjBfTaskUpdateBooter*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfBootDRI::NObjBfTaskUpdateBooter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Init((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: SetBootImage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: Run((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<BfBootDRI::NObjBfTaskUpdateBooter> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfBootDRI::NObjBfTaskUpdateBooter> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
