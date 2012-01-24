/****************************************************************************
** Meta object code from reading C++ file 'HwBoardFsmStates.h'
**
** Created: Tue Jan 24 14:56:14 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IpsiusService/HwBoardFsmStates.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HwBoardFsmStates.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IpsiusService__HwBoardFsmStates[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       1,   14, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // enums: name, flags, count, data
      32, 0x0,    9,   18,

 // enum data: key, value
      39, uint(IpsiusService::HwBoardFsmStates::StInitial),
      49, uint(IpsiusService::HwBoardFsmStates::StDiscovered),
      62, uint(IpsiusService::HwBoardFsmStates::StWaitBooterExit),
      79, uint(IpsiusService::HwBoardFsmStates::StReady),
      87, uint(IpsiusService::HwBoardFsmStates::StOff),
      93, uint(IpsiusService::HwBoardFsmStates::StBusy),
     100, uint(IpsiusService::HwBoardFsmStates::StService),
     110, uint(IpsiusService::HwBoardFsmStates::StBlocked),
     120, uint(IpsiusService::HwBoardFsmStates::StWaitBooterEnter),

       0        // eod
};

static const char qt_meta_stringdata_IpsiusService__HwBoardFsmStates[] = {
    "IpsiusService::HwBoardFsmStates\0States\0"
    "StInitial\0StDiscovered\0StWaitBooterExit\0"
    "StReady\0StOff\0StBusy\0StService\0StBlocked\0"
    "StWaitBooterEnter\0"
};

const QMetaObject IpsiusService::HwBoardFsmStates::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_IpsiusService__HwBoardFsmStates,
      qt_meta_data_IpsiusService__HwBoardFsmStates, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IpsiusService::HwBoardFsmStates::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IpsiusService::HwBoardFsmStates::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IpsiusService::HwBoardFsmStates::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IpsiusService__HwBoardFsmStates))
        return static_cast<void*>(const_cast< HwBoardFsmStates*>(this));
    return QObject::qt_metacast(_clname);
}

int IpsiusService::HwBoardFsmStates::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<IpsiusService::HwBoardFsmStates> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
