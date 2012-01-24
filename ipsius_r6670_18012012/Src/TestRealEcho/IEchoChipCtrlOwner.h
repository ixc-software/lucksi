#ifndef IECHOCHIPCTRLOWNER_H
#define IECHOCHIPCTRLOWNER_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "EchoApp/SfxProto.h"

namespace TestRealEcho
{
    class IEchoChipCtrlOwner : Utils::IBasicInterface
    {
    public:
        virtual void Activated() = 0;
        virtual void Error(const std::string& err) = 0;
        virtual EchoApp::IMainRecvSide& MainIntf() = 0;
    };
} // namespace TestRealEcho

#endif
