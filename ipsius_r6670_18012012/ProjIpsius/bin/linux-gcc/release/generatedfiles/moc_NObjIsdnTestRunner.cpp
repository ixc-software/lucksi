/****************************************************************************
** Meta object code from reading C++ file 'NObjIsdnTestRunner.h'
**
** Created: Tue Jan 24 17:49:34 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/IsdnTest/NObjIsdnTestRunner.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjIsdnTestRunner.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IsdnTest__NObjIsdnTestRunner[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      30,   29,   29,   29, 0x02,
      46,   29,   29,   29, 0x02,
      72,   29,   29,   29, 0x02,
     115,   29,   29,   29, 0x02,
     137,   29,   29,   29, 0x02,
     160,   29,   29,   29, 0x02,
     183,   29,   29,   29, 0x02,
     204,   29,   29,   29, 0x02,
     228,   29,   29,   29, 0x02,
     254,   29,   29,   29, 0x02,
     279,   29,   29,   29, 0x02,
     308,  298,   29,   29, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_IsdnTest__NObjIsdnTestRunner[] = {
    "IsdnTest::NObjIsdnTestRunner\0\0"
    "BeginLapdTest()\0BeginCheckBchannelsBusy()\0"
    "BeginCheckConnectionDisconnectionRoutine()\0"
    "BeginCheckReloadDrv()\0BeginCheckReloadLapd()\0"
    "BeginCheckReloadUser()\0BeginSimulLiveTest()\0"
    "BeginDeactivationTest()\0"
    "BeginReactivationL3Test()\0"
    "BeginWrongL3PacketTest()\0BeginRestartTest()\0"
    "pAsyncCmd\0Run(DRI::IAsyncCmd*)\0"
};

const QMetaObject IsdnTest::NObjIsdnTestRunner::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_IsdnTest__NObjIsdnTestRunner,
      qt_meta_data_IsdnTest__NObjIsdnTestRunner, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IsdnTest::NObjIsdnTestRunner::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IsdnTest::NObjIsdnTestRunner::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IsdnTest::NObjIsdnTestRunner::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IsdnTest__NObjIsdnTestRunner))
        return static_cast<void*>(const_cast< NObjIsdnTestRunner*>(this));
    if (!strcmp(_clname, "ITestResult"))
        return static_cast< ITestResult*>(const_cast< NObjIsdnTestRunner*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int IsdnTest::NObjIsdnTestRunner::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: BeginLapdTest(); break;
        case 1: BeginCheckBchannelsBusy(); break;
        case 2: BeginCheckConnectionDisconnectionRoutine(); break;
        case 3: BeginCheckReloadDrv(); break;
        case 4: BeginCheckReloadLapd(); break;
        case 5: BeginCheckReloadUser(); break;
        case 6: BeginSimulLiveTest(); break;
        case 7: BeginDeactivationTest(); break;
        case 8: BeginReactivationL3Test(); break;
        case 9: BeginWrongL3PacketTest(); break;
        case 10: BeginRestartTest(); break;
        case 11: Run((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 12;
    }
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<IsdnTest::NObjIsdnTestRunner> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<IsdnTest::NObjIsdnTestRunner> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
