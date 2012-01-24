/****************************************************************************
** Meta object code from reading C++ file 'L3MessageType.h'
**
** Created: Tue Jan 24 14:57:47 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/ISDN/L3MessageType.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'L3MessageType.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ISDN__L3MessgeType[] = {

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
      19, 0x0,   32,   18,

 // enum data: key, value
      24, uint(ISDN::L3MessgeType::ALERTING),
      33, uint(ISDN::L3MessgeType::CALL_PROCEEDING),
      49, uint(ISDN::L3MessgeType::CONNECT),
      57, uint(ISDN::L3MessgeType::CONNECT_ACKNOWLEDGE),
      77, uint(ISDN::L3MessgeType::PROGRESS),
      86, uint(ISDN::L3MessgeType::SETUP),
      92, uint(ISDN::L3MessgeType::SETUP_ACKNOWLEDGE),
     110, uint(ISDN::L3MessgeType::RESUME),
     117, uint(ISDN::L3MessgeType::RESUME_ACKNOWLEDGE),
     136, uint(ISDN::L3MessgeType::RESUME_REJECT),
     150, uint(ISDN::L3MessgeType::SUSPEND),
     158, uint(ISDN::L3MessgeType::SUSPEND_ACKNOWLEDGE),
     178, uint(ISDN::L3MessgeType::SUSPEND_REJECT),
     193, uint(ISDN::L3MessgeType::USER_INFORMATION),
     210, uint(ISDN::L3MessgeType::DISCONNECT),
     221, uint(ISDN::L3MessgeType::RELEASE),
     229, uint(ISDN::L3MessgeType::RELEASE_COMPLETE),
     246, uint(ISDN::L3MessgeType::RESTART),
     254, uint(ISDN::L3MessgeType::RESTART_ACKNOWLEDGE),
     274, uint(ISDN::L3MessgeType::SEGMENT),
     282, uint(ISDN::L3MessgeType::CONGESTION_CONTROL),
     301, uint(ISDN::L3MessgeType::INFORMATION),
     313, uint(ISDN::L3MessgeType::FACILITY),
     322, uint(ISDN::L3MessgeType::NOTIFY),
     329, uint(ISDN::L3MessgeType::STATUS),
     336, uint(ISDN::L3MessgeType::STATUS_ENQUIRY),
     351, uint(ISDN::L3MessgeType::HOLD),
     356, uint(ISDN::L3MessgeType::HOLD_ACK),
     365, uint(ISDN::L3MessgeType::RETRIEVE),
     374, uint(ISDN::L3MessgeType::RETRIEVE_ACK),
     387, uint(ISDN::L3MessgeType::RETRIEVE_REJ),
     400, uint(ISDN::L3MessgeType::UNRNOWN),

       0        // eod
};

static const char qt_meta_stringdata_ISDN__L3MessgeType[] = {
    "ISDN::L3MessgeType\0Type\0ALERTING\0"
    "CALL_PROCEEDING\0CONNECT\0CONNECT_ACKNOWLEDGE\0"
    "PROGRESS\0SETUP\0SETUP_ACKNOWLEDGE\0"
    "RESUME\0RESUME_ACKNOWLEDGE\0RESUME_REJECT\0"
    "SUSPEND\0SUSPEND_ACKNOWLEDGE\0SUSPEND_REJECT\0"
    "USER_INFORMATION\0DISCONNECT\0RELEASE\0"
    "RELEASE_COMPLETE\0RESTART\0RESTART_ACKNOWLEDGE\0"
    "SEGMENT\0CONGESTION_CONTROL\0INFORMATION\0"
    "FACILITY\0NOTIFY\0STATUS\0STATUS_ENQUIRY\0"
    "HOLD\0HOLD_ACK\0RETRIEVE\0RETRIEVE_ACK\0"
    "RETRIEVE_REJ\0UNRNOWN\0"
};

const QMetaObject ISDN::L3MessgeType::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ISDN__L3MessgeType,
      qt_meta_data_ISDN__L3MessgeType, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ISDN::L3MessgeType::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ISDN::L3MessgeType::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ISDN::L3MessgeType::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ISDN__L3MessgeType))
        return static_cast<void*>(const_cast< L3MessgeType*>(this));
    return QObject::qt_metacast(_clname);
}

int ISDN::L3MessgeType::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<ISDN::L3MessgeType> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
