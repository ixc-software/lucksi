/****************************************************************************
** Meta object code from reading C++ file 'liuProfile.h'
**
** Created: Tue Jan 24 17:47:22 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Blackfin/Src/Ds2155/liuProfile.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'liuProfile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Ds2155__Enums[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       2,   14, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // enums: name, flags, count, data
      14, 0x0,    3,   22,
      27, 0x0,    5,   28,

 // enum data: key, value
      37, uint(Ds2155::Enums::lcAmi),
      43, uint(Ds2155::Enums::lcHdb3),
      50, uint(Ds2155::Enums::lcB8zs),
      57, uint(Ds2155::Enums::ctPCM30),
      65, uint(Ds2155::Enums::ctPCM31),
      73, uint(Ds2155::Enums::ctNoStruct),
      84, uint(Ds2155::Enums::ctD4),
      89, uint(Ds2155::Enums::ctEsf),

       0        // eod
};

static const char qt_meta_stringdata_Ds2155__Enums[] = {
    "Ds2155::Enums\0LineCodeType\0CycleType\0"
    "lcAmi\0lcHdb3\0lcB8zs\0ctPCM30\0ctPCM31\0"
    "ctNoStruct\0ctD4\0ctEsf\0"
};

const QMetaObject Ds2155::Enums::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Ds2155__Enums,
      qt_meta_data_Ds2155__Enums, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ds2155::Enums::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ds2155::Enums::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ds2155::Enums::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ds2155__Enums))
        return static_cast<void*>(const_cast< Enums*>(this));
    return QObject::qt_metacast(_clname);
}

int Ds2155::Enums::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Ds2155::Enums> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
