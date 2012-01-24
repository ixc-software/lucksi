
#ifndef __UTILS_STATISTICBASE__
#define __UTILS_STATISTICBASE__

// StatisticBase.h

#include "Platform/Platform.h"
#include "Platform/PlatformMemory.h"
#include "Platform/PlatformTypes.h"

#include "IBasicInterface.h"
#include "ManagedList.h"

#include "StatisticElement.h"

namespace Utils
{
    // Base class for all fields, using for getting access to the class instance field
    // TField -- field type
    // TOwner -- field owner
    template<class TField, class TOwner> 
    class MetaField
    {
        Platform::IntPtrValue m_offset; 

        static TField& GetField(TOwner &instance, Platform::IntPtrValue offset)
        {
            Platform::IntPtrValue instanceVal = Platform::IntPtr(&instance).Value();
            TField *p = Platform::IntPtr(instanceVal + offset).ToPtr<TField>();

            return *p;
        }

    public:
        MetaField(TField &field) : m_offset(Platform::IntPtr(&field).Value())
        {}

        TField& Get(TOwner &instance) { return GetField(instance, m_offset); }
        const TField& Get(TOwner &instance) const { return GetField(instance, m_offset); }
    };

    // -----------------------------------------------------------

    // Base interface for all fields
    // TOwner -- field owner
    template<class TOwner>
    class IMetaField : 
        public Utils::IBasicInterface
    {
    public:
        virtual const std::string& Name() const = 0;
        virtual const std::string& Type() const = 0;

        virtual void Clear(TOwner &instance) = 0;
        virtual std::string ToString(TOwner &instance, bool showFieldType = false) const = 0;
        virtual void SummWith(TOwner &instance, TOwner &otherInstance) = 0;
    };

    // -----------------------------------------------------------

    namespace MetaFieldDetail
    {
        const std::string CMetaFieldAndValueSep = " = ";
        
    } // namespace MetaFieldDetail


    // -----------------------------------------------------------

    // Bf compile bug: this templates can't be inner for FieldBase
    template<bool isArithmetic>
    struct FieldBaseTypeClear
    {
        template<class T>
        static void Do(T &val) { val = 0; }
    };

    template<>
    struct FieldBaseTypeClear<false>
    {
        template<class T>
        static void Do(T &val) { val.Clear(); }
    };

    
    // -----------------------------------------------------------
    // Base class for all fields
    // TField -- field type
    // TOwner -- field owner type
    template<class TField, class TOwner>
    class FieldBase : 
        public IMetaField<TOwner>
    {
        enum
        {
            CIsArithmetic = boost::is_arithmetic<TField>::value,
        };

        MetaField<TField, TOwner> m_base;
         
        std::string m_name;
        std::string m_type;
        
    protected:

        virtual void FieldToStream(std::ostringstream &os, const TField &field) const = 0;


    // IMetaField impl
    public:
        const std::string& Name() const { return m_name; }
        const std::string& Type() const { return m_type; }

        void Clear(TOwner &instance)
        {
            FieldBaseTypeClear<CIsArithmetic>::Do( m_base.Get(instance) );
        }

        std::string ToString(TOwner &instance, bool showFieldType = false) const 
        {
            std::ostringstream os;
            os << Name();
            if (showFieldType) os << "(" << Type() << ")";
            os << MetaFieldDetail::CMetaFieldAndValueSep;
            FieldToStream(os, m_base.Get(instance));

            return os.str();
        }

        void SummWith(TOwner &instance, TOwner &otherInstance)
        {
            m_base.Get(instance) += m_base.Get(otherInstance);
        }

    public:
        FieldBase(TField &field, const std::string &fieldName) : 
            m_base(field), m_name(fieldName), m_type(typeid(TField).name())
        {
        }
    }; 

    // -----------------------------------------------------------
    // Class-field for simple types (int, float etc.)
    // TField -- class with operator+=, operator= and can be set to 0
    // TOwner -- field owner type    
    // TField must be build-in arithmetic type OR support .Clear(), << to stream, +=
    template<class TField, class TOwner>
    class NumField : public FieldBase<TField, TOwner>
    {
        typedef FieldBase<TField, TOwner> Base;
        
        void FieldToStream(std::ostringstream &os, const TField &field) const // override
        {
            os << field;
        }


    public:
        NumField(TField &field, const std::string &fieldName) : 
            Base(field, fieldName)
        {
        }
    }; 

    // -----------------------------------------------------------
    // TStatElement -- StatisticElement<>
    // TOwner -- field owner type
    // statLevel -- parameter for ToString() method
    template<class TStatElement, class TOwner>
    class StatElemField : 
        public FieldBase<TStatElement, TOwner>
    {
        typedef FieldBase<TStatElement, TOwner> Base;

        int m_statLvl;

        void FieldToStream(std::ostringstream &os, const TStatElement &field) const // override
        {
            os << "{ " << field.ToString(m_statLvl) << " }";
        }


    public: 
        StatElemField(TStatElement &field, const std::string &fieldName, int statLevel) : 
            Base(field, fieldName), m_statLvl(statLevel)
        {
        }
    };

    // -----------------------------------------------------------

    // List of the fields in class 
    // TFieldOwner -- fields owner, 
    //                should have static method InitMeta(StatMetaFieldList<TFieldOwner> &meta),
    //                where all his fields will be registred as meta information
    template<class TFieldOwner>
    class StatMetaFieldList
    {
        Utils::ManagedList<IMetaField<TFieldOwner> > m_list;

        IMetaField<TFieldOwner>* Find(const std::string &name)
        {
            for (size_t i = 0; i < Count(); ++i)
            {
                if (m_list[i]->Name() == name) return m_list[i];
            }

            return 0;
        }

