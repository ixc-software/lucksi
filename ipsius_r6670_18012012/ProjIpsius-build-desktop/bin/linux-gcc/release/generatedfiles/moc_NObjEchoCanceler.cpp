/****************************************************************************
** Meta object code from reading C++ file 'NObjEchoCanceler.h'
**
** Created: Tue Jan 24 14:58:32 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/TestRealEcho/NObjEchoCanceler.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjEchoCanceler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TestRealEcho__NObjEchoCanceler[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      59,   32,   31,   31, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_TestRealEcho__NObjEchoCanceler[] = {
    "TestRealEcho::NObjEchoCanceler\0\0"
    "txFile,rxFile,echoTaps,out\0"
    "Process(QString,QString,int,QString)\0"
};

const QMetaObject TestRealEcho::NObjEchoCanceler::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_TestRealEcho__NObjEchoCanceler,
      qt_meta_data_TestRealEcho__NObjEchoCanceler, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TestRealEcho::NObjEchoCanceler::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TestRealEcho::NObjEchoCanceler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TestRealEcho::NObjEchoCanceler::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TestRealEcho__NObjEchoCanceler))
        return static_cast<void*>(const_cast< NObjEchoCanceler*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int TestRealEcho::NObjEchoCanceler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Process((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<TestRealEcho::NObjEchoCanceler> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<TestRealEcho::NObjEchoCanceler> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
