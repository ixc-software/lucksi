/****************************************************************************
** Meta object code from reading C++ file 'ChDataCapture.h'
**
** Created: Tue Jan 24 14:57:28 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/iCmpExt/ChDataCapture.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ChDataCapture.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iCmpExt__ChDataCaptureMode[] = {

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
      27, 0x0,    3,   18,

 // enum data: key, value
      33, uint(iCmpExt::ChDataCaptureMode::None),
      38, uint(iCmpExt::ChDataCaptureMode::Bin),
      42, uint(iCmpExt::ChDataCaptureMode::Wav),

       0        // eod
};

static const char qt_meta_stringdata_iCmpExt__ChDataCaptureMode[] = {
    "iCmpExt::ChDataCaptureMode\0Value\0None\0"
    "Bin\0Wav\0"
};

const QMetaObject iCmpExt::ChDataCaptureMode::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_iCmpExt__ChDataCaptureMode,
      qt_meta_data_iCmpExt__ChDataCaptureMode, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iCmpExt::ChDataCaptureMode::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iCmpExt::ChDataCaptureMode::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iCmpExt::ChDataCaptureMode::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iCmpExt__ChDataCaptureMode))
        return static_cast<void*>(const_cast< ChDataCaptureMode*>(this));
    if (!strcmp(_clname, "boost::noncopyable"))
        return static_cast< boost::noncopyable*>(const_cast< ChDataCaptureMode*>(this));
    return QObject::qt_metacast(_clname);
}

int iCmpExt::ChDataCaptureMode::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<iCmpExt::ChDataCaptureMode> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
