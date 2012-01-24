/****************************************************************************
** Meta object code from reading C++ file 'UserCallInternalFsmBase.h'
**
** Created: Tue Jan 24 14:58:09 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IsdnTest/UserCallInternalFsmBase.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UserCallInternalFsmBase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IsdnTest__UserCallInternalStates[] = {

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
      33, 0x0,   13,   18,

 // enum data: key, value
      36, uint(IsdnTest::UserCallInternalStates::st_null),
      44, uint(IsdnTest::UserCallInternalStates::st_waitDssCallCreated),
      66, uint(IsdnTest::UserCallInternalStates::st_waitConnConfirm),
      85, uint(IsdnTest::UserCallInternalStates::st_waitIncommingCall),
     106, uint(IsdnTest::UserCallInternalStates::st_waitAlerting),
     122, uint(IsdnTest::UserCallInternalStates::st_waitConnected),
     139, uint(IsdnTest::UserCallInternalStates::st_waitDisconnected),
     159, uint(IsdnTest::UserCallInternalStates::st_waitDiscWithCause),
     180, uint(IsdnTest::UserCallInternalStates::st_waitLinkDisc),
     196, uint(IsdnTest::UserCallInternalStates::st_waitLinkConnected),
     217, uint(IsdnTest::UserCallInternalStates::st_waitDssCallErrorMsg),
     240, uint(IsdnTest::UserCallInternalStates::st_waitSelfTimer),
     257, uint(IsdnTest::UserCallInternalStates::st_complete),

       0        // eod
};

static const char qt_meta_stringdata_IsdnTest__UserCallInternalStates[] = {
    "IsdnTest::UserCallInternalStates\0St\0"
    "st_null\0st_waitDssCallCreated\0"
    "st_waitConnConfirm\0st_waitIncommingCall\0"
    "st_waitAlerting\0st_waitConnected\0"
    "st_waitDisconnected\0st_waitDiscWithCause\0"
    "st_waitLinkDisc\0st_waitLinkConnected\0"
    "st_waitDssCallErrorMsg\0st_waitSelfTimer\0"
    "st_complete\0"
};

const QMetaObject IsdnTest::UserCallInternalStates::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_IsdnTest__UserCallInternalStates,
      qt_meta_data_IsdnTest__UserCallInternalStates, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IsdnTest::UserCallInternalStates::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IsdnTest::UserCallInternalStates::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IsdnTest::UserCallInternalStates::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IsdnTest__UserCallInternalStates))
        return static_cast<void*>(const_cast< UserCallInternalStates*>(this));
    return QObject::qt_metacast(_clname);
}

int IsdnTest::UserCallInternalStates::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<IsdnTest::UserCallInternalStates> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
