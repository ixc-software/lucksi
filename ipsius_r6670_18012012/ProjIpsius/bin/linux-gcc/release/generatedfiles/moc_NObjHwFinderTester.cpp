/****************************************************************************
** Meta object code from reading C++ file 'NObjHwFinderTester.h'
**
** Created: Tue Jan 24 17:48:57 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/HiLevelTests/HwFinder/NObjHwFinderTester.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjHwFinderTester.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HiLevelTests__HwFinder__NObjHwFinderTester[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      74,   44,   43,   43, 0x02,
     134,  112,   43,   43, 0x22,
     178,  168,   43,   43, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_HiLevelTests__HwFinder__NObjHwFinderTester[] = {
    "HiLevelTests::HwFinder::NObjHwFinderTester\0"
    "\0taskName,hwNum,fwuVer,timeout\0"
    "AddTaskWaitAlloc(QString,int,int,int)\0"
    "taskName,hwNum,fwuVer\0"
    "AddTaskWaitAlloc(QString,int,int)\0"
    "pAsyncCmd\0StartAllTask(DRI::IAsyncCmd*)\0"
};

const QMetaObject HiLevelTests::HwFinder::NObjHwFinderTester::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_HiLevelTests__HwFinder__NObjHwFinderTester,
      qt_meta_data_HiLevelTests__HwFinder__NObjHwFinderTester, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HiLevelTests::HwFinder::NObjHwFinderTester::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HiLevelTests::HwFinder::NObjHwFinderTester::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HiLevelTests::HwFinder::NObjHwFinderTester::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HiLevelTests__HwFinder__NObjHwFinderTester))
        return static_cast<void*>(const_cast< NObjHwFinderTester*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int HiLevelTests::HwFinder::NObjHwFinderTester::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: AddTaskWaitAlloc((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 1: AddTaskWaitAlloc((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: StartAllTask((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<HiLevelTests::HwFinder::NObjHwFinderTester> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<HiLevelTests::HwFinder::NObjHwFinderTester> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
