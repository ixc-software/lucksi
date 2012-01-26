/****************************************************************************
** Meta object code from reading C++ file 'GateCallInfo.h'
**
** Created: Tue Jan 24 17:48:45 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Dss1ToSip/GateCallInfo.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GateCallInfo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dss1ToSip__GateCallState[] = {

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
      25, 0x0,    4,   18,

 // enum data: key, value
      31, uint(Dss1ToSip::GateCallState::Trying),
      38, uint(Dss1ToSip::GateCallState::Dialing),
      46, uint(Dss1ToSip::GateCallState::Ringing),
      54, uint(Dss1ToSip::GateCallState::Talk),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__GateCallState[] = {
    "Dss1ToSip::GateCallState\0Value\0Trying\0"
    "Dialing\0Ringing\0Talk\0"
};

const QMetaObject Dss1ToSip::GateCallState::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__GateCallState,
      qt_meta_data_Dss1ToSip__GateCallState, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::GateCallState::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::GateCallState::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::GateCallState::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__GateCallState))
        return static_cast<void*>(const_cast< GateCallState*>(this));
    return QObject::qt_metacast(_clname);
}

int Dss1ToSip::GateCallState::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<Dss1ToSip::GateCallState> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
