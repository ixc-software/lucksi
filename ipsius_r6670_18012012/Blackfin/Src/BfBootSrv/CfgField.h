#ifndef CFGFIELD_H
#define CFGFIELD_H

#include "RangeValidator.h"
#include "BfBootCore/ConfigLine.h"
#include "BfBootCore/StringToTypeConverter.h"
#include "IRegField.h"
#include "Utils/ErrorsSubsystem.h"

namespace BfBootSrv
{
    // ключ дл€ фильтрации при доступе к полю через список полей
    struct CfgFieldProp
    {
        //enum Mask
        //{
        //    MaskA = 1 << 0,
        //    MaskB = 1 << 1,
        //    //...
        //};
        //
        //enum Key
        //{
        //    All = MaskA | MaskB,
        //    UserSettings = MaskA,
        //    InternalSettings = MaskB,
        //};       

        //static bool KeyMask(int key, int mask)
        //{
        //    return (key & mask) ? true : false;
        //}

        enum Key
        {            
            UserSettings,
            InternalSettings
        }; 


    };

    // обобщение CfgField скрывающее шаблон
    class ICfgField : public Utils::IBasicInterface
    {
    public:
        virtual const std::string& getName() const = 0;
        virtual CfgFieldProp::Key getFindMark() const = 0;
        //virtual bool Correspondence(const std::string& name, CfgFieldProp::Key key) const = 0;
        virtual bool setValue(const BfBootCore::ConfigLine& line) = 0; // false если не выполнено
        virtual BfBootCore::ConfigLine ToCfgLine() const = 0;
        virtual bool CompareVal(const ICfgField* other) const = 0; 
    };

    // ѕоле конфигурационной структуры Config
    template<class TField, template<class> class TRange = NoRangeValidation >
    class CfgField : public ICfgField  // copyable
    {   
        typedef CfgField<TField, TRange> TMy;

        bool m_inited;
        TField m_val;
        std::string m_name;
        CfgFieldProp::Key m_key;

        TRange<TField> m_range;                

        static const std::string& NotInited() // метка "значение не установленно"
        {
            static std::string notInited("NotInited");
            return notInited;
        }

    // ICfgField impl:
    private:
        const std::string& getName() const
        {
            return m_name;
        }

        CfgFieldProp::Key getFindMark() const
        {
            return m_key;
        }
        
        bool setValue(const BfBootCore::ConfigLine& line)
        {
            ESS_ASSERT(line.Name() == m_name);
            if (line.Value() == NotInited()) return true;

            BfBootCore::StringToTypeConverter::fromString(line.Value(), m_val);
            if ( !m_range.Validate(m_val) ) return false;
            m_inited = true;
            return true;
        }

        BfBootCore::ConfigLine ToCfgLine()const
        {
            //ESS_ASSERT(m_inited == true && "Need Init manuly. No default value."); // это штатна€ ситуаци€. »спользовать исключение.
            if (m_inited) return BfBootCore::ConfigLine(m_name, BfBootCore::StringToTypeConverter::toString(m_val));
            return BfBootCore::ConfigLine(m_name, NotInited());
        }    

        bool CompareVal(const ICfgField* other) const
        {
            const TMy* pOther = dynamic_cast<const TMy*>(other);
            ESS_ASSERT(pOther);
            return pOther->CompareVal(*this);
        }

    public:


        // конструктор задающий значение по умолчанию с указанием предела        
        CfgField(const std::string& name, CfgFieldProp::Key key, const TField& val, IRegField& list, const TRange<TField>& range)             
            : m_name(name),
            m_key(key),
            m_range(range)
        {
            Init(val);
            list.RegField(*this);
        }

        // конструктор неинициализированного элемента с указанием предела
        CfgField(const std::string& name, CfgFieldProp::Key key, IRegField& list, const TRange<TField>& range) 
            : m_inited(false),
            m_name(name),
            m_key(key),
            m_range(range)
        {
            list.RegField(*this);
        }

        // конструктор задающий значение по умолчанию        
        CfgField(const std::string& name, CfgFieldProp::Key key, const TField& val, IRegField& list)
            : m_name(name),
            m_key(key)
        {
            ESS_ASSERT( Init(val) ); // other exception?
            list.RegField(*this);
        }

        // конструктор неинициализированного элемента
        CfgField(const std::string& name, CfgFieldProp::Key key, IRegField& list)             
            : m_inited(false),
            m_name(name),
            m_key(key)
        {
            list.RegField(*this);
        }

        bool Init(const TField& val) 
        {   
            m_inited = false;               // -- или ASSERT или оставл€ть прежнеее значение ?
            if (m_range.Validate(val))
            {
                m_val = val;
                m_inited = true;                
            }            
            
            return m_inited;
        }   

        TMy& operator = (const TField& val)
        {
            ESS_ASSERT( Init(val) );
            return *this;
        }

        const TField& Value() const
        {
            ESS_ASSERT( IsInited() );
            return m_val;
        }

        TField& Value()
        {
            ESS_ASSERT( IsInited() );
            return m_val;
        }

        // for tests
        bool CompareVal(const TMy& other) const
        {
            if (m_inited == false && other.m_inited == false) return true;
            if (m_inited != other.m_inited) return false;
            return m_val == other.m_val;
        }

        bool IsInited() const
        {            
            return m_inited;
        }

        void Clear() // uninit
        {
            m_inited = false;
        }
        

        // возможно понадобитс€        
        /*const TRange<TField> getRange() const
        {
            return m_range;
        }*/


    };
} // namespace BfBootSrv

#endif
