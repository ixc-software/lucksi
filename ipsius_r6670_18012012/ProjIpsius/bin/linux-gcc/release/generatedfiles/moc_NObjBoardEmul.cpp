/****************************************************************************
** Meta object code from reading C++ file 'NObjBoardEmul.h'
**
** Created: Tue Jan 24 17:47:12 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/BfEmul/NObjBoardEmul.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjBoardEmul.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BfEmul__NObjBoardEmul[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       6,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      27,   22, 0x01095003,
      41,   37, 0x02095001,
      52,   37, 0x02095001,
      63,   22, 0x01095003,
      72,   37, 0x02095003,
      80,   22, 0x01095001,

       0        // eod
};

static const char qt_meta_stringdata_BfEmul__NObjBoardEmul[] = {
    "BfEmul::NObjBoardEmul\0bool\0UseBooter\0"
    "int\0MinRtpPort\0MaxRtpPort\0TraceInd\0"
    "SoftRev\0Enabled\0"
};

const QMetaObject BfEmul::NObjBoardEmul::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_BfEmul__NObjBoardEmul,
      qt_meta_data_BfEmul__NObjBoardEmul, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BfEmul::NObjBoardEmul::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BfEmul::NObjBoardEmul::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BfEmul::NObjBoardEmul::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BfEmul__NObjBoardEmul))
        return static_cast<void*>(const_cast< NObjBoardEmul*>(this));
    if (!strcmp(_clname, "Utils::IExitTaskObserver"))
        return static_cast< Utils::IExitTaskObserver*>(const_cast< NObjBoardEmul*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjBoardEmul*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int BfEmul::NObjBoardEmul::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_useBooter; break;
        case 1: *reinterpret_cast< int*>(_v) = MinRtpPort(); break;
        case 2: *reinterpret_cast< int*>(_v) = MaxRtpPort(); break;
        case 3: *reinterpret_cast< bool*>(_v) = m_traceE1App; break;
        case 4: *reinterpret_cast< int*>(_v) = m_softVer; break;
        case 5: *reinterpret_cast< bool*>(_v) = m_enabled; break;
        }
        _id -= 6;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_useBooter = *reinterpret_cast< bool*>(_v); break;
        case 3: m_traceE1App = *reinterpret_cast< bool*>(_v); break;
        case 4: m_softVer = *reinterpret_cast< int*>(_v); break;
        }
        _id -= 6;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 6;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<BfEmul::NObjBoardEmul> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<BfEmul::NObjBoardEmul> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
