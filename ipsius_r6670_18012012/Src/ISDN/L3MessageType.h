#ifndef _L3_MESSAGE_TYPE_H_
#define _L3_MESSAGE_TYPE_H_

namespace ISDN
{
	class L3MessgeType : public QObject
	{
		Q_OBJECT
		Q_ENUMS(Type)
	public:
		// перечисление стандартных типов пакетов и значения их стандартного идентификатора
		enum Type
		{                
			ALERTING =          0x01,
			CALL_PROCEEDING   = 0x02,
			CONNECT           = 0x07,
			CONNECT_ACKNOWLEDGE = 0x0f,
			PROGRESS           = 0x03,
			SETUP              = 0x05, 
			SETUP_ACKNOWLEDGE  = 0x0d,
			RESUME             = 0x26,
			RESUME_ACKNOWLEDGE = 0x2e,
			RESUME_REJECT     =  0x22,
			SUSPEND           =  0x25,
			SUSPEND_ACKNOWLEDGE = 0x2d,
			SUSPEND_REJECT    =  0x21,
			USER_INFORMATION  =  0x20,
			DISCONNECT        =  0x45,
			RELEASE           =  0x4d,
			RELEASE_COMPLETE  = 0x5a,
			RESTART           =  0x46,
			RESTART_ACKNOWLEDGE = 0x4e,
			SEGMENT            = 0x60,
			CONGESTION_CONTROL = 0x79,
			INFORMATION       =  0x7b,
			  FACILITY          =  0x62,
			NOTIFY            =  0x6e,
			STATUS            =  0x7d,
			STATUS_ENQUIRY    =  0x75,
			HOLD              =  0x24,
			HOLD_ACK          =  0x28,
			RETRIEVE          =  0x31,
			RETRIEVE_ACK      =  0x33,
			RETRIEVE_REJ      =  0x37,
			UNRNOWN			  =  0xFF 
		};

	};

} // ISDN

#endif

