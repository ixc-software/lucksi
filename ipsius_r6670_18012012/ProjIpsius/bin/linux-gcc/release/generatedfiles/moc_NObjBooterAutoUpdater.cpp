/****************************************************************************
** Meta object code from reading C++ file 'NObjBooterAutoUpdater.h'
**
** Created: Tue Jan 24 17:47:43 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IpsiusService/NObjBooterAutoUpdater.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjBooterAutoUpdater.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IpsiusService__NObjBooterAutoUpdater[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       1,   19, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      60,   38,   37,   37, 0x02,

 // properties: name, type, flags
      97,   89, 0x0a095003,

       0        // eod
};

static const char qt_meta_stringdata_IpsiusService__NObjBooterAutoUpdater[] = {
    "IpsiusService::NObjBooterAutoUpdater\0"
    "\0pCmd,minVer,accumMsec\0"
    "Run(DRI::IAsyncCmd*,int,int)\0QString\0"
    "BootIamge\0"
};

const QMetaObject IpsiusService::NObjBooterAutoUpdater::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_IpsiusService__NObjBooterAutoUpdater,
      qt_meta_data_IpsiusService__NObjBooterAutoUpdater, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IpsiusService::NObjBooterAutoUpdater::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IpsiusService::NObjBooterAutoUpdater::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IpsiusService::NObjBooterAutoUpdater::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IpsiusService__NObjBooterAutoUpdater))
        return static_cast<void*>(const_cast< NObjBooterAutoUpdater*>(this));
    if (!strcmp(_clname, "IBootUpdater"))
        return static_cast< IBootUpdater*>(const_cast< NObjBooterAutoUpdater*>(this));
    if (!strcmp(_clname, "BfBootCli::IBroadcastReceiverToOwner"))
        return static_cast< BfBootCli::IBroadcastReceiverToOwner*>(const_cast< NObjBooterAutoUpdater*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int IpsiusService::NObjBooterAutoUpdater::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Run((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = m_imgFile; break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_imgFile = *reinterpret_cast< QString*>(_v); break;
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
	DRI::NamedObjectTypeRegister<IpsiusService::NObjBooterAutoUpdater> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IpsiusService::NObjBooterAutoUpdater> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
