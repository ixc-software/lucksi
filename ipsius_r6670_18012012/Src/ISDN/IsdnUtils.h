#ifndef DETAILS_H
#define DETAILS_H

#include "Utils/QtEnumResolver.h"
#include "iLog/LogWrapper.h"

namespace ISDN
{
    namespace IsdnUtils
    {
        using iLogCommon::LogString;

        // ��������������� ����� ��� ������ � ��������������
        class CorrectSet
        {
            std::set<int> m_setValue; // ����� ���������� ��������
        public:

            void Add(int val);

            void Reset();

            // return true if conversion possible
            template<class T>
            bool TryConvert(int value, T& out)
            {            
                std::set<int>::iterator i = m_setValue.find( value );
                bool rez = (i != m_setValue.end());
                if (rez) out = T(*i) ;
                return rez;
            }
        };

        // extract substring beginning after last inclusion substring "::" 
        QString ShortName(const QString& isxStr);

        std::string ShortName(std::string& isxStr);

        LogString ToLogString(const QString &s);
        LogString ToLogString(const std::string &s);

        template<class T>
        static QString TypeToShortName()
        {
            return ShortName(  Platform::FormatTypeidName( typeid(T).name() ).c_str()  );
        }

        template<class T>
        static QString TypeToShortName(const T&)
        {
            return TypeToShortName<T>();
        }       

        // ��� ������������� ���� ��������� � ���� ������.
        // ����� ������������ ������ ����� enum StateFields, � ���������� ��������� _maxField_ 
        // � ���������� friend class BitsetState< ... >;
        /*
        template <typename T>
        class BitsetState : public std::bitset<T::_maxField_>
        {

            typedef std::bitset<T::_maxField_> Base;
            typedef typename Base::reference Ref;
            
            bool operator[]( size_t _Pos ) const;
            // reference operator[]( size_t _Pos );
            Ref operator[]( size_t _Pos );

        public:

            // Base::reference operator[] (typename T::StateFields flagName)
            Ref operator[] (typename T::StateFields flagName)
            {
                ESS_ASSERT(position < T::_maxField_ - 1 );
                return Base::operator[](flagName);
            }

            void Set(typename T::StateFields flagName)
            {
                Base::operator[](flagName) = true;
            }

            void UnSet(typename T::StateFields flagName)
            {
                Base::operator[](flagName) = false;
            }

            //TODO StateToString
        };
        */

    } // IsdnUtils
} //ISDN

#endif

