#ifndef __DELETEIFPOINTER__
#define __DELETEIFPOINTER__

#include "stdafx.h"
#include "ErrorsSubsystem.h"
#include "IBasicInterface.h"

namespace Utils
{
    namespace mpl = boost::mpl; 

    // class, inherited from this type can't be deleted by DeleteIfPointer()
    class INonDestructable : public IBasicInterface {};


    // pointer of this class deleted by call to ProcessSpecialDelete()
    class ISpecialDestructable : public IBasicInterface
    {
    public:
        virtual void ProcessSpecialDelete() = 0;
    };

    // ------------------------------------------------------------

    namespace Detail
    {

        struct DeleteNop
        {

            template<class T>
            static void Do(T &p) 
            {
                // nothing
            }

        };

        struct TryDeletePointer
        {

            template<class T>
            struct DeleteByType
            {
                static void Delete(INonDestructable *p)
                {
                    // nothing
                }

                static void Delete(ISpecialDestructable *p)
                {
                    p->ProcessSpecialDelete();
                }

                static void Delete(const void *pIn)
                {
                    const T *p = static_cast<const T*>(pIn);
                    delete p;
                }

            };

            template<class T>
            static void Do(T *p) 
            {
                TUT_ASSERT(p != 0);
                DeleteByType<T>::Delete(p);
            }

        };


    }

	
    template<class T>
    void DeleteIfPointer(T &p)
    {
        using namespace Detail;

        const bool isPtr = boost::is_pointer<T>::value;
        typedef typename mpl::if_c<isPtr, TryDeletePointer, DeleteNop>::type Tx;
        Tx::Do(p);
    }

    void DeletePointerTest();

    // ------------------------------------------------------------

    template<typename T>
    void DeletePointer(T* &p)
    {    
        delete p;
        p = 0;
    }

}  // namespace



#endif

