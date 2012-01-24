#ifndef BChannelsDistributor_H
#define BChannelsDistributor_H

#include "stdafx.h"
#include "Utils/SafeRef.h"

namespace ISDN
{
    using boost::shared_ptr;

    class DssCallParams;
    class IeChannelIdentification;
    class L3Packet;
    class L3Call;
    class IBDistribStrategy;
    class IIsdnL3Internal;

    typedef shared_ptr<IeChannelIdentification> SharedIeChan;
    class IDssToGroup;


    // �������������� ��������(�������������� �������� / ���������� ������� � ���������� ���������� �������)
    class BChannelsDistributor : boost::noncopyable
    {            
        Utils::SafeRef<IDssToGroup> m_intfGroup;
        IIsdnL3Internal& m_IL3;

    public:
        // ������� Ie ��� ������� ������� ����� ������� � ����� ��� ���������,
        // ����� ��������������� ����������� ������. ������� ������� ���������� ������ shared_ptr.
        SharedIeChan CreateInquiry(shared_ptr<const DssCallParams> pInParams, L3Call* pCall);

        // ��������� Ie-������� �, ���� ��������, �������� ����������� ������, �������� Ie-������.
        // ������� ������� ���������� ������ shared_ptr.
        SharedIeChan CreateResponse(const L3Packet& incoming, L3Call* pCall);

        // ������������ Ie-������, �������� ����������� ������
        bool ProcessResponse(const L3Packet& incoming, L3Call* pCall);        

        BChannelsDistributor(Utils::SafeRef<IDssToGroup> intfGroupe, IIsdnL3Internal& IL3)
            : m_intfGroup(intfGroupe),
            m_IL3(IL3)
        {}
    };
} // namespace ISDN

#endif

