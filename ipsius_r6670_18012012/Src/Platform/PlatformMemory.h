#ifndef __PLATFORMMEMORY__
#define __PLATFORMMEMORY__

#include "Platform/PlatformTypes.h"

namespace Platform
{

    // Класс для преобразования указатель <-> целое число
    template<class T>
    class IntPtrT
    {
        T m_value;

    public:

        explicit IntPtrT(void *p)
        {
            Set(p);
        }

        explicit IntPtrT(T v)
        {
            Set(v);
        }

        void Set(void *p)
        {
            m_value = reinterpret_cast<T>(p);
        }

        void Set(T v)
        {
            m_value = v;
        }

        T Value() const
        {
            return m_value;
        }

        template<class TPtr>
        TPtr* ToPtr() const
        {
            return  reinterpret_cast<TPtr*>(m_value);
        }

        void* ToVoid() const
        {
            return ToPtr<void>();
        }

        bool ValueIsAligned() const
        {
            return ((m_value % CMemoryAlign) == 0);
        }

    };

    typedef IntPtrT<IntPtrValue> IntPtr;

    // ----------------------------------------------------------------

    #define ALIGN_CALC(_addr, _align) ( (_addr + _align - 1) - (_addr + _align - 1) % _align )

    // выравнивание адреса для платформы
    template<IntPtrValue addr, int align = CMemoryAlign>
    struct AlignAddr
    {
        enum
        {               
            Value = ALIGN_CALC(addr, align)
        };
    };

    // выравнивание размера для платформы (чаще всего бессмысленная операция)
    template<class T>
    struct AlignSizeOf
    {
        enum
        {               
            Value = AlignAddr<sizeof(T)>::Value
        };
    };

    struct AlignRuntime
    {
        static IntPtrValue Calc(IntPtrValue addr)
        {
            return ALIGN_CALC(addr, CMemoryAlign);
        }

        template<class TVal>
        static TVal Calc(TVal val)
        {
            return ALIGN_CALC(val, CMemoryAlign);
        }

    };

    #undef ALIGN_CALC

    template<class T>
    struct AlignAssertion
    {
        enum
        {
            Value = (sizeof(T) % CMemoryAlign == 0)
        };

        BOOST_STATIC_ASSERT(Value); 
    };


    // ----------------------------------------------------------------

    namespace Detail
    {
        // Compile time validation for AlignAddr and IntPtr
        struct Check
        {
            
            template<int align, IntPtrValue addr, IntPtrValue result>
            struct Verify
            {
                BOOST_STATIC_ASSERT( (AlignAddr<addr, align>::Value == result) ); 
            
                static void Foo() {}
            };

            void Align_1()
            {
                const int align = 1;

                Verify<align, 0, 0>::Foo();
                Verify<align, 1, 1>::Foo();
                Verify<align, 2, 2>::Foo();
                Verify<align, 3, 3>::Foo();
                Verify<align, 4, 4>::Foo();
                Verify<align, 5, 5>::Foo();
            }

            void Align_4()
            {
                const int align = 4;

                Verify<align, 0, 0>::Foo();
                Verify<align, 1, 4>::Foo();
                Verify<align, 2, 4>::Foo();
                Verify<align, 3, 4>::Foo();
                Verify<align, 4, 4>::Foo();
                Verify<align, 5, 8>::Foo();
            }

            // just compilation test, make runtime test - ?!
            void PtrTest()
            {
                IntPtr ptr(0x100);

                int* pInt = ptr.ToPtr<int>();
                void *p = ptr.ToVoid();

                ptr.Set(p);
                IntPtrValue val = ptr.Value();
            }

        };
    }


}  // namespace Platform

#endif

