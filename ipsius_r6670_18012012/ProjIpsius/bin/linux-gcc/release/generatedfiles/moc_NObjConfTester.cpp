/****************************************************************************
** Meta object code from reading C++ file 'NObjConfTester.h'
**
** Created: Tue Jan 24 17:48:51 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/HiLevelTests/ConfTest/NObjConfTester.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjConfTester.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HiLevelTests__ConfTest__NObjConfTester[] = {

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
      74,   40,   39,   39, 0x02,

 // properties: name, type, flags
     120,  116, 0x02095003,
     128,  116, 0x02095003,
     134,  116, 0x02095003,
     140,  116, 0x02095003,
     151,  116, 0x02095003,

       0        // eod
};

static const char qt_meta_stringdata_HiLevelTests__ConfTest__NObjConfTester[] = {
    "HiLevelTests::ConfTest::NObjConfTester\0"
    "\0pAsyncCmd,board1,board2,useRtpSrc\0"
    "Run(DRI::IAsyncCmd*,QString,QString,bool)\0"
    "int\0Channel\0LFreq\0RFreq\0MinRtpPort\0"
    "MaxRtpPort\0"
};

const QMetaObject HiLevelTests::ConfTest::NObjConfTester::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_HiLevelTests__ConfTest__NObjConfTester,
      qt_meta_data_HiLevelTests__ConfTest__NObjConfTester, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HiLevelTests::ConfTest::NObjConfTester::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HiLevelTests::ConfTest::NObjConfTester::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HiLevelTests::ConfTest::NObjConfTester::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HiLevelTests__ConfTest__NObjConfTester))
        return static_cast<void*>(const_cast< NObjConfTester*>(this));
    if (!strcmp(_clname, "ConfTestProfile"))
        return static_cast< ConfTestProfile*>(const_cast< NObjConfTester*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int HiLevelTests::ConfTest::NObjConfTester::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Run((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        default: ;
        }
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = m_channel; break;
        case 1: *reinterpret_cast< int*>(_v) = m_lFreq; break;
        case 2: *reinterpret_cast< int*>(_v) = m_rFreq; break;
        case 3: *reinterpret_cast< int*>(_v) = m_minRtpPort; break;
        case 4: *reinterpret_cast< int*>(_v) = m_maxRtpPort; break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_channel = *reinterpret_cast< int*>(_v); break;
        case 1: m_lFreq = *reinterpret_cast< int*>(_v); break;
        case 2: m_rFreq = *reinterpret_cast< int*>(_v); break;
        case 3: m_minRtpPort = *reinterpret_cast< int*>(_v); break;
        case 4: m_maxRtpPort = *reinterpret_cast< int*>(_v); break;
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
	DRI::NamedObjectTypeRegister<HiLevelTests::ConfTest::NObjConfTester> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<HiLevelTests::ConfTest::NObjConfTester> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
