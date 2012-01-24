
#include "stdafx.h"

#include "VirtualInvokeTest.h"
#include "Utils/VirtualInvoke.h"
#include "Utils/ErrorsSubsystem.h"

namespace
{
	using namespace Utils;

    const int CDefaultVal = 5;

    int GValue;
	bool GParamIs0;

	// ---------------------------------

	class Param
	{
		int m_val;

	public:
		Param(int val = 0) : m_val(val)
		{
		}

		const int& GetVal() const { return m_val; }
		void SetVal(int val) {m_val = val; }
	};

    // ---------------------------------

    // simply smart pointer, without reference count
    template<class T>
    class SmartPtr : boost::noncopyable
    {
        T *m_p;

    public:

        SmartPtr() : m_p(0) {}
        SmartPtr(T *p) : m_p(p) {}

        ~SmartPtr()
        {
            delete m_p;
        }

        T* get() { return m_p; }

        T* operator->()
        {
            ESS_ASSERT(m_p != 0);
            return m_p;
        }

    };

	// ---------------------------------

	// func without param
    void SetGValueToDefault()
	{
		GValue = CDefaultVal;
	}

	// ---------------------------------

	// func with simple param
	void AddToGValue(int val)
	{
		GValue += val;
	}

	// ---------------------------------
	// func with ref param
	void FnFillParamWithCDefaultVal(Param &p)
	{
		p.SetVal(CDefaultVal);
	}

	// ---------------------------------
	// func with const ref param
	void FnCheckParam(const Param &p)
	{
        TUT_ASSERT(p.GetVal() == CDefaultVal);
	}

	// ---------------------------------
	// func with const ptr param
	void FnCheckParamIs0(const Param *p)
	{
		TUT_ASSERT(p != 0);
		TUT_ASSERT(p->GetVal() == 0);
        GParamIs0 = true;
	}

	// ---------------------------------
	// func with ptr param
	void FnSetParamTo0(Param* p)
	{
		TUT_ASSERT(p != 0);
		p->SetVal(0);
	}

	// ---------------------------------
	
	class VITest
	{
		typedef VITest T;
        
        int m_val;
		bool m_paramIs0;

		// method without param
		void SetValToDefault() 
		{
			m_val = CDefaultVal; 
		}

		// method with simple param
		void AddToVal(int val)
		{
			m_val += val;
		}

		// method with ref param
		void FillParamWithCDefaultVal(Param &p)
		{
			p.SetVal(CDefaultVal);
		}

		// method with const ref param
		void CheckParam(const Param &p)
		{
            TUT_ASSERT(p.GetVal() == CDefaultVal);
		}

		// method with ptr param
		void SetParamTo0(Param* p)
		{
			TUT_ASSERT(p != 0);
			p->SetVal(0);
		}

		// method with const ptr param
		void CheckParamIs0(const Param *p)
		{
			TUT_ASSERT(p != 0);
			TUT_ASSERT(p->GetVal() == 0);
			m_paramIs0 = true;
		}

        // method with 2 params
        void FillFirstWithSecond(Param &p, const int &val)
        {
            p.SetVal(val);
        }

        void FunctionsTest()
        {
            // check simple
            VIExecuter::Exec( VirtualInvoke::Create(&SetGValueToDefault) );
            TUT_ASSERT(GValue == CDefaultVal);

            VIExecuter::Exec( VirtualInvoke::Create(&AddToGValue, CDefaultVal) );
            TUT_ASSERT( GValue == (CDefaultVal * 2) );

            // check ref
            Param p;
            VIExecuter::Exec( VirtualInvoke::Create(&FnFillParamWithCDefaultVal, p) );
            VIExecuter::Exec( VirtualInvoke::Create(&FnCheckParam, p) );

            // check ptr
            VIExecuter::Exec( VirtualInvoke::Create(&FnSetParamTo0, &p) );

            SmartPtr<Param> pP( new Param(CDefaultVal) );
            VIExecuter::Exec( VirtualInvoke::Create(&FnSetParamTo0, pP.get()) );
            TUT_ASSERT(pP->GetVal() == 0);

            SmartPtr<const Param> pCP(new Param());
            VIExecuter::Exec( VirtualInvoke::Create(&FnCheckParamIs0, pCP.get()) );
            TUT_ASSERT(GParamIs0);            
        }

        void MethodsTest()
        {
            // check simple
            VIExecuter::Exec( VirtualInvoke::Create(&T::SetValToDefault, *this) );
            TUT_ASSERT(m_val == CDefaultVal);

            VIExecuter::Exec( VirtualInvoke::Create(&T::AddToVal, *this, CDefaultVal) );
            TUT_ASSERT( m_val == (CDefaultVal * 2) );

            // check ref
            Param p2;
            VIExecuter::Exec( VirtualInvoke::Create(&T::FillParamWithCDefaultVal, *this, p2) );
            VIExecuter::Exec( VirtualInvoke::Create(&T::CheckParam, *this, p2) );

            // check multi params
            int val = 8;
            VIExecuter::Exec( VirtualInvoke::Create(&T::FillFirstWithSecond, *this, p2, val) );
            TUT_ASSERT(p2.GetVal() == val);

            // check prt
            VIExecuter::Exec( VirtualInvoke::Create(&T::SetParamTo0, *this, &p2) );

            SmartPtr<Param> pP2(new Param(CDefaultVal));
            VIExecuter::Exec( VirtualInvoke::Create(&T::SetParamTo0, *this, pP2.get()) );
            TUT_ASSERT(pP2->GetVal() == 0);

            SmartPtr<const Param> pCP2(new Param());
            VIExecuter::Exec( VirtualInvoke::Create(&T::CheckParamIs0, *this, pCP2.get()) );
            TUT_ASSERT(m_paramIs0);
        }

	public:
		VITest() : m_val(0), m_paramIs0(false)
		{
			// test with functions
            FunctionsTest();

			// test with methods
            MethodsTest();
		}
	};

} // namespace

// -------------------------------------

namespace UtilsTests
{
	void VirtualInvokeTest()
	{
        GValue = 0;
        GParamIs0 = false;

		VITest test;
	}

} // namespace UtilsTests

