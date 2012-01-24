#ifndef BROADCASTFILTER_H
#define BROADCASTFILTER_H

#include "stdafx.h"
#include "Utils/InitVar.h"

namespace BfBootCli
{
    struct BroadcastFilter
    {
        static const bool CCompareMacInLower = true;

        // При фильтрации сравниваются только инициализированные поля.
        Utils::InitProp<int> m_hwNum;
        Utils::InitProp<int> m_hwType;
        Utils::InitProp<int> m_cbpPort;
        Utils::InitProp<int> m_cmpPort;
        Utils::InitProp<QString> m_mac; // QString used for DRI wrappers
        Utils::InitProp<int> m_softNum;
        Utils::InitProp<QString> m_srcAddress; // QString used for DRI wrappers

        bool IsSatisfy(const BfBootCore::BroadcastMsg& msg) const
        {
            QString macInMsg(msg.Mac.c_str());        
            macInMsg = CCompareMacInLower ? macInMsg.toLower() : macInMsg.toUpper();

            bool satisfy = 
                m_hwNum.OptionalCompare(msg.HwNumber) &&
                m_hwType.OptionalCompare(msg.HwType) &&
                m_cbpPort.OptionalCompare(msg.BooterPort) &&
                m_cmpPort.OptionalCompare(msg.CmpPort) &&
                m_mac.OptionalCompare( macInMsg ) &&
                m_softNum.OptionalCompare(msg.SoftRelNumber);

            if (m_srcAddress.IsInited())
            {
                Utils::HostInf expectedHost;
                expectedHost.Address(m_srcAddress.get().toStdString());                

                satisfy = satisfy && expectedHost.Address() == msg.SrcAddress.Address();
            }

            return satisfy;
        }
    };
} // namespace BfBootCli

#endif
