#ifndef IUDPLOGRUNTIMERECORDING_H
#define IUDPLOGRUNTIMERECORDING_H

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "UlvTypes.h"

namespace Ulv
{
    class RecordViewSett;

    ESS_TYPEDEF(RecordingErr);

    class IUdpLogRuntimeRecording : public Utils::IBasicInterface
    {
    public:
        virtual void Add(const QString &formattedData) = 0;
        virtual const RecordViewSett& Format() const = 0;
        virtual QString DestDescription() const = 0;
        virtual ddword RecordedSize() const = 0;
    };

} // namespace Ulv

#endif // IUDPLOGRUNTIMERECORDING_H
