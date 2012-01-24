/****************************************************************************
** Meta object code from reading C++ file 'NObjWrongL3PacketTestSettings.h'
**
** Created: Tue Jan 24 14:58:07 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IsdnTest/NObjWrongL3PacketTestSettings.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjWrongL3PacketTestSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IsdnTest__NObjWrongL3PacketTestSettings[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       5,   19, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      49,   41,   40,   40, 0x02,

 // properties: name, type, flags
      71,   67, 0x02095001,
      87,   67, 0x02095001,
     103,   67, 0x02095003,
     115,   67, 0x02095003,
     130,   67, 0x02095003,

       0        // eod
};

static const char qt_meta_stringdata_IsdnTest__NObjWrongL3PacketTestSettings[] = {
    "IsdnTest::NObjWrongL3PacketTestSettings\0"
    "\0from,to\0LenRange(int,int)\0int\0"
    "MinLenRndPacket\0MaxLenRndPacket\0"
    "DurationSec\0RndPackPercent\0VerboseTimeSec\0"
};

const QMetaObject IsdnTest::NObjWrongL3PacketTestSettings::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_IsdnTest__NObjWrongL3PacketTestSettings,
      qt_meta_data_IsdnTest__NObjWrongL3PacketTestSettings, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IsdnTest::NObjWrongL3PacketTestSettings::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IsdnTest::NObjWrongL3PacketTestSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IsdnTest::NObjWrongL3PacketTestSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IsdnTest__NObjWrongL3PacketTestSettings))
        return static_cast<void*>(const_cast< NObjWrongL3PacketTestSettings*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjWrongL3PacketTestSettings*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int IsdnTest::NObjWrongL3PacketTestSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: LenRange((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = m_MinLen; break;
        case 1: *reinterpret_cast< int*>(_v) = m_MaxLen; break;
        case 2: *reinterpret_cast< int*>(_v) = m_Duration; break;
        case 3: *reinterpret_cast< int*>(_v) = m_rndPackPercent; break;
        case 4: *reinterpret_cast< int*>(_v) = m_verboseTimeSec; break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 2: m_Duration = *reinterpret_cast< int*>(_v); break;
        case 3: m_rndPackPercent = *reinterpret_cast< int*>(_v); break;
        case 4: m_verboseTimeSec = *reinterpret_cast< int*>(_v); break;
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
	DRI::NamedObjectTypeRegister<IsdnTest::NObjWrongL3PacketTestSettings> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IsdnTest::NObjWrongL3PacketTestSettings> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
