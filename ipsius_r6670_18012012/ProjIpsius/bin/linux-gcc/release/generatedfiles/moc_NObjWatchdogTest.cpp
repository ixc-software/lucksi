/****************************************************************************
** Meta object code from reading C++ file 'NObjWatchdogTest.h'
**
** Created: Tue Jan 24 17:49:14 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iCmpExt/NObjWatchdogTest.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjWatchdogTest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iCmpExt__NObjWatchdogTest[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      42,   27,   26,   26, 0x02,
      85,   27,   26,   26, 0x02,
     127,   27,   26,   26, 0x02,
     168,   27,   26,   26, 0x02,
     204,   27,   26,   26, 0x02,
     245,   27,   26,   26, 0x02,
     283,   27,   26,   26, 0x02,
     339,  318,   26,   26, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_iCmpExt__NObjWatchdogTest[] = {
    "iCmpExt::NObjWatchdogTest\0\0pAsyncCmd,addr\0"
    "SmallWatchdogTest(DRI::IAsyncCmd*,QString)\0"
    "HugeWatchdogTest(DRI::IAsyncCmd*,QString)\0"
    "LoopForeverTest(DRI::IAsyncCmd*,QString)\0"
    "AssertTest(DRI::IAsyncCmd*,QString)\0"
    "OutOfMemoryTest(DRI::IAsyncCmd*,QString)\0"
    "EchoHaltTest(DRI::IAsyncCmd*,QString)\0"
    "StateInfo(DRI::IAsyncCmd*,QString)\0"
    "pAsyncCmd,addr,times\0"
    "RunWatchdogLoop(DRI::IAsyncCmd*,QString,int)\0"
};

const QMetaObject iCmpExt::NObjWatchdogTest::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_iCmpExt__NObjWatchdogTest,
      qt_meta_data_iCmpExt__NObjWatchdogTest, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iCmpExt::NObjWatchdogTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iCmpExt::NObjWatchdogTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iCmpExt::NObjWatchdogTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iCmpExt__NObjWatchdogTest))
        return static_cast<void*>(const_cast< NObjWatchdogTest*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int iCmpExt::NObjWatchdogTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SmallWatchdogTest((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: HugeWatchdogTest((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 2: LoopForeverTest((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: AssertTest((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 4: OutOfMemoryTest((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 5: EchoHaltTest((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 6: StateInfo((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 7: RunWatchdogLoop((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<iCmpExt::NObjWatchdogTest> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<iCmpExt::NObjWatchdogTest> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
