/****************************************************************************
** Meta object code from reading C++ file 'NObjFwuMaker.h'
**
** Created: Tue Jan 24 14:57:24 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/HiLevelTests/HwFinder/NObjFwuMaker.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjFwuMaker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HiLevelTests__HwFinder__NObjFwuMaker[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       4,   19, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      84,   38,   37,   37, 0x02,

 // properties: name, type, flags
     129,  121, 0x0a095003,
     137,  121, 0x0a095003,
     149,  145, 0x02095003,
     162,  121, 0x0a095003,

       0        // eod
};

static const char qt_meta_stringdata_HiLevelTests__HwFinder__NObjFwuMaker[] = {
    "HiLevelTests::HwFinder::NObjFwuMaker\0"
    "\0fwuName,revision,boardTypeRange,boardNumRange\0"
    "MakeFwu(QString,int,QString,QString)\0"
    "QString\0LdrPath\0LdrFile\0int\0DummyLdrSize\0"
    "FwuPath\0"
};

const QMetaObject HiLevelTests::HwFinder::NObjFwuMaker::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_HiLevelTests__HwFinder__NObjFwuMaker,
      qt_meta_data_HiLevelTests__HwFinder__NObjFwuMaker, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HiLevelTests::HwFinder::NObjFwuMaker::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HiLevelTests::HwFinder::NObjFwuMaker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HiLevelTests::HwFinder::NObjFwuMaker::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HiLevelTests__HwFinder__NObjFwuMaker))
        return static_cast<void*>(const_cast< NObjFwuMaker*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int HiLevelTests::HwFinder::NObjFwuMaker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: MakeFwu((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        default: ;
        }
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = m_ldrPath; break;
        case 1: *reinterpret_cast< QString*>(_v) = getMainLdrFile(); break;
        case 2: *reinterpret_cast< int*>(_v) = m_dummyLdrSize; break;
        case 3: *reinterpret_cast< QString*>(_v) = m_FwuPath; break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_ldrPath = *reinterpret_cast< QString*>(_v); break;
        case 1: m_ldrFile = *reinterpret_cast< QString*>(_v); break;
        case 2: m_dummyLdrSize = *reinterpret_cast< int*>(_v); break;
        case 3: m_FwuPath = *reinterpret_cast< QString*>(_v); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<HiLevelTests::HwFinder::NObjFwuMaker> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<HiLevelTests::HwFinder::NObjFwuMaker> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
