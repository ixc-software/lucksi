/****************************************************************************
** Meta object code from reading C++ file 'NObjHwBoardList.h'
**
** Created: Tue Jan 24 14:56:21 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IpsiusService/NObjHwBoardList.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjHwBoardList.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IpsiusService__NObjHwBoardList[] = {

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
      45,   32,   31,   31, 0x02,
      84,   79,   31,   31, 0x22,

       0        // eod
};

static const char qt_meta_stringdata_IpsiusService__NObjHwBoardList[] = {
    "IpsiusService::NObjHwBoardList\0\0"
    "pCmd,briefly\0ListBoards(DRI::ICmdOutput*,bool)\0"
    "pCmd\0ListBoards(DRI::ICmdOutput*)\0"
};

const QMetaObject IpsiusService::NObjHwBoardList::staticMetaObject = {
    { &NamedObject::staticMetaObject, qt_meta_stringdata_IpsiusService__NObjHwBoardList,
      qt_meta_data_IpsiusService__NObjHwBoardList, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IpsiusService::NObjHwBoardList::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IpsiusService::NObjHwBoardList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IpsiusService::NObjHwBoardList::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IpsiusService__NObjHwBoardList))
        return static_cast<void*>(const_cast< NObjHwBoardList*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjHwBoardList*>(this));
    return NamedObject::qt_metacast(_clname);
}

int IpsiusService::NObjHwBoardList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NamedObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: ListBoards((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: ListBoards((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
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
	DRI::NamedObjectTypeRegister<IpsiusService::NObjHwBoardList> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IpsiusService::NObjHwBoardList> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
