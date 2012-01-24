/****************************************************************************
** Meta object code from reading C++ file 'NamedObject.h'
**
** Created: Tue Jan 24 14:56:29 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/Domain/NamedObject.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NamedObject.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Domain__NamedObject[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      40,   14, // methods
       1,  214, // properties
       1,  217, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      35,   21,   20,   20, 0x00,
      85,   76,   20,   20, 0x20,
     118,   76,   20,   20, 0x00,
     180,  155,   20,   20, 0x00,
     220,   76,   20,   20, 0x20,
     255,   76,   20,   20, 0x00,
     288,   76,   20,   20, 0x00,
     345,  325,   20,   20, 0x00,
     421,  386,   20,   20, 0x00,
     489,  471,   20,   20, 0x20,
     545,  531,   20,   20, 0x00,
     588,   76,   20,   20, 0x20,
     623,   76,   20,   20, 0x00,
     657,   76,   20,   20, 0x00,
     735,  691,   20,   20, 0x00,
     823,  792,   20,   20, 0x20,
     895,  875,   20,   20, 0x20,
     949,  939,   20,   20, 0x00,
    1012,  976,   20,   20, 0x00,
    1070, 1047,   20,   20, 0x20,
    1109, 1101,   20,   20, 0x20,
    1166, 1135,   20,   20, 0x00,
    1223, 1205,   20,   20, 0x20,
    1254, 1101,   20,   20, 0x20,
    1308, 1280,   20,   20, 0x00,
    1371, 1354,   20,   20, 0x20,
    1409, 1101,   20,   20, 0x20,
    1469, 1439,   20,   20, 0x00,
    1514, 1354,   20,   20, 0x20,
    1551, 1101,   20,   20, 0x20,
    1606, 1580,   20,   20, 0x00,
    1650, 1354,   20,   20, 0x20,
    1686, 1101,   20,   20, 0x20,
    1745, 1714,   20,   20, 0x00,
    1789, 1354,   20,   20, 0x20,
    1828, 1101,   20,   20, 0x20,
    1904, 1859,   20,   20, 0x00,
    1978, 1947,   20,   20, 0x20,
    2033, 2016,   20,   20, 0x20,
    2066, 1101,   20,   20, 0x20,

 // properties: name, type, flags
    2098, 2094, 0x02095003,

 // enums: name, flags, count, data
    2107, 0x0,    3,  221,

 // enum data: key, value
    2114, uint(Domain::NamedObject::ErrorTag),
    2123, uint(Domain::NamedObject::WarningTag),
    2134, uint(Domain::NamedObject::InfoTag),

       0        // eod
};

static const char qt_meta_stringdata_Domain__NamedObject[] = {
    "Domain::NamedObject\0\0pContext,path\0"
    "CS_Set(DRI::ISessionCmdContext*,QString)\0"
    "pContext\0CS_Set(DRI::ISessionCmdContext*)\0"
    "CS_SetBack(DRI::ISessionCmdContext*)\0"
    "pContext,continueOnError\0"
    "CS_Begin(DRI::ISessionCmdContext*,bool)\0"
    "CS_Begin(DRI::ISessionCmdContext*)\0"
    "CS_End(DRI::ISessionCmdContext*)\0"
    "CS_Discard(DRI::ISessionCmdContext*)\0"
    "pContext,intervalMs\0"
    "CS_Timeout(DRI::ISessionCmdContext*,int)\0"
    "pContext,fileName,macroReplaceList\0"
    "CS_Exec(DRI::ISessionCmdContext*,QString,QString)\0"
    "pContext,fileName\0"
    "CS_Exec(DRI::ISessionCmdContext*,QString)\0"
    "pContext,text\0CS_Print(DRI::ISessionCmdContext*,QString)\0"
    "CS_Print(DRI::ISessionCmdContext*)\0"
    "CS_Exit(DRI::ISessionCmdContext*)\0"
    "CS_Info(DRI::ISessionCmdContext*)\0"
    "pContext,objectType,objectName,setAsCurrent\0"
    "ObjCreate(DRI::ISessionCmdContext*,QString,QString,bool)\0"
    "pContext,objectType,objectName\0"
    "ObjCreate(DRI::ISessionCmdContext*,QString,QString)\0"
    "pContext,objectType\0"
    "ObjCreate(DRI::ISessionCmdContext*,QString)\0"
    "pAsyncCmd\0ObjDelete(DRI::IAsyncCmd*)\0"
    "pOutput,listProperties,recurseLevel\0"
    "ObjList(DRI::ICmdOutput*,bool,int)\0"
    "pOutput,listProperties\0"
    "ObjList(DRI::ICmdOutput*,bool)\0pOutput\0"
    "ObjList(DRI::ICmdOutput*)\0"
    "pOutput,withTypes,propertyName\0"
    "ObjView(DRI::ICmdOutput*,bool,QString)\0"
    "pOutput,withTypes\0ObjView(DRI::ICmdOutput*,bool)\0"
    "ObjView(DRI::ICmdOutput*)\0"
    "pOutput,typeName,methodName\0"
    "MetaMethods(DRI::ICmdOutput*,QString,QString)\0"
    "pOutput,typeName\0MetaMethods(DRI::ICmdOutput*,QString)\0"
    "MetaMethods(DRI::ICmdOutput*)\0"
    "pOutput,typeName,propertyName\0"
    "MetaFields(DRI::ICmdOutput*,QString,QString)\0"
    "MetaFields(DRI::ICmdOutput*,QString)\0"
    "MetaFields(DRI::ICmdOutput*)\0"
    "pOutput,typeName,enumName\0"
    "MetaEnums(DRI::ICmdOutput*,QString,QString)\0"
    "MetaEnums(DRI::ICmdOutput*,QString)\0"
    "MetaEnums(DRI::ICmdOutput*)\0"
    "pOutput,typeName,withInherited\0"
    "MetaTypeInfo(DRI::ICmdOutput*,QString,bool)\0"
    "MetaTypeInfo(DRI::ICmdOutput*,QString)\0"
    "MetaTypeInfo(DRI::ICmdOutput*)\0"
    "pOutput,fullInfo,withInherited,onlyCreatable\0"
    "MetaTypes(DRI::ICmdOutput*,bool,bool,bool)\0"
    "pOutput,fullInfo,withInherited\0"
    "MetaTypes(DRI::ICmdOutput*,bool,bool)\0"
    "pOutput,fullInfo\0MetaTypes(DRI::ICmdOutput*,bool)\0"
    "MetaTypes(DRI::ICmdOutput*)\0int\0"
    "LogLevel\0LogTag\0ErrorTag\0WarningTag\0"
    "InfoTag\0"
};

const QMetaObject Domain::NamedObject::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Domain__NamedObject,
      qt_meta_data_Domain__NamedObject, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Domain::NamedObject::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Domain::NamedObject::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Domain::NamedObject::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Domain__NamedObject))
        return static_cast<void*>(const_cast< NamedObject*>(this));
    if (!strcmp(_clname, "iCore::MsgObject"))
        return static_cast< iCore::MsgObject*>(const_cast< NamedObject*>(this));
    if (!strcmp(_clname, "IDomain"))
        return static_cast< IDomain*>(const_cast< NamedObject*>(this));
    if (!strcmp(_clname, "IPropertyWriteEvent"))
        return static_cast< IPropertyWriteEvent*>(const_cast< NamedObject*>(this));
    if (!strcmp(_clname, "IDomainExceptionHook"))
        return static_cast< IDomainExceptionHook*>(const_cast< NamedObject*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< NamedObject*>(this));
    return QObject::qt_metacast(_clname);
}

int Domain::NamedObject::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: CS_Set((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: CS_Set((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1]))); break;
        case 2: CS_SetBack((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1]))); break;
        case 3: CS_Begin((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: CS_Begin((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1]))); break;
        case 5: CS_End((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1]))); break;
        case 6: CS_Discard((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1]))); break;
        case 7: CS_Timeout((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: CS_Exec((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 9: CS_Exec((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 10: CS_Print((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 11: CS_Print((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1]))); break;
        case 12: CS_Exit((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1]))); break;
        case 13: CS_Info((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1]))); break;
        case 14: ObjCreate((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 15: ObjCreate((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 16: ObjCreate((*reinterpret_cast< DRI::ISessionCmdContext*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 17: ObjDelete((*reinterpret_cast< DRI::IAsyncCmd*(*)>(_a[1]))); break;
        case 18: ObjList((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 19: ObjList((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 20: ObjList((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 21: ObjView((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 22: ObjView((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 23: ObjView((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 24: MetaMethods((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 25: MetaMethods((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 26: MetaMethods((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 27: MetaFields((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 28: MetaFields((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 29: MetaFields((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 30: MetaEnums((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 31: MetaEnums((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 32: MetaEnums((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 33: MetaTypeInfo((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 34: MetaTypeInfo((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 35: MetaTypeInfo((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        case 36: MetaTypes((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 37: MetaTypes((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 38: MetaTypes((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 39: MetaTypes((*reinterpret_cast< DRI::ICmdOutput*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 40;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = LogLevelDri(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: LogLevelDri(*reinterpret_cast< int*>(_v)); break;
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
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<Domain::NamedObject> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
