#ifndef RANGELIST_H
#define RANGELIST_H

#include "stdafx.h"
#include "ErrorsSubsystem.h"
#include "Platform/Platform.h"
 
namespace Utils
{
    ESS_TYPEDEF(RangeParserError);    
            
    class ValueRange
    {
        Platform::ddword m_fromVal;
        Platform::ddword m_toVal;   

        bool RangeOk() const { return (m_toVal >= m_fromVal); }
    public:
        
        ValueRange( const std::string& s, const std::string& sep = "-");
        ValueRange( Platform::ddword from, Platform::ddword to);

        Platform::ddword From() const {return m_fromVal;}
        Platform::ddword To() const {return m_toVal;}
        Platform::ddword Width() const {return m_toVal - m_fromVal + 1;}

        void ExpandLow(int val); // расширение по нижнему пределу
        void ExpandHi(int val); // расширение по верхнему пределу;

        bool ExistValue(const Platform::ddword& val) const
        {
            return m_fromVal <= val && val <= m_toVal;                
        }
        std::string ToString(const std::string& sep = "-") const;

        template<class T>
        void AddToContainer(T &cont) const 
        {
            ESS_ASSERT( RangeOk() );

            for(Platform::ddword i = m_fromVal; i <= m_toVal; ++i)
            {
                cont.push_back(i);
            }
        }


        static void Test();
    };

    
    // Список диапазонов значений типа Platform::ddword.
    // Поддерживает преобразование и из строки и обратно
    // имеющей формат: val-val, val-val, .... val-
    // Символы разделители "-", "," могут быть заменены другими.
    // Элемент val может иметь десятичны и hex формат.
    // НЕ КОНТРОЛИРУЕТ пересечение диапазонов.  ???
    class RangeList
    {
        std::vector<ValueRange> m_rangeList; // vector?        
        
    public:
        
        // can throw
        RangeList(const std::string& inStr, const std::string& sep = ",", const std::string& prolong = "-");                                                      

        // return 0 if can`t
        static RangeList* Create(const std::string& inStr, std::string& err, const std::string& sep = ",", const std::string& prolong = "-");
        
        void AddRange(const ValueRange& range) {m_rangeList.push_back(range);}
        bool ExistValue(const Platform::ddword& val) const;
        const ValueRange& operator[](int i) const;              
        const ValueRange& At(int i) const {return operator[](i);}
        ValueRange& operator[](int i);              
        int Size() const;
        void Clear();
        std::string ToString(const std::string& sep = ",", const std::string& prolong = "-") const; 

        static void Test();
    };        


    //void TestRangeList();
    
   
} // namespace Utils

#endif
