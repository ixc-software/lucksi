/****************************************************************************
** Meta object code from reading C++ file 'TelnetKeys.h'
**
** Created: Tue Jan 24 17:48:06 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/DRI/TelnetKeys.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TelnetKeys.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TelnetKey__Map[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       1,   14, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // enums: name, flags, count, data
      15, 0x0,   27,   18,

 // enum data: key, value
      21, uint(TelnetKey::Map::None),
      26, uint(TelnetKey::Map::BS),
      29, uint(TelnetKey::Map::Tab),
      33, uint(TelnetKey::Map::Escape),
      40, uint(TelnetKey::Map::Del),
      44, uint(TelnetKey::Map::Enter),
      50, uint(TelnetKey::Map::Up),
      53, uint(TelnetKey::Map::Down),
      58, uint(TelnetKey::Map::Left),
      63, uint(TelnetKey::Map::Right),
      69, uint(TelnetKey::Map::Ins),
      73, uint(TelnetKey::Map::Home),
      78, uint(TelnetKey::Map::End),
      82, uint(TelnetKey::Map::PageUp),
      89, uint(TelnetKey::Map::PageDown),
      98, uint(TelnetKey::Map::F1),
     101, uint(TelnetKey::Map::F2),
     104, uint(TelnetKey::Map::F3),
     107, uint(TelnetKey::Map::F4),
     110, uint(TelnetKey::Map::F5),
     113, uint(TelnetKey::Map::F6),
     116, uint(TelnetKey::Map::F7),
     119, uint(TelnetKey::Map::F8),
     122, uint(TelnetKey::Map::F9),
     125, uint(TelnetKey::Map::F10),
     129, uint(TelnetKey::Map::F11),
     133, uint(TelnetKey::Map::F12),

       0        // eod
};

static const char qt_meta_stringdata_TelnetKey__Map[] = {
    "TelnetKey::Map\0KeyID\0None\0BS\0Tab\0"
    "Escape\0Del\0Enter\0Up\0Down\0Left\0Right\0"
    "Ins\0Home\0End\0PageUp\0PageDown\0F1\0F2\0"
    "F3\0F4\0F5\0F6\0F7\0F8\0F9\0F10\0F11\0F12\0"
};

const QMetaObject TelnetKey::Map::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_TelnetKey__Map,
      qt_meta_data_TelnetKey__Map, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TelnetKey::Map::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TelnetKey::Map::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TelnetKey::Map::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TelnetKey__Map))
        return static_cast<void*>(const_cast< Map*>(this));
    return QObject::qt_metacast(_clname);
}

int TelnetKey::Map::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}

// ---------------------------------------------- 
// QObject register 

namespace
{
	Utils::QObjFactoryRegister<TelnetKey::Map> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
