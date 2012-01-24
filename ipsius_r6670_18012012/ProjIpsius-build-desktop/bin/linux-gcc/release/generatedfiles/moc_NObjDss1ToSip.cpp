/****************************************************************************
** Meta object code from reading C++ file 'NObjDss1ToSip.h'
**
** Created: Tue Jan 24 14:57:13 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Dss1ToSip/NObjDss1ToSip.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjDss1ToSip.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dss1ToSip__NObjDss1ToSip[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       3,   64, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      84,   26,   25,   25, 0x02,
     178,  133,   25,   25, 0x22,
     257,  223,   25,   25, 0x22,
     303,  298,   25,   25, 0x02,
     337,  323,   25,   25, 0x02,
     374,  366,   25,   25, 0x22,
     398,  366,   25,   25, 0x02,
     421,  366,   25,   25, 0x02,
     444,   25,   25,   25, 0x02,
     479,  458,   25,   25, 0x02,

 // properties: name, type, flags
     523,  518, 0x01095003,
     531,  518, 0x01095001,
     556,  548, 0x0a095003,

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__NObjDss1ToSip[] = {
    "Dss1ToSip::NObjDss1ToSip\0\0"
    "hardType,name,boardAlias,sideType,intfNumber,DChannelSlot\0"
    "AddDss1(QString,QString,QString,QString,int,int)\0"
    "hardType,name,boardAlias,sideType,intfNumber\0"
    "AddDss1(QString,QString,QString,QString,int)\0"
    "hardType,name,boardAlias,sideType\0"
    "AddDss1(QString,QString,QString,QString)\0"
    "name\0DeleteDss1(QString)\0pOutput,brief\0"
    "Calls(DRI::ICmdOutput*,bool)\0pOutput\0"
    "Calls(DRI::ICmdOutput*)\0Info(DRI::ICmdOutput*)\0"
    "Stat(DRI::ICmdOutput*)\0RestartStat()\0"
    "pAsyncCmd,maxTimeout\0"
    "WaitWhileNoactive(DRI::IAsyncCmd*,int)\0"
    "bool\0Enabled\0ActiveTrunkCount\0QString\0"
    "SipPoint\0"
};

const QMetaObject Dss1ToSip::NObjDss1ToSip::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__NObjDss1ToSip,
      qt_meta_data_Dss1ToSip__NObjDss1ToSip, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::NObjDss1ToSip::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::NObjDss1ToSip::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::NObjDss1ToSip::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__NObjDss1ToSip))
        return static_cast<void*>(const_cast< NObjDss1ToSip*>(this));
    if (!strcmp(_clname, "iSip::ISipAccessPointEvents"))
        return static_cast< iSip::ISipAccessPointEvents*>(const_cast< NObjDss1ToSip*>(this));
    if (!strcmp(_clname, "IGateDss1Events"))
        return static_cast< IGateDss1Events*>(const_cast< NObjDss1ToSip*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Dss1ToSip::NObjDss1ToSip::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: AddDss1((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])),(*reinterpret_cast< int(*)>(_a[6]))); break;
        case 1: AddDss1((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5]))); break;
        case 2: AddDss1((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4]))); break;
        case 3: DeleteDss1((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: Calls((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: Calls((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 6: Info((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 7: Stat((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 8: RestartStat(); break;
        case 9: WaitWhileNoactive((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 10;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = m_isEnabled; break;
        case 1: *reinterpret_cast< bool*>(_v) = m_activeTrunkCount; break;
        case 2: *reinterpret_cast< QString*>(_v) = m_sipPointName; break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: Enable(*reinterpret_cast< bool*>(_v)); break;
        case 2: SipPoint(*reinterpret_cast< QString*>(_v)); break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 3;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<Dss1ToSip::NObjDss1ToSip> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Dss1ToSip::NObjDss1ToSip> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
