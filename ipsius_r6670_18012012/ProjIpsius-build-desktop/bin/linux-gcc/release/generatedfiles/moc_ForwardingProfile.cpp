/****************************************************************************
** Meta object code from reading C++ file 'ForwardingProfile.h'
**
** Created: Tue Jan 24 14:58:11 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/ForwardingProfile.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ForwardingProfile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__ForwardingType[] = {

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
      21, 0x0,    5,   18,

 // enum data: key, value
      25, uint(iSip::ForwardingType::None),
      30, uint(iSip::ForwardingType::Unconditional),
      44, uint(iSip::ForwardingType::OnBusy),
      51, uint(iSip::ForwardingType::OnFailure),
      61, uint(iSip::ForwardingType::OnNoAnswer),

       0        // eod
};

static const char qt_meta_stringdata_iSip__ForwardingType[] = {
    "iSip::ForwardingType\0Val\0None\0"
    "Unconditional\0OnBusy\0OnFailure\0"
    "OnNoAnswer\0"
};

const QMetaObject iSip::ForwardingType::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_iSip__ForwardingType,
      qt_meta_data_iSip__ForwardingType, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::ForwardingType::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::ForwardingType::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::ForwardingType::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__ForwardingType))
        return static_cast<void*>(const_cast< ForwardingType*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< ForwardingType*>(this));
    return QObject::qt_metacast(_clname);
}

int iSip::ForwardingType::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<iSip::ForwardingType> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
