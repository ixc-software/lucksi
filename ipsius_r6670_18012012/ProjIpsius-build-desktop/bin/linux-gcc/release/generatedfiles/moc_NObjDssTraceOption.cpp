/****************************************************************************
** Meta object code from reading C++ file 'NObjDssTraceOption.h'
**
** Created: Tue Jan 24 14:57:50 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/ISDN/NObjDssTraceOption.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjDssTraceOption.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ISDN__NObjDssTraceOption[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       5,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      30,   25, 0x01095003,
      38,   25, 0x01095003,
      48,   25, 0x01095003,
      55,   25, 0x01095003,
      60,   25, 0x01095003,

       0        // eod
};

static const char qt_meta_stringdata_ISDN__NObjDssTraceOption[] = {
    "ISDN::NObjDssTraceOption\0bool\0Enabled\0"
    "IeContent\0IeList\0Data\0Calls\0"
};

const QMetaObject ISDN::NObjDssTraceOption::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_ISDN__NObjDssTraceOption,
      qt_meta_data_ISDN__NObjDssTraceOption, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ISDN::NObjDssTraceOption::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ISDN::NObjDssTraceOption::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ISDN::NObjDssTraceOption::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ISDN__NObjDssTraceOption))
        return static_cast<void*>(const_cast< NObjDssTraceOption*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjDssTraceOption*>(this));
    if (!strcmp(_clname, "DssTraceOption"))
        return static_cast< DssTraceOption*>(const_cast< NObjDssTraceOption*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int ISDN::NObjDssTraceOption::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_traceOn; break;
        case 1: *reinterpret_cast< bool*>(_v) = m_traceIeContent; break;
        case 2: *reinterpret_cast< bool*>(_v) = m_traceIeList; break;
        case 3: *reinterpret_cast< bool*>(_v) = m_traceDataExcangeL3; break;
        case 4: *reinterpret_cast< bool*>(_v) = m_traceDataExcangeL3Call; break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_traceOn = *reinterpret_cast< bool*>(_v); break;
        case 1: m_traceIeContent = *reinterpret_cast< bool*>(_v); break;
        case 2: m_traceIeList = *reinterpret_cast< bool*>(_v); break;
        case 3: m_traceDataExcangeL3 = *reinterpret_cast< bool*>(_v); break;
        case 4: m_traceDataExcangeL3Call = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 5;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<ISDN::NObjDssTraceOption> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<ISDN::NObjDssTraceOption> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
