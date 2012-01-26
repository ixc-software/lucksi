/****************************************************************************
** Meta object code from reading C++ file 'SipUtils.h'
**
** Created: Tue Jan 24 17:50:13 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/SipUtils.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SipUtils.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__SipUtils[] = {

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
      15, 0x0,    3,   18,

 // enum data: key, value
      25, uint(iSip::SipUtils::Short),
      31, uint(iSip::SipUtils::Detail),
      38, uint(iSip::SipUtils::Resip),

       0        // eod
};

static const char qt_meta_stringdata_iSip__SipUtils[] = {
    "iSip::SipUtils\0PrintMode\0Short\0Detail\0"
    "Resip\0"
};

const QMetaObject iSip::SipUtils::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_iSip__SipUtils,
      qt_meta_data_iSip__SipUtils, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::SipUtils::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::SipUtils::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::SipUtils::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__SipUtils))
        return static_cast<void*>(const_cast< SipUtils*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< SipUtils*>(this));
    return QObject::qt_metacast(_clname);
}

int iSip::SipUtils::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<iSip::SipUtils> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
