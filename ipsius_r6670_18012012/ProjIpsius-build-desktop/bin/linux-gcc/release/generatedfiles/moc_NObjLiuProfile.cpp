/****************************************************************************
** Meta object code from reading C++ file 'NObjLiuProfile.h'
**
** Created: Tue Jan 24 14:57:17 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Dss1ToSip/NObjLiuProfile.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjLiuProfile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dss1ToSip__NObjLiuProfile[] = {

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
      54,   26, 0x0009500b,
      88,   63, 0x0009500b,
      99,   94, 0x01095003,
     108,  104, 0x02095003,
     113,  104, 0x02095003,
     117,  104, 0x02095003,
     122,   94, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__NObjLiuProfile[] = {
    "Dss1ToSip::NObjLiuProfile\0"
    "Ds2155::Enums::LineCodeType\0LineCode\0"
    "Ds2155::Enums::CycleType\0Cycle\0bool\0"
    "CRC4\0int\0MFAS\0FAS\0NFAS\0SyncMaster\0"
};

#ifdef Q_NO_DATA_RELOCATION
static const QMetaObjectAccessor qt_meta_extradata_Dss1ToSip__NObjLiuProfile[] = {
        Ds2155::Enums::getStaticMetaObject,
#else
static const QMetaObject *qt_meta_extradata_Dss1ToSip__NObjLiuProfile[] = {
        &Ds2155::Enums::staticMetaObject,
#endif //Q_NO_DATA_RELOCATION
    0
};

static const QMetaObjectExtraData qt_meta_extradata2_Dss1ToSip__NObjLiuProfile = {
    qt_meta_extradata_Dss1ToSip__NObjLiuProfile, 0 
};

const QMetaObject Dss1ToSip::NObjLiuProfile::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__NObjLiuProfile,
      qt_meta_data_Dss1ToSip__NObjLiuProfile, &qt_meta_extradata2_Dss1ToSip__NObjLiuProfile }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::NObjLiuProfile::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::NObjLiuProfile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::NObjLiuProfile::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__NObjLiuProfile))
        return static_cast<void*>(const_cast< NObjLiuProfile*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjLiuProfile*>(this));
    if (!strcmp(_clname, "Ds2155::LiuProfile"))
        return static_cast< Ds2155::LiuProfile*>(const_cast< NObjLiuProfile*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Dss1ToSip::NObjLiuProfile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< Ds2155::Enums::LineCodeType*>(_v) = m_lineCode; break;
        case 1: *reinterpret_cast< Ds2155::Enums::CycleType*>(_v) = m_cycle; break;
        case 2: *reinterpret_cast< bool*>(_v) = m_enableCrc4; break;
        case 3: *reinterpret_cast< int*>(_v) = m_MFAS; break;
        case 4: *reinterpret_cast< int*>(_v) = m_FAS; break;
        case 5: *reinterpret_cast< int*>(_v) = m_NFAS; break;
        case 6: *reinterpret_cast< bool*>(_v) = m_syncMaster; break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_lineCode = *reinterpret_cast< Ds2155::Enums::LineCodeType*>(_v); break;
        case 1: m_cycle = *reinterpret_cast< Ds2155::Enums::CycleType*>(_v); break;
        case 2: m_enableCrc4 = *reinterpret_cast< bool*>(_v); break;
        case 3: m_MFAS = *reinterpret_cast< int*>(_v); break;
        case 4: m_FAS = *reinterpret_cast< int*>(_v); break;
        case 5: m_NFAS = *reinterpret_cast< int*>(_v); break;
        case 6: m_syncMaster = *reinterpret_cast< bool*>(_v); break;
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
static const uint qt_meta_data_Dss1ToSip__NObjLiuProfileHolder[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       1,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      37,   32, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__NObjLiuProfileHolder[] = {
    "Dss1ToSip::NObjLiuProfileHolder\0bool\0"
    "Enabled\0"
};

const QMetaObject Dss1ToSip::NObjLiuProfileHolder::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__NObjLiuProfileHolder,
      qt_meta_data_Dss1ToSip__NObjLiuProfileHolder, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::NObjLiuProfileHolder::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::NObjLiuProfileHolder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::NObjLiuProfileHolder::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__NObjLiuProfileHolder))
        return static_cast<void*>(const_cast< NObjLiuProfileHolder*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Dss1ToSip::NObjLiuProfileHolder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_enabled; break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: SetEnabled(*reinterpret_cast< bool*>(_v)); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<Dss1ToSip::NObjLiuProfile> GReg_0;
	DRI::NamedObjectTypeRegister<Dss1ToSip::NObjLiuProfileHolder> GReg_1;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Dss1ToSip::NObjLiuProfile> GRegQObj_0;
	Utils::QObjFactoryRegister<Dss1ToSip::NObjLiuProfileHolder> GRegQObj_1;
}; // end namespace 
QT_END_MOC_NAMESPACE
