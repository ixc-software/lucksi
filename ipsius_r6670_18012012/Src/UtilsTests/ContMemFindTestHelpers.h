
#ifndef __CONTMEMFINDTESTHELPERS__
#define __CONTMEMFINDTESTHELPERS__

// ContMemFindTestHelpers.h

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

namespace UtilsTests
{
    namespace mpl = boost::mpl; 
        
    // Asserts that T* == *(T**)
    class AssertIsEqualRemPtr
    {
    public:
        template<class TPtr, class TPtrPtr>
        static void Exec(TPtr ptr, TPtrPtr pptr)
        {
            /*
            // doesn't work due to comparison between T const and T (?)
            typedef typename boost::remove_pointer<TPtrPtr>::type TPtrSimple; 
            enum { Check = boost::is_same<TPtr, TPtrSimple>::value, };
            BOOST_STATIC_ASSERT(Check == true);
            */
            TUT_ASSERT(ptr == *pptr);
        }
        
    };

    // -------------------------------------

    // Asserts that T* == T*
    class AssertIsEqual
    {
    public:
        template<class TPtr, class TPtr2>
        static void Exec(TPtr ptr, TPtr2 ptr2)
        {
            TUT_ASSERT(ptr == ptr2);
        }
    };

    // -------------------------------------
    // Using for tets MemFind<>'s methods return value, when pointer's returned
    // Makes different assertion depends on class T.
    // TPtr - always pointer
    // T - pointer or pointer to pointer
    template<class TPtr, class T>
    void CheckFoundPtr(TPtr ptr, T checkPtr)
    {
        typedef typename boost::remove_pointer<T>::type TFirstPtrRemoved; 
        
        typedef typename mpl::if_<boost::is_pointer<TFirstPtrRemoved>, 
                                   AssertIsEqualRemPtr, 
                                   AssertIsEqual>::type Assertion;      

        Assertion::Exec(ptr, checkPtr);
    };

    // -------------------------------------

    class CheckFoundPtrTest
    {
        static void CheckAndDelete(int *ptr = 0)
        {
            int *p = ptr;
            int *p2 = ptr;
            int **pp = &p;
    
            CheckFoundPtr(p, pp);
            CheckFoundPtr(p, p2);
    
            delete ptr;
        }
        
    public:
        static void Run()
        {
            CheckAndDelete();
            CheckAndDelete(new int(8));
        }
    };
    
} // namespace UtilsTests


#endif
