#ifndef DSSTRACEOPTION_H
#define DSSTRACEOPTION_H

namespace ISDN
{
    struct DssTraceOption
    {
        DssTraceOption(bool onAll = false)
            // default option
            : m_traceOn(onAll),
            m_traceIeContent(onAll),
            m_traceIeList(onAll),
            m_traceDataExcangeL3(onAll),
            m_traceDataExcangeL3Call(onAll)            
        {}

        bool m_traceOn; // ��������� ��������� ������ ����������� IsdnL3 � ���� ��������� ��������. ���� false, ��������� �� ������������
        bool m_traceIeContent;
        bool m_traceIeList;
        bool m_traceDataExcangeL3;
        bool m_traceDataExcangeL3Call;              
    };
} // namespace ISDN

#endif
