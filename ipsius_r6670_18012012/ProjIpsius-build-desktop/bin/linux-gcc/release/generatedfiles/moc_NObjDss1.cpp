/****************************************************************************
** Meta object code from reading C++ file 'NObjDss1.h'
**
** Created: Tue Jan 24 14:57:04 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Dss1ToSip/NObjDss1.h"
#include "DRI/NamedObjectFactory.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NObjDss1.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dss1ToSip__NObjDss1[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
      14,   79, // properties
       1,  121, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      25,   21,   20,   20, 0x02,
      60,   38,   20,   20, 0x02,
      97,   86,   20,   20, 0x02,
     136,  122,   20,   20, 0x02,
     173,  165,   20,   20, 0x22,
     197,  165,   20,   20, 0x02,
     220,  165,   20,   20, 0x02,
     243,   20,   20,   20, 0x02,
     262,  257,   20,   20, 0x02,
     283,  165,   20,   20, 0x02,
     313,   20,   20,   20, 0x02,
     332,  165,   20,   20, 0x02,
     362,   20,   20,   20, 0x02,

 // properties: name, type, flags
     389,  381, 0x0a095001,
     399,  395, 0x02095001,
     415,  381, 0x0a095001,
     430,  381, 0x0a095001,
     447,  381, 0x0a095001,
     454,  395, 0x02095003,
     467,  395, 0x02095003,
     492,  487, 0x01095003,
     509,  487, 0x01095003,
     534,  487, 0x01095003,
     548,  381, 0x0a095003,
     566,  487, 0x01095003,
     577,  487, 0x01095003,
     588,  395, 0x02095003,

 // enums: name, flags, count, data
     606, 0x0,    3,  125,

 // enum data: key, value
     616, uint(Dss1ToSip::NObjDss1::NotActive),
     626, uint(Dss1ToSip::NObjDss1::Active),
     633, uint(Dss1ToSip::NObjDss1::WaitRestartComplete),

       0        // eod
};

static const char qt_meta_stringdata_Dss1ToSip__NObjDss1[] = {
    "Dss1ToSip::NObjDss1\0\0par\0Enable(bool)\0"
    "boardAlias,intfNumber\0AddInterface(QString,int)\0"
    "boardAlias\0DeleteInterface(QString)\0"
    "pOutput,brief\0Calls(DRI::ICmdOutput*,bool)\0"
    "pOutput\0Calls(DRI::ICmdOutput*)\0"
    "Info(DRI::ICmdOutput*)\0Stat(DRI::ICmdOutput*)\0"
    "RestartStat()\0file\0CallsDetail(QString)\0"
    "L2Statistic(DRI::ICmdOutput*)\0"
    "ClearL2Statistic()\0L3Statistic(DRI::ICmdOutput*)\0"
    "ClearL3Statistic()\0QString\0State\0int\0"
    "ActivationCount\0LastActivation\0"
    "LastDeactivation\0HwType\0SigInterface\0"
    "AddressCompleteSize\0bool\0SendAlertingTone\0"
    "UseSharpAsNumberComplete\0RelAnonymCall\0"
    "AnonymCallingAddr\0RestartReq\0L2TraceInd\0"
    "WaitAnswerTimeout\0StateType\0NotActive\0"
    "Active\0WaitRestartComplete\0"
};

const QMetaObject Dss1ToSip::NObjDss1::staticMetaObject = {
    { &Domain::NamedObject::staticMetaObject, qt_meta_stringdata_Dss1ToSip__NObjDss1,
      qt_meta_data_Dss1ToSip__NObjDss1, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dss1ToSip::NObjDss1::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dss1ToSip::NObjDss1::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dss1ToSip::NObjDss1::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dss1ToSip__NObjDss1))
        return static_cast<void*>(const_cast< NObjDss1*>(this));
    if (!strcmp(_clname, "DRI::INonCreatable"))
        return static_cast< DRI::INonCreatable*>(const_cast< NObjDss1*>(this));
    if (!strcmp(_clname, "ObjLink::IObjectLinksHost"))
        return static_cast< ObjLink::IObjectLinksHost*>(const_cast< NObjDss1*>(this));
    if (!strcmp(_clname, "ISDN::ICallbackDss"))
        return static_cast< ISDN::ICallbackDss*>(const_cast< NObjDss1*>(this));
    if (!strcmp(_clname, "IGateDss1"))
        return static_cast< IGateDss1*>(const_cast< NObjDss1*>(this));
    if (!strcmp(_clname, "IGateDss1ForCall"))
        return static_cast< IGateDss1ForCall*>(const_cast< NObjDss1*>(this));
    if (!strcmp(_clname, "ISDN::IDssToGroup"))
        return static_cast< ISDN::IDssToGroup*>(const_cast< NObjDss1*>(this));
    if (!strcmp(_clname, "IBoardInfoReq"))
        return static_cast< IBoardInfoReq*>(const_cast< NObjDss1*>(this));
    typedef Domain::NamedObject QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Dss1ToSip::NObjDss1::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Domain::NamedObject QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Enable((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: AddInterface((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: DeleteInterface((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: Calls((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: Calls((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 5: Info((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 6: Stat((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 7: RestartStat(); break;
        case 8: CallsDetail((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: L2Statistic((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 10: ClearL2Statistic(); break;
        case 11: L3Statistic((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 12: ClearL3Statistic(); break;
        default: ;
        }
        _id -= 13;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = GetStateStr(); break;
        case 1: *reinterpret_cast< int*>(_v) = m_activationCount; break;
        case 2: *reinterpret_cast< QString*>(_v) = LastActivation(); break;
        case 3: *reinterpret_cast< QString*>(_v) = LastDeactivation(); break;
        case 4: *reinterpret_cast< QString*>(_v) = HardwareType(); break;
        case 5: *reinterpret_cast< int*>(_v) = SigInterface(); break;
        case 6: *reinterpret_cast< int*>(_v) = m_addressCompleteSize; break;
        case 7: *reinterpret_cast< bool*>(_v) = m_sendAlertingTone; break;
        case 8: *reinterpret_cast< bool*>(_v) = m_useSharpAsNumberComplete; break;
        case 9: *reinterpret_cast< bool*>(_v) = m_relAnonymCall; break;
        case 10: *reinterpret_cast< QString*>(_v) = AnonymCallingAddr(); break;
        case 11: *reinterpret_cast< bool*>(_v) = m_restartReq; break;
        case 12: *reinterpret_cast< bool*>(_v) = L2TraceInd(); break;
        case 13: *reinterpret_cast< int*>(_v) = m_waitAnswerTimeout; break;
        }
        _id -= 14;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 5: SigInterface(*reinterpret_cast< int*>(_v)); break;
        case 6: m_addressCompleteSize = *reinterpret_cast< int*>(_v); break;
        case 7: m_sendAlertingTone = *reinterpret_cast< bool*>(_v); break;
        case 8: m_useSharpAsNumberComplete = *reinterpret_cast< bool*>(_v); break;
        case 9: m_relAnonymCall = *reinterpret_cast< bool*>(_v); break;
        case 10: AnonymCallingAddr(*reinterpret_cast< QString*>(_v)); break;
        case 11: m_restartReq = *reinterpret_cast< bool*>(_v); break;
        case 12: L2TraceInd(*reinterpret_cast< bool*>(_v)); break;
        case 13: m_waitAnswerTimeout = *reinterpret_cast< int*>(_v); break;
        }
        _id -= 14;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 14;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// ---------------------------------------------- 
// NObj register 

namespace
{
	DRI::NamedObjectTypeRegister<Dss1ToSip::NObjDss1> GReg_0;
}; // end namespace 

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Dss1ToSip::NObjDss1> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