        // pField delete in ~StatMetaFieldList()
        void AddField(IMetaField<TFieldOwner> *pField)
        {
            ESS_ASSERT(Find(pField->Name()) == 0);

            m_list.Add(pField);
        }
        
    public:
        StatMetaFieldList()
        {
            // init self from TFieldOwner::InitMeta()
            TFieldOwner::InitMeta(*this);
        }

        // Add for NumField<> fields
        template<class TField>
        void Add(TField &field, const std::string &fieldName)
        {
            IMetaField<TFieldOwner> *p = 
                new NumField<TField, TFieldOwner>(field, fieldName); // , TypeToName<TField>());
            
            AddField(p);
        }

        /*
        // Add for StatElemField<> fields
        template<class TStatElement, class TStatAccum, class TStatCounter, class TAdvancedParams>
        void Add(StatisticElement<TStatElement, TStatAccum, TStatCounter> &field, 
                 const std::string &fieldName, TAdvancedParams params)
        {
            IMetaField<TFieldOwner> *p = 
                new StatElemField<StatisticElement<TStatElement, TStatAccum, TStatCounter>, 
                                  TFieldOwner>(field, fieldName, params);
            
            AddField(p);
        }
        */
        
        // Add for StatElemField<> fields
        template<class TStatElement, class TAdvancedParams>
        void Add(TStatElement &field, const std::string &fieldName, TAdvancedParams params)
        {
            IMetaField<TFieldOwner> *p = 
                new StatElemField<TStatElement, TFieldOwner>(field, fieldName, params);
            
            AddField(p);
        }
        
        IMetaField<TFieldOwner>& operator[](size_t index)
        {
            ESS_ASSERT(index < Count());
            
            return *m_list[index];
        }

        size_t Count() const { return m_list.Size(); }
    };

    // -----------------------------------------------------------

    Platform::Mutex& StatisticBaseMetaMutex();

    // -----------------------------------------------------------
    
    // Base class for all statistic classes
    // TClass -- class-heir of StatisticBase<>, 
    //           should have static method StatMetaFieldList<TClass>& Meta() 
    //           to get access to meta data for this class
    template<class TClass>
    class StatisticBase
    {
        TClass& TClassInstance() const  // this method is formal "const"
        {
            const TClass *p = dynamic_cast<const TClass*>(this);
            ESS_ASSERT(p != 0);
            
            return const_cast<TClass&>(*p);
        }

        // static Utils::StatMetaFieldList<TClass> *PClassMeta;
        static bool MetaInitDone;

        static Utils::StatMetaFieldList<TClass>& GetMeta()
        {
            static Utils::StatMetaFieldList<TClass> meta;
            return meta;
        }
        
    public:
        StatisticBase() 
        {
            // Clear(); // <- base class not constructed
        }

        virtual ~StatisticBase() {}

        void Clear()
        {
            StatMetaFieldList<TClass> &meta = TClass::Meta();
            TClass &inst = TClassInstance();

            for (size_t i = 0; i < meta.Count(); ++i)
            {
                meta[i].Clear(inst);
            }
        }

        void SummWith(const StatisticBase<TClass> &other)
        {
            StatMetaFieldList<TClass> &meta = TClass::Meta();
            TClass &inst = TClassInstance();
            TClass &inst2 = other.TClassInstance();

            for (size_t i = 0; i < meta.Count(); ++i)
            {
                meta[i].SummWith(inst, inst2);
            }
        }

        virtual void ToString(std::string &res, const std::string &sep, bool showFieldType) const
        {
            StatMetaFieldList<TClass> &meta = TClass::Meta();
            TClass &inst = TClassInstance();

            res.clear();

            for (size_t i = 0; i < meta.Count(); ++i)
            {
                res += meta[i].ToString(inst, showFieldType);
                if (i < (meta.Count() - 1)) res += sep;
            }
        }

        std::string ToString(const std::string &sep = "; ", bool showFieldType = false) const
        {
            std::string res;
            ToString(res, sep, showFieldType);
            
            return res;
        }

        static Utils::StatMetaFieldList<TClass>& Meta()                  
        {
            if (!MetaInitDone)
            {
                Platform::MutexLocker lock( StatisticBaseMetaMutex() );

                Utils::StatMetaFieldList<TClass> &res = GetMeta();
                MetaInitDone = true;

                return res;
            }

            return GetMeta();                                                
        }   

        static void InitMeta(Utils::StatMetaFieldList<TClass> &meta)     
        {                                                               
            TClass *p = 0;                                               
            TClass::RegisterFields(*p , meta);                                  
        }   
        
        typedef Utils::StatisticBase<TClass> TBase;                      
        
    };

    template<class TClass>
    bool StatisticBase<TClass>::MetaInitDone = false;

} // namespace Utils

// -----------------------------------------------------------

// Macros using for fields registration
#define STATISTIC_META_INFO(_type)                                  \
    static void RegisterFields(_type &inst,                         \
                    Utils::StatMetaFieldList<_type> &meta)          \
    

// -----------------------------------------------------------

/*
How to use StatisticBase<>:

    1) Inherit your statistic class from StatisticBase<>: 

       class YourClass : public StatisticBase<YourClass> { ... };
        
    2) Register class fields using macros STATISTIC_META_INFO
  
Example:

namespace
{
    using namespace Utils;
    
    struct SomeStat : public StatisticBase<SomeStat>
    {
        int Field;
        StatElementForInt Stat;
        
        SomeStat() : StatisticBase<SomeStat>()
        {
            TBase::Clear();
        }

        STATISTIC_META_INFO(SomeStat)
        {
            meta.Add(inst.Field, "Field");
            meta.Add(inst.Stat, "Stat", StatElementForInt::LevAverage);
        }
    };

} // namespace
*/

#endif
