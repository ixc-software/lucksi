#include "stdafx.h"
#include "ISDN/IeConstants.h"
#include "Dss1ToSipCause.h"

//http://www.voipforo.com/en/SIP/SIP_error_messages.php

namespace Dss1ToSip 
{
	int Dss1ToSipCauseConvertor::SipResponseToDss1Cause(int codeSip)
	{
		switch(codeSip)
		{
		case 400: 
			return ISDN::IeConstants::TemporaryFailure;
		case 401:
		case 402:
		case 403:
			return ISDN::IeConstants::CallRejected;
		case 404:
			return ISDN::IeConstants::UnallocatedNumber;
		case 405:
			return ISDN::IeConstants::ServiceUnavailable;
		case 406:
			return ISDN::IeConstants::ServiceNotImplemented;
		case 407:
			return ISDN::IeConstants::CallRejected;
		case 408:
			return ISDN::IeConstants::RecoveryTimerExpire;
		case 410:
			return ISDN::IeConstants::NumberChanged;
		case 413:
		case 414:
			return ISDN::IeConstants::Interworking;
		case 415:
			return ISDN::IeConstants::ServiceNotImplemented;
		case 416:
		case 420:
		case 421:
		case 423:
			return ISDN::IeConstants::Interworking;
		case 480:
			return ISDN::IeConstants::NoUserResponding;
		case 481:
			return ISDN::IeConstants::TemporaryFailure;
		case 482:
		case 483:
			return ISDN::IeConstants::ExchangeRoutingError;
		case 484:
			return ISDN::IeConstants::InvalidNumber;
		case 485:
			return ISDN::IeConstants::UnallocatedNumber;
		case 486:
			return ISDN::IeConstants::UserBusy;
		case 487:
			return ISDN::IeConstants::CallRejected;
		case 500:
			return ISDN::IeConstants::TemporaryFailure;
		case 501:
			return ISDN::IeConstants::ServiceNotImplemented;
		case 502:
			return ISDN::IeConstants::NetworkOutOfOrder;
		case 503:
			return ISDN::IeConstants::TemporaryFailure;
		case 513:
			return ISDN::IeConstants::Interworking;
		case 600:
			return ISDN::IeConstants::UserBusy;
		case 603:
			return ISDN::IeConstants::CallRejected;
		case 604:
			return ISDN::IeConstants::UnallocatedNumber;
		}
		return ISDN::IeConstants::NormCallClearing;
	}

	int Dss1ToSipCauseConvertor::Dss1CauseToSipResponse(int code)
	{
		switch(code)
		{
		case ISDN::IeConstants::TemporaryFailure: 
			return 400;
		case ISDN::IeConstants::CallRejected:
			return 401;
		case ISDN::IeConstants::UnallocatedNumber:
			return 404;
		case ISDN::IeConstants::ServiceUnavailable:
			return 405;
		case ISDN::IeConstants::ServiceNotImplemented:
			return 406;
		case ISDN::IeConstants::RecoveryTimerExpire:
			return 408;
		case ISDN::IeConstants::NumberChanged:
			return 410;
		case ISDN::IeConstants::Interworking:
			return 413;
		case ISDN::IeConstants::NoUserResponding:
			return 480;
		case ISDN::IeConstants::ExchangeRoutingError:
			return 482;
		case ISDN::IeConstants::InvalidNumber:
			return 484;
		case ISDN::IeConstants::UserBusy:
			return 486;
		case ISDN::IeConstants::NetworkOutOfOrder:
			return 502;
		}
		return 481;
	}
};
