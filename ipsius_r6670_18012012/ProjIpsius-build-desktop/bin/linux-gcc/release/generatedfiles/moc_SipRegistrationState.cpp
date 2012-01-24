/****************************************************************************
** Meta object code from reading C++ file 'SipRegistrationState.h'
**
** Created: Tue Jan 24 14:58:27 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/SipRegistrationState.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SipRegistrationState.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__SipRegistrationState[] = {

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
      27, 0x0,    5,   18,

 // enum data: key, value
      33, uint(iSip::SipRegistrationState::Disabled),
      42, uint(iSip::SipRegistrationState::RsNoRegistration),
      59, uint(iSip::SipRegistrationState::RsProgress),
      70, uint(iSip::SipRegistrationState::RsSuccess),
      80, uint(iSip::SipRegistrationState::RsRemoved),

       0        // eod
};

static const char qt_meta_stringdata_iSip__SipRegistrationState[] = {
    "iSip::SipRegistrationState\0Value\0"
    "Disabled\0RsNoRegistration\0RsProgress\0"
    "RsSuccess\0RsRemoved\0"
};

const QMetaObject iSip::SipRegistrationState::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_iSip__SipRegistrationState,
      qt_meta_data_iSip__SipRegistrationState, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::SipRegistrationState::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::SipRegistrationState::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::SipRegistrationState::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__SipRegistrationState))
        return static_cast<void*>(const_cast< SipRegistrationState*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< SipRegistrationState*>(this));
    return QObject::qt_metacast(_clname);
}

int iSip::SipRegistrationState::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<iSip::SipRegistrationState> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
