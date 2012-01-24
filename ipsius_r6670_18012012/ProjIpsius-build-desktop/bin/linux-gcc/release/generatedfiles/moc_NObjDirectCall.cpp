/****************************************************************************
** Meta object code from reading C++ file 'NObjDirectCall.h'
**
** Created: Tue Jan 24 14:58:30 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/TestRealEcho/NObjDirectCall.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjDirectCall.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TestRealEcho__NObjDirectCall[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       7,   44, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      38,   30,   29,   29, 0x02,
      57,   52,   29,   29, 0x02,
      87,   75,   29,   29, 0x02,
     130,  115,   29,   29, 0x02,
     170,  152,   29,   29, 0x02,
     229,  210,   29,   29, 0x02,

 // properties: name, type, flags
     258,  254, 0x02095003,
     269,  254, 0x02095003,
     288,  280, 0x0a095003,
     298,  254, 0x02095003,
     306,  280, 0x0a095003,
     319,  314, 0x01095003,
     327,  280, 0x0a095001,

       0        // eod
};

static const char qt_meta_stringdata_TestRealEcho__NObjDirectCall[] = {
    "TestRealEcho::NObjDirectCall\0\0objName\0"
    "Init(QString)\0name\0SetCodec(QString)\0"
    "file,isWave\0SetSourceFile(QString,bool)\0"
    "amplitude,frec\0SetSourceGen(int,int)\0"
    "pAsyncCmd,timeout\0"
    "WaitInterfaceReady(DRI::IAsyncCmd*,int)\0"
    "pAsyncCmd,duration\0Run(DRI::IAsyncCmd*,int)\0"
    "int\0MinRtpPort\0MaxRtpPort\0QString\0"
    "LocalHost\0Channel\0OutFile\0bool\0WaveOut\0"
    "Codec\0"
};

const QMetaObject TestRealEcho::NObjDirectCall::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_TestRealEcho__NObjDirectCall,
      qt_meta_data_TestRealEcho__NObjDirectCall, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TestRealEcho::NObjDirectCall::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TestRealEcho::NObjDirectCall::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TestRealEcho::NObjDirectCall::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TestRealEcho__NObjDirectCall))
        return static_cast<void*>(const_cast< NObjDirectCall*>(this));
    if (!strcmp(_clname, "IDirectCallToOwner"))
        return static_cast< IDirectCallToOwner*>(const_cast< NObjDirectCall*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int TestRealEcho::NObjDirectCall::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Init((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: SetCodec((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: SetSourceFile((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: SetSourceGen((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: WaitInterfaceReady((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: Run((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 6;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = m_minRtpPort; break;
        case 1: *reinterpret_cast< int*>(_v) = m_maxRtpPort; break;
        case 2: *reinterpret_cast< QString*>(_v) = m_localHost; break;
        case 3: *reinterpret_cast< int*>(_v) = m_channel; break;
        case 4: *reinterpret_cast< QString*>(_v) = m_outFile; break;
        case 5: *reinterpret_cast< bool*>(_v) = m_waveOut; break;
        case 6: *reinterpret_cast< QString*>(_v) = CodecInfo(); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: m_minRtpPort = *reinterpret_cast< int*>(_v); break;
        case 1: m_maxRtpPort = *reinterpret_cast< int*>(_v); break;
        case 2: m_localHost = *reinterpret_cast< QString*>(_v); break;
        case 3: m_channel = *reinterpret_cast< int*>(_v); break;
        case 4: m_outFile = *reinterpret_cast< QString*>(_v); break;
        case 5: m_waveOut = *reinterpret_cast< bool*>(_v); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<TestRealEcho::NObjDirectCall> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<TestRealEcho::NObjDirectCall> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
