/****************************************************************************
** Meta object code from reading C++ file 'NObjDss1BoardSettings.h'
**
** Created: Tue Jan 24 14:57:06 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Dss1ToSip/NObjDss1BoardSettings.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjDss1BoardSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dss1ToSip__NObjDss1BoardSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__NObjDss1BoardSettings[] = {
    "Dss1ToSip::NObjDss1BoardSettings\0"
};

const QMetaObject Dss1ToSip::NObjDss1BoardSettings::staticMetaObject = {
    { &iCmpExt::NObjCmpDevSettings::staticMetaObject, qt_meta_stringdata_Dss1ToSip__NObjDss1BoardSettings,
      qt_meta_data_Dss1ToSip__NObjDss1BoardSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::NObjDss1BoardSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::NObjDss1BoardSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::NObjDss1BoardSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__NObjDss1BoardSettings))
        return static_cast<void*>(const_cast< NObjDss1BoardSettings*>(this));
    typedef iCmpExt::NObjCmpDevSettings QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Dss1ToSip::NObjDss1BoardSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef iCmpExt::NObjCmpDevSettings QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Dss1ToSip::NObjDss1BoardSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
