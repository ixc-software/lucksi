/****************************************************************************
** Meta object code from reading C++ file 'IeConstants.h'
**
** Created: Tue Jan 24 14:57:41 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Src/ISDN/IeConstants.h"
#include "Utils/QObjFactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IeConstants.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ISDN__IeConstants[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
      19,   14, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // enums: name, flags, count, data
      18, 0x0,    2,   90,
      27, 0x0,    9,   94,
      36, 0x0,    6,  112,
      49, 0x0,    4,  124,
      64, 0x0,    6,  132,
      73, 0x0,    2,  144,
      83, 0x0,    7,  148,
      93, 0x0,    2,  162,
     102, 0x0,    2,  166,
     111, 0x0,    4,  170,
     122, 0x0,    2,  178,
     131, 0x0,    4,  182,
     140, 0x0,    7,  190,
     150, 0x0,    7,  204,
     158, 0x0,    4,  218,
     169, 0x0,    4,  226,
     182, 0x0,    9,  234,
     191, 0x0,   20,  252,
     203, 0x0,    3,  292,

 // enum data: key, value
     216, uint(ISDN::IeConstants::NT),
     219, uint(ISDN::IeConstants::TE),
     222, uint(ISDN::IeConstants::U),
     224, uint(ISDN::IeConstants::LPN),
     228, uint(ISDN::IeConstants::LN),
     231, uint(ISDN::IeConstants::TN),
     234, uint(ISDN::IeConstants::RLN),
     238, uint(ISDN::IeConstants::RPN),
     242, uint(ISDN::IeConstants::INTL),
     247, uint(ISDN::IeConstants::BI),
     250, uint(ISDN::IeConstants::SpecNonStandart),
     266, uint(ISDN::IeConstants::NotEndToEndISDN),
     282, uint(ISDN::IeConstants::DestAdrNonIsdn),
     297, uint(ISDN::IeConstants::OrigAdrNonIsdn),
     312, uint(ISDN::IeConstants::HasRetToIsdn),
     325, uint(ISDN::IeConstants::Interworking),
     338, uint(ISDN::IeConstants::InBand),
     345, uint(ISDN::IeConstants::CCITT),
     351, uint(ISDN::IeConstants::INTERNATIONAL),
     365, uint(ISDN::IeConstants::NATIONAL),
     374, uint(ISDN::IeConstants::NETWORK_SPECIFIC),
     391, uint(ISDN::IeConstants::SPEECH),
     398, uint(ISDN::IeConstants::DIGITAL),
     406, uint(ISDN::IeConstants::RESTRICTED_DIGITAL),
     425, uint(ISDN::IeConstants::AUDIO_3_1K),
     436, uint(ISDN::IeConstants::DIGITAL_W_TONES),
     452, uint(ISDN::IeConstants::VIDEO),
     458, uint(ISDN::IeConstants::Circuit),
     466, uint(ISDN::IeConstants::Packet),
     473, uint(ISDN::IeConstants::InPacket),
     482, uint(ISDN::IeConstants::KbPs64),
     489, uint(ISDN::IeConstants::KbPs128),
     497, uint(ISDN::IeConstants::KbPs384),
     505, uint(ISDN::IeConstants::KbPs1536),
     514, uint(ISDN::IeConstants::KbPs1920),
     523, uint(ISDN::IeConstants::MultiRate),
     533, uint(ISDN::IeConstants::Bri),
     537, uint(ISDN::IeConstants::Other),
     543, uint(ISDN::IeConstants::Preferred),
     553, uint(ISDN::IeConstants::Exclusive),
     563, uint(ISDN::IeConstants::NoChannel),
     573, uint(ISDN::IeConstants::B1orIndInFollowing),
     592, uint(ISDN::IeConstants::B2orReserved),
     605, uint(ISDN::IeConstants::AnyChannel),
     616, uint(ISDN::IeConstants::Num),
     620, uint(ISDN::IeConstants::Map),
     624, uint(ISDN::IeConstants::BChannels),
     634, uint(ISDN::IeConstants::H0),
     637, uint(ISDN::IeConstants::H11),
     641, uint(ISDN::IeConstants::H12),
     645, uint(ISDN::IeConstants::UnknownType),
     657, uint(ISDN::IeConstants::International),
     671, uint(ISDN::IeConstants::National),
     680, uint(ISDN::IeConstants::NetSpec),
     688, uint(ISDN::IeConstants::Subscriber),
     699, uint(ISDN::IeConstants::Abbreviated),
     711, uint(ISDN::IeConstants::Ext),
     715, uint(ISDN::IeConstants::UnknownPlan),
     727, uint(ISDN::IeConstants::ISDNtelephony),
     741, uint(ISDN::IeConstants::Data),
     746, uint(ISDN::IeConstants::Telex),
     752, uint(ISDN::IeConstants::NationalPlan),
     765, uint(ISDN::IeConstants::Private),
     773, uint(ISDN::IeConstants::ExtReserve),
     784, uint(ISDN::IeConstants::Allowed),
     792, uint(ISDN::IeConstants::Restricted),
     803, uint(ISDN::IeConstants::NotAvailable),
     816, uint(ISDN::IeConstants::PesenIndReserved),
     833, uint(ISDN::IeConstants::UserProv_NotScreened),
     854, uint(ISDN::IeConstants::UserProv_Passed),
     870, uint(ISDN::IeConstants::UserProv_Failed),
     886, uint(ISDN::IeConstants::NetProv),
     894, uint(ISDN::IeConstants::V110_X30),
     903, uint(ISDN::IeConstants::G711_PCMU),
     913, uint(ISDN::IeConstants::G711_PCMA),
     923, uint(ISDN::IeConstants::G721_ADPCM),
     934, uint(ISDN::IeConstants::H2xx),
     939, uint(ISDN::IeConstants::Non_CCITT),
     949, uint(ISDN::IeConstants::V120),
     954, uint(ISDN::IeConstants::X31_HDLC),
     963, uint(ISDN::IeConstants::UserInfoMax),
     975, uint(ISDN::IeConstants::DialTone),
     984, uint(ISDN::IeConstants::RingBackToneOn),
     999, uint(ISDN::IeConstants::InterceptToneOn),
    1015, uint(ISDN::IeConstants::NetCongestionToneOn),
    1035, uint(ISDN::IeConstants::BusyToneOn),
    1046, uint(ISDN::IeConstants::ConfirmToneOn),
    1060, uint(ISDN::IeConstants::AnswToneOn),
    1071, uint(ISDN::IeConstants::CallWaitingToneOn),
    1089, uint(ISDN::IeConstants::OffHookWarning),
    1104, uint(ISDN::IeConstants::PreemptionToneOn),
    1121, uint(ISDN::IeConstants::TonesOff),
    1130, uint(ISDN::IeConstants::AllertingOn_p0),
    1145, uint(ISDN::IeConstants::AllertingOn_p1),
    1160, uint(ISDN::IeConstants::AllertingOn_p2),
    1175, uint(ISDN::IeConstants::AllertingOn_p3),
    1190, uint(ISDN::IeConstants::AllertingOn_p4),
    1205, uint(ISDN::IeConstants::AllertingOn_p5),
    1220, uint(ISDN::IeConstants::AllertingOn_p6),
    1235, uint(ISDN::IeConstants::AllertingOn_p7),
    1250, uint(ISDN::IeConstants::AllertingOff),
    1263, uint(ISDN::IeConstants::Channels),
    1272, uint(ISDN::IeConstants::SingleIntf),
    1283, uint(ISDN::IeConstants::AllIntf),

       0        // eod
};

static const char qt_meta_stringdata_ISDN__IeConstants[] = {
    "ISDN::IeConstants\0SideType\0Location\0"
    "ProgressDscr\0CodingStandard\0TransCap\0"
    "TransMode\0TransRate\0IntfType\0PrefExcl\0"
    "ChanSelect\0NumOrMap\0ChanType\0TypeOfNum\0"
    "NumPlan\0PresentInd\0ScreeningInd\0"
    "UserInfo\0SignalValue\0RestartClass\0NT\0"
    "TE\0U\0LPN\0LN\0TN\0RLN\0RPN\0INTL\0BI\0"
    "SpecNonStandart\0NotEndToEndISDN\0"
    "DestAdrNonIsdn\0OrigAdrNonIsdn\0"
    "HasRetToIsdn\0Interworking\0InBand\0CCITT\0"
    "INTERNATIONAL\0NATIONAL\0NETWORK_SPECIFIC\0"
    "SPEECH\0DIGITAL\0RESTRICTED_DIGITAL\0"
    "AUDIO_3_1K\0DIGITAL_W_TONES\0VIDEO\0"
    "Circuit\0Packet\0InPacket\0KbPs64\0KbPs128\0"
    "KbPs384\0KbPs1536\0KbPs1920\0MultiRate\0"
    "Bri\0Other\0Preferred\0Exclusive\0NoChannel\0"
    "B1orIndInFollowing\0B2orReserved\0"
    "AnyChannel\0Num\0Map\0BChannels\0H0\0H11\0"
    "H12\0UnknownType\0International\0National\0"
    "NetSpec\0Subscriber\0Abbreviated\0Ext\0"
    "UnknownPlan\0ISDNtelephony\0Data\0Telex\0"
    "NationalPlan\0Private\0ExtReserve\0Allowed\0"
    "Restricted\0NotAvailable\0PesenIndReserved\0"
    "UserProv_NotScreened\0UserProv_Passed\0"
    "UserProv_Failed\0NetProv\0V110_X30\0"
    "G711_PCMU\0G711_PCMA\0G721_ADPCM\0H2xx\0"
    "Non_CCITT\0V120\0X31_HDLC\0UserInfoMax\0"
    "DialTone\0RingBackToneOn\0InterceptToneOn\0"
    "NetCongestionToneOn\0BusyToneOn\0"
    "ConfirmToneOn\0AnswToneOn\0CallWaitingToneOn\0"
    "OffHookWarning\0PreemptionToneOn\0"
    "TonesOff\0AllertingOn_p0\0AllertingOn_p1\0"
    "AllertingOn_p2\0AllertingOn_p3\0"
    "AllertingOn_p4\0AllertingOn_p5\0"
    "AllertingOn_p6\0AllertingOn_p7\0"
    "AllertingOff\0Channels\0SingleIntf\0"
    "AllIntf\0"
};

const QMetaObject ISDN::IeConstants::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ISDN__IeConstants,
      qt_meta_data_ISDN__IeConstants, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ISDN::IeConstants::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ISDN::IeConstants::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ISDN::IeConstants::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ISDN__IeConstants))
        return static_cast<void*>(const_cast< IeConstants*>(this));
    return QObject::qt_metacast(_clname);
}

int ISDN::IeConstants::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
	Utils::QObjFactoryRegister<ISDN::IeConstants> GRegQObj_0;
}; // end namespace 
QT_END_MOC_NAMESPACE
