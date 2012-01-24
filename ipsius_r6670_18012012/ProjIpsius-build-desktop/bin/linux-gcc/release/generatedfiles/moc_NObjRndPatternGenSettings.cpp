/****************************************************************************
** Meta object code from reading C++ file 'NObjRndPatternGenSettings.h'
**
** Created: Tue Jan 24 14:58:03 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IsdnTest/NObjRndPatternGenSettings.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjRndPatternGenSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IsdnTest__NObjRndPatternGenSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       7,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      40,   36, 0x02095103,
      45,   36, 0x02095103,
      70,   36, 0x02095103,
      84,   36, 0x02095103,
     106,   36, 0x02095103,
     118,   36, 0x02095103,
     140,   36, 0x02095103,

       0        // eod
};

static const char qt_meta_stringdata_IsdnTest__NObjRndPatternGenSettings[] = {
    "IsdnTest::NObjRndPatternGenSettings\0"
    "int\0Seed\0ChangeBytesChancePercent\0"
    "MaxChangeBits\0CutBytesChancePercent\0"
    "MaxCutBytes\0AddBytesChancePercent\0"
    "MaxAddBytes\0"
};

const QMetaObject IsdnTest::NObjRndPatternGenSettings::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_IsdnTest__NObjRndPatternGenSettings,
      qt_meta_data_IsdnTest__NObjRndPatternGenSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IsdnTest::NObjRndPatternGenSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IsdnTest::NObjRndPatternGenSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IsdnTest::NObjRndPatternGenSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IsdnTest__NObjRndPatternGenSettings))
        return static_cast<void*>(const_cast< NObjRndPatternGenSettings*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjRndPatternGenSettings*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int IsdnTest::NObjRndPatternGenSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = getSeed(); break;
        case 1: *reinterpret_cast< int*>(_v) = getChangeBytesChancePercent(); break;
        case 2: *reinterpret_cast< int*>(_v) = getMaxChangeBits(); break;
        case 3: *reinterpret_cast< int*>(_v) = getCutBytesChancePercent(); break;
        case 4: *reinterpret_cast< int*>(_v) = getMaxCutBytes(); break;
        case 5: *reinterpret_cast< int*>(_v) = getAddBytesChancePercent(); break;
        case 6: *reinterpret_cast< int*>(_v) = getMaxAddBytes(); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setSeed(*reinterpret_cast< int*>(_v)); break;
        case 1: setChangeBytesChancePercent(*reinterpret_cast< int*>(_v)); break;
        case 2: setMaxChangeBits(*reinterpret_cast< int*>(_v)); break;
        case 3: setCutBytesChancePercent(*reinterpret_cast< int*>(_v)); break;
        case 4: setMaxCutBytes(*reinterpret_cast< int*>(_v)); break;
        case 5: setAddBytesChancePercent(*reinterpret_cast< int*>(_v)); break;
        case 6: setMaxAddBytes(*reinterpret_cast< int*>(_v)); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<IsdnTest::NObjRndPatternGenSettings> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IsdnTest::NObjRndPatternGenSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
