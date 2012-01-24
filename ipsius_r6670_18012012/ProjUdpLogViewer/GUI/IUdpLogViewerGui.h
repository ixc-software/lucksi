#ifndef IUDPLOGVIEWERGUI_H
#define IUDPLOGVIEWERGUI_H

#include "Utils/IBasicInterface.h"

namespace Ulv
{
    class UdpLogDBStatus;
    class ModelToViewParams;

} // namespace Ulv

// ------------------------------------------------------------------------

namespace UlvGui
{
    class GuiCtrlToModelParams;

    // Interface for UdpLogViewer owner
    class IUdpLogViewerGui : public Utils::IBasicInterface
    {
    public:
        virtual bool IsVisible() const = 0;
        virtual bool IsMinimized() const = 0;
        virtual bool IsNotInFocus() const = 0;

        virtual void ShowError(const QString &err, bool isCritical) = 0;
        virtual void ShowInfo(const QString &text) = 0;

        // virtual void Close() = 0;

        virtual void Update(boost::shared_ptr<Ulv::ModelToViewParams> params) = 0;
        virtual void UpdateStatus(Ulv::UdpLogDBStatus status) = 0;
        virtual void UpdateDebug() = 0;

        virtual boost::shared_ptr<UlvGui::GuiCtrlToModelParams> CurrentParams() const = 0;
    };

} // namespace UlvGui


#endif // IUDPLOGVIEWERGUI_H
