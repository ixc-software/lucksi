/****************************************************************************
** Meta object code from reading C++ file 'L3CallFsm.h'
**
** Created: Tue Jan 24 17:49:24 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/ISDN/L3CallFsm.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'L3CallFsm.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ISDN__L3CallFsmStates[] = {

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
      22, 0x0,   15,   18,

 // enum data: key, value
      29, uint(ISDN::L3CallFsmStates::st_null),
      37, uint(ISDN::L3CallFsmStates::CallInitiated_1),
      53, uint(ISDN::L3CallFsmStates::OverlapSending_2),
      70, uint(ISDN::L3CallFsmStates::CallPresent_6),
      84, uint(ISDN::L3CallFsmStates::CallReceive_7),
      98, uint(ISDN::L3CallFsmStates::OutCallProceeding_3),
     118, uint(ISDN::L3CallFsmStates::CallDelivered_4),
     134, uint(ISDN::L3CallFsmStates::ConnectReq_8),
     147, uint(ISDN::L3CallFsmStates::InCallProc_9),
     160, uint(ISDN::L3CallFsmStates::Active_10),
     170, uint(ISDN::L3CallFsmStates::DiscRequested_11),
     187, uint(ISDN::L3CallFsmStates::DiscIndication_12),
     205, uint(ISDN::L3CallFsmStates::ReleaseReq_19),
     219, uint(ISDN::L3CallFsmStates::OverlapReceiv_25),
     236, uint(ISDN::L3CallFsmStates::tmp),

       0        // eod
};

static const char qt_meta_stringdata_ISDN__L3CallFsmStates[] = {
    "ISDN::L3CallFsmStates\0States\0st_null\0"
    "CallInitiated_1\0OverlapSending_2\0"
    "CallPresent_6\0CallReceive_7\0"
    "OutCallProceeding_3\0CallDelivered_4\0"
    "ConnectReq_8\0InCallProc_9\0Active_10\0"
    "DiscRequested_11\0DiscIndication_12\0"
    "ReleaseReq_19\0OverlapReceiv_25\0tmp\0"
};

const QMetaObject ISDN::L3CallFsmStates::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ISDN__L3CallFsmStates,
      qt_meta_data_ISDN__L3CallFsmStates, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ISDN::L3CallFsmStates::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ISDN::L3CallFsmStates::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ISDN::L3CallFsmStates::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ISDN__L3CallFsmStates))
        return static_cast<void*>(const_cast< L3CallFsmStates*>(this));
    return QObject::qt_metacast(_clname);
}

int ISDN::L3CallFsmStates::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<ISDN::L3CallFsmStates> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
