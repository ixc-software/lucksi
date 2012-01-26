/****************************************************************************
** Meta object code from reading C++ file 'NObjSipTrustedNodes.h'
**
** Created: Tue Jan 24 17:50:05 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iSip/NObjSipTrustedNodes.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjSipTrustedNodes.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iSip__NObjSipTrustedNodes[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       1,   49, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      56,   27,   26,   26, 0x02,
     147,  123,   26,   26, 0x22,
     227,  210,   26,   26, 0x02,
     285,  273,   26,   26, 0x22,
     341,  327,   26,   26, 0x02,
     374,   26,   26,   26, 0x02,
     393,  385,   26,   26, 0x02,

 // properties: name, type, flags
     421,  417, 0x02095001,

       0        // eod
};

static const char qt_meta_stringdata_iSip__NObjSipTrustedNodes[] = {
    "iSip::NObjSipTrustedNodes\0\0"
    "pOutput,transport,value,port\0"
    "AddNode(DRI::ICmdOutput*,iSip::SipTransportInfo::Type,QString,int)\0"
    "pOutput,transport,value\0"
    "AddNode(DRI::ICmdOutput*,iSip::SipTransportInfo::Type,QString)\0"
    "pOutput,par,port\0"
    "IsItTrustedNode(DRI::ICmdOutput*,QString,int)\0"
    "pOutput,par\0IsItTrustedNode(DRI::ICmdOutput*,QString)\0"
    "pOutput,index\0DeleteNode(DRI::ICmdOutput*,int)\0"
    "ClearAll()\0pOutput\0Print(DRI::ICmdOutput*)\0"
    "int\0Size\0"
};

const QMetaObject iSip::NObjSipTrustedNodes::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iSip__NObjSipTrustedNodes,
      qt_meta_data_iSip__NObjSipTrustedNodes, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iSip::NObjSipTrustedNodes::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iSip::NObjSipTrustedNodes::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iSip::NObjSipTrustedNodes::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iSip__NObjSipTrustedNodes))
        return static_cast<void*>(const_cast< NObjSipTrustedNodes*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjSipTrustedNodes*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iSip::NObjSipTrustedNodes::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: AddNode((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< iSip::SipTransportInfo::Type(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 1: AddNode((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< iSip::SipTransportInfo::Type(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 2: IsItTrustedNode((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 3: IsItTrustedNode((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 4: DeleteNode((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: ClearAll(); break;
        case 6: Print((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 7;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = Size(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
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
	DRI::NamedObjectTypeRegister<iSip::NObjSipTrustedNodes> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iSip::NObjSipTrustedNodes> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
