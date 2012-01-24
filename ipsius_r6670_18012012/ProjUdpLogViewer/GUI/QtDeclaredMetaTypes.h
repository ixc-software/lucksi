#ifndef __QTDECLAREDMETATYPES__
#define __QTDECLAREDMETATYPES__

#include "Core/UdpLogDBStatus.h"
#include "Core/ModelToViewParams.h"

// Types used in signals and slots

Q_DECLARE_METATYPE(Ulv::UdpLogDBStatus);
Q_DECLARE_METATYPE(boost::shared_ptr<Ulv::ModelToViewParams>);
Q_DECLARE_METATYPE(Ulv::UdpLogRecordSrcInfoList);

// those also have to registered via call
//
// int qRegisterMetaType<TType>();
//
// before first using it in connect function

#endif
