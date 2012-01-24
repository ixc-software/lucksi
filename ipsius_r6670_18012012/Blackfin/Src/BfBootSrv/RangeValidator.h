#ifndef RANGEVALIDATOR_H
#define RANGEVALIDATOR_H

namespace BfBootSrv
{
    template<class T>
    class Range
    {
        const T m_min;
        const T m_max;

    public:

        // ���� ���� ������� �������������� ���� ����������� - � ������� 
        // ���� �������� ������������� �������� �������, � � �������������� ������ �� ������
        Range()            
        {            
        }

        // [min, max]
        Range(const T& min, const T& max) : m_min(min), m_max(max)
        {}

        bool Validate(const T& val) const
        {
            return (val >= m_min && val <= m_max);
        }
    };

    template<class T>
    class NoRangeValidation
    {
    public:
        bool Validate(const T& val) const
        {
            return true;
        }
    };
} // namespace BfBootSrv

#endif
