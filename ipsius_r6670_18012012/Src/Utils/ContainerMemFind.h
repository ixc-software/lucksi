#ifndef FINDERBYMEMBER_H
#define FINDERBYMEMBER_H

#include "stdafx.h"
#include "IterTypeToValueType.h"
#include "ErrorsSubsystem.h"

/*
       MemFind<T> -- ����� �������� � �����������, ��������� �������� � ����� ����� �����

*/

namespace Utils
{

    namespace mpl = boost::mpl;    

    namespace Detail
    {
        // ������� ���������� assertions
        struct NoThrow
        {
            static void Throw(){/*do nothing*/}
        };

        struct EssAssert
        {
            static void Throw() { ESS_ASSERT(0 && "MemFind<> -- element not found"); }
        };

        struct TutAssert
        {
            static void Throw() { TUT_ASSERT(0 && "MemFind<> -- element not found"); }
        };


        // �������� ������� MemFind �� ���������. 
        // �������� ������������� �� ��������� ������ ��� ������������ ����������.
        template <class THowThrow>
        struct ThrowWithoutMsg
        {
            static void Throw()
            {
                THowThrow::Throw();
            }
        };
    } //namespace Detail
    
    typedef Detail::ThrowWithoutMsg<Detail::NoThrow> NoThrow;
    typedef Detail::ThrowWithoutMsg<Detail::EssAssert> EssAssertion;
    typedef Detail::ThrowWithoutMsg<Detail::TutAssert> TutAssertion;

    // ������� �������������� ����� ������������� MemFind
    class FindBase
    {        

    protected:        
        
        // ����������� �������������� ���� � ��������� �� ���, ���� �� ��� ������� �� ��������
        template<class TValue>
        struct ValToPtrVal
        {
            typedef typename mpl::if_
                <
                typename boost::is_pointer<TValue>::type,
                TValue,
                TValue*
                >::type type;
        };

        // ����������� �������������� ���� ��������� � ��������� �� �������� ���
        template<class TIter>
        class IterToPtrValue 
        {
            typedef typename IterTypeToValueType<TIter>::TValue TValue;

        public:

            typedef typename ValToPtrVal<TValue>::type type;

        };        

        // ����������� ���������� ����� ���������� � ��������� �� ���� ����������
        template<class TCont>
        class ContainerTraits
        {
            typedef typename TCont::value_type TValue;

            struct NonConstCont
            {
                typedef TValue value_type;
                typedef typename TCont::iterator iter_type;
            };

            struct ConstCont
            {
                typedef const TValue value_type;
                typedef typename TCont::const_iterator iter_type;
            };

            typedef typename mpl::if_
                <
                typename boost::is_const<TCont>::type,
                ConstCont,
                NonConstCont
                >::type TSelected;


        public:

            typedef typename ValToPtrVal<typename TSelected::value_type>::type ptr_value_type; // ptrValue???
            //typedef typename ValToPtrVal<typename TSelected::iter_type>::type iter_type;
            typedef typename TSelected::iter_type iter_type;
            //typedef typename TSelected::value_type value_type; // ptrValue???

        };


        //======================================================================

        // ������� ��� ��������� � ����������
        // TArg  ������ ���� ������ �� �� ������ �������������� ����� �������� �������� ������!
        template<class TObj, class TPred, class TArg>
        class PredicateFunctor
        {
            TPred m_pred;
            const TArg& m_arg;

        public:

            PredicateFunctor(const TArg& arg, TPred pred) 
                : m_pred(pred),
                m_arg(arg)
            {}

            bool operator()(const TObj& obj)
            {
                return (obj.*m_pred)(m_arg);
            }

            bool operator()(const TObj* const pObj)
            {
                // todo if 0 ?
                return (pObj->*m_pred)(m_arg);
                // ��� return operator()(*pObj);
            }

            bool operator()(boost::shared_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return (p->*m_pred)(m_arg);
            }

            bool operator()(boost::scoped_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return (p->*m_pred)(m_arg);
            }
        };

        // ������������� �������� ��������� ��� ���������
        template<class TObj, class TPred>
        class PredicateFunctor<TObj, TPred, void>
        {
            TPred m_pred;

        public:

            PredicateFunctor(TPred pred) 
                : m_pred(pred)
            {}

            bool operator()(const TObj& obj)
            {
                return (obj.*m_pred)();
            }

            bool operator()(const TObj* const pObj)
            {
                // todo if 0 ?
                return (pObj->*m_pred)();
                // ��� return operator()(*pObj);
            }

            bool operator()(boost::shared_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return (p->*m_pred)();
            }

            bool operator()(boost::scoped_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return (p->*m_pred)();
            }

        };       

        //======================================================================

        // ������� ��� ������ ������� ���� .getXXX()
        template<class TObj, class TGetter, class TArg>
        class GetterFunctor
        {
            TGetter m_getter;
            const TArg& m_arg;

        public:

            GetterFunctor(const TArg& arg, TGetter getter)
                : m_getter(getter),
                m_arg(arg)
            {}

            bool operator()(const TObj& obj)
            {
                return (obj.*m_getter)() == m_arg;
            }                

            bool operator()(const TObj* const pObj)
            {
                // todo if 0?
                return operator()(*pObj);
            }
             
            bool operator()(boost::shared_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return (p->*m_getter)() == m_arg;
            }

            bool operator()(boost::scoped_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return (p->*m_getter)() == m_arg;
            }

        };

        //--------------------------------------------------------

        // added (todo ���������� � GetterFunctor)
        // ������� ��� ������ ������� ���� TArg* .getXXX() 
        template<class TObj, class TGetter, class TArg>
        class PtrGetterFunctor
        {
            TGetter m_getter;
            const TArg& m_arg;

        public:

            PtrGetterFunctor(const TArg& arg, TGetter getter)
                : m_getter(getter),
                m_arg(arg)
            {}

            bool operator()(TObj& obj)
            {
                return *( (obj.*m_getter)() ) == m_arg;
            }

            bool operator()(TObj* pObj)
            {
                // todo if 0?
                return operator()(*pObj);
            }

            bool operator()(boost::shared_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return *( (p->*m_getter)() ) == m_arg;
            }

            bool operator()(boost::scoped_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return *( (p->*m_getter)() ) == m_arg;
            }

        };

        //--------------------------------------------------------

        // added (todo ���������� � GetterFunctor)
        // ������� ��� ������ ������� ���� .getXXX()
        template<class TObj, class TGetter, class TArg>
        class NoConstGetterFunctor
        {
            TGetter m_getter;
            const TArg& m_arg;

        public:

            NoConstGetterFunctor(const TArg& arg, TGetter getter)
                : m_getter(getter),
                m_arg(arg)
            {}

            bool operator()(TObj& obj)
            {
                return (obj.*m_getter)() == m_arg;
            }                

            bool operator()(TObj* const pObj)
            {
                // todo if 0?
                return operator()(*pObj);
            }

            bool operator()(boost::shared_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return (p->*m_getter)() == m_arg;
            }

            bool operator()(boost::scoped_ptr<TObj> pObj)
            {
                TObj* p = pObj.get();
                ESS_ASSERT(p != 0);
                return (p->*m_getter)() == m_arg;
            }

        };

        





        //======================================================================

        // �������� ��������������� ���� � ��������� �� ���        
        template<class T>
        static
        T* IterContentToPtr(T& obj)
        {
            return &obj;
        }
        
        template<class T>
        static
        T* IterContentToPtr(T* p)
        {
            return p;
        }

        //======================================================================

        // ���������������� ������� ������ � ���������
        template<class TContainer>
        class ContWriter        
        {
            TContainer& m_cont;
            bool m_wasWrited; // ������ ���� ��� ����������� ������

        public:

            ContWriter(TContainer& cont) 
                : m_cont(cont), 
                m_wasWrited(false)
            {
                //TODO Is Empty ???
            }

            template<class TData>
            void Write(const TData& data)
            {
                m_wasWrited = true;
                m_cont.push_back(data);
            }

            // ������ ���� ��� ����������� ������
            bool IsWrited() const
            {
                return m_wasWrited;
            }
        };

        //======================================================================

        // TODO Finder ������ ��� �������� ��� ��������?
        template< template<class,class,class> class Functor>
        class Finder
        {
            template<class TIter, class TFunctor>
            static TIter FindByFunctor(TIter begin, TIter end, TFunctor f)
            {
                TIter i;
                for (i = begin; i != end; ++i) // ��� �������� ���������� �������!
                    if ( f(*i) ) break;
                return i;
            }

            template<class TIter, class TFunctor, class TOut>
            static TIter FindAllByFunctor(TIter begin, TIter end, TFunctor f, ContWriter<TOut>& out)
            {
                TIter i;
                for (i = begin; i != end; ++i) // ��� �������� ���������� �������!
                    if ( f(*i) ) out.Write(i);
                return i;
            }

        public: 

            template<class TObj, class TArg, class TIter, class MemFn>
            static TIter Find(TIter begin, TIter end, MemFn fn, const TArg& arg)
            {
                Functor<TObj, MemFn, TArg> f(arg, fn);
                return FindByFunctor(begin, end, f);
            }            

            template<class TObj, class TIter, class MemFn>
            static TIter Find(TIter begin, TIter end, MemFn fn)
            {
                Functor<TObj, MemFn, void> f(fn);
                return FindByFunctor(begin, end, f);
            }    

            //-------------

            template<class TObj, class TArg, class TIter, class MemFn, class TOut>
            static TIter FindAll(TIter begin, TIter end, MemFn fn, const TArg& arg, ContWriter<TOut>& out)
            {
                Functor<TObj, MemFn, TArg> f(arg, fn);
                return FindAllByFunctor(begin, end, f, out);
            }            

            template<class TObj, class TIter, class MemFn, class TOut>
            static TIter FindAll(TIter begin, TIter end, MemFn fn, ContWriter<TOut>& out)
            {
                Functor<TObj, MemFn, void> f(fn);
                return FindAllByFunctor(begin, end, f, out);
            }  
        };

    };


    /*
        MemFind<
        ��� NoThrow - �� ������������ ����������, ���� �� �������� ���������� �������� ����� ��� ������� ���������
        ��� EssAssertion - ���� �� �������� ESS_ASSERT (��� ������),
        ��� TutAsertion - ���� �� �������� TUT_ASSERT (��� ������), 
        ��� ��� ���� ���������� �������������� �� ESS::BaseException |*Exception*|, ������������� � ������� ESS_THROW_MSG>
    */

    // �� ������������������ �� ���� ���������� ������.
    // ��� ������� ��� ���� ���� �������� ���������� ��� ���������� NoThrow ������ ������� � trow ��������,
    // ������������� �������������� ���������� ��������� �� ������
    template<class TException = NoThrow> 
    class MemFind : public FindBase
    {
        enum { CInherCheck = boost::is_base_of<ESS::BaseException/* Exception*/, TException>::value };
        BOOST_STATIC_ASSERT( CInherCheck );

        
        static void Assertion(bool condition, const std::string& errMsg)
        {
            if (!condition)
                ESS::DoException<TException>(errMsg, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__);
        };

    // User functions
    public:

        //=======================================================================================
        //FindRangeRetIter - ��������� �������, ������������ ��������, ������������ ����� ��
        // ���������, ������������� �� ��������� 4-� �������.


        // �������� c ����������            
        template<class TIter, class TArg, class TObj> 
        static
            TIter FindRangeRetIter(TIter begin, TIter end, 
            bool (TObj::*memPredicate)(const TArg&) const,  // method signature
            const TArg& arg,
            const std::string& errMsg)
        {        
            TIter i = Finder<PredicateFunctor>::Find<TObj>(begin, end, memPredicate, arg);
            Assertion(i != end, errMsg); 
            return i;
        }

        // �������� ��� ���������            
        template<class TIter, class TObj>    
        static
            TIter FindRangeRetIter(TIter begin, TIter end, 
            bool (TObj::*memPredicate)(void) const,
            const std::string& errMsg)  // method signature
        {        
            TIter i = Finder<PredicateFunctor>::Find<TObj>(begin, end, memPredicate);
            Assertion(i != end, errMsg);          
            return i;
        }

        template<class TIter, class TArg, class TObj>    
        static
            TIter FindRangeRetIter(TIter begin, TIter end, 
            TArg (TObj::*memGet)(void) const, // method signature
            const TArg& arg,
            const std::string& errMsg)
        {        
            TIter i = Finder<GetterFunctor>::Find<TObj>(begin, end, memGet, arg);     
            Assertion(i != end, errMsg);
            return i;
        }    

        template<class TIter, class TArg, class TObj>                
        static
            TIter FindRangeRetIter(TIter begin, TIter end,
            const TArg&(TObj::*memGet)(void)const,
            const TArg& arg,
            const std::string& errMsg )
        {        
            TIter i = Finder<GetterFunctor>::Find<TObj>(begin, end, memGet, arg);
            Assertion(i != end, errMsg);
            return i;
        }    

        // added
        template<class TIter, class TArg, class TObj>                
        static
            TIter FindRangeRetIter(TIter begin, TIter end,
            TArg&(TObj::*memGet)(void),
            const TArg& arg,
            const std::string& errMsg )
        {        
            TIter i = Finder<NoConstGetterFunctor>::Find<TObj>(begin, end, memGet, arg);
            Assertion(i != end, errMsg);
            return i;
        }  

        // added
        template<class TIter, class TArg, class TObj>                
        static
            TIter FindRangeRetIter(TIter begin, TIter end,
            TArg*(TObj::*memGet)(void),
            const TArg& arg,
            const std::string& errMsg )
        {        
            TIter i = Finder<PtrGetterFunctor>::Find<TObj>(begin, end, memGet, arg);
            Assertion(i != end, errMsg);
            return i;
        }  

        //==============================================================================================
        // FindRangeRetPtr - ��������� �������, ������������ ���������. ���������� ��� ������� ���� ���� �������
        // ��������� ��� ���. ����� � ��������� ����������.

        // Overload: without arg            
        template<class TIter, class TArg, class TFn> 
        static
            typename IterToPtrValue<TIter>::type    
            FindRangeRetPtr(TIter begin, TIter end,
            TFn mem,
            const TArg& arg,
            const std::string& errMsg)
        {        
            TIter i = FindRangeRetIter(begin, end, mem, arg, errMsg);            
            return IterContentToPtr(*i);            
        }

        // Overload: arg exist          
        template<class TIter, class TFn>
        static
            typename IterToPtrValue<TIter>::type    
            FindRangeRetPtr(TIter begin, TIter end,
            TFn mem,
            const std::string& errMsg)
        {        
            TIter i = FindRangeRetIter(begin, end, mem, errMsg);            
            return IterContentToPtr(*i);
        }

        //=======================================================================================    
        //FindContRetPtr - ��������� �������, ������������ ���������. ���������� ��� ������� ���� ���� �������
        // ��������� ��� ���. ����� �� ����������.

        // Overload: without arg 
        template< class TContainer, class TFn>     
        typename ContainerTraits <TContainer>::ptr_value_type
            FindContRetPtr(TContainer& cont, TFn memFn, const std::string& errMsg)
        {        
            return FindRangeRetPtr(cont.begin(), cont.end(), memFn, errMsg);
        }

        // Overload: arg exist
        template< class TContainer, class TFn, class TArg> 
        typename ContainerTraits <TContainer>::ptr_value_type    
            FindContRetPtr(TContainer& cont, TFn memFn, const TArg& arg, const std::string& errMsg)
        {        
            return FindRangeRetPtr(cont.begin(), cont.end(), memFn, arg, errMsg);
        }

        //=======================================================================================    
        //FindContRetIter - ��������� �������, ������������ ��������. ���������� ��� ������� ���� ���� �������
        // ��������� ��� ���. ����� �� ����������.


        // Overload: without arg 
        template< class TContainer, class TFn>     
        static
        typename ContainerTraits <TContainer>::iter_type
            FindContRetIter(TContainer& cont, TFn memFn, const std::string& errMsg)
        {        
            return FindRangeRetIter(cont.begin(), cont.end(), memFn, errMsg);
        }

        //// Overload: arg exist
        //template< class TContainer, class TFn, class TArg> 
        //static
        //typename ContainerTraits <TContainer>::iter_type
        //    FindContRetIter(TContainer& cont, TFn memFn, /*const */TArg& arg, const std::string& errMsg)
        //{        
        //    return FindRangeRetIter(cont.begin(), cont.end(), memFn, arg, errMsg);
        //}

        // Overload: arg exist
            template< class TContainer, class TFn, class TArg> 
        static
            typename ContainerTraits <TContainer>::iter_type
            FindContRetIter(TContainer& cont, TFn memFn, const TArg& arg, const std::string& errMsg)
        {        
            return FindRangeRetIter(cont.begin(), cont.end(), memFn, arg, errMsg);
        }

        //=======================================================================================
        // FindAllRange - ��������� �������, ������������ � ���������� �� ������ ��������
        // ��������� ��������� ���� ��������� �������� � ���������� ��������� ����������.


        // �������� c ����������            
        template<class TIter, class TArg, class TObj, class TOutCont> 
        static
            void FindAllRange(TIter begin, TIter end, 
            bool (TObj::*memPredicate)(const TArg&) const,  // method signature
            const TArg& arg,
            TOutCont& out,
            const std::string& errMsg)
        {        
            ContWriter<TOutCont> writer(out);
            Finder<PredicateFunctor>::FindAll<TObj>(begin, end, memPredicate, arg, writer);
            Assertion( writer.IsWrited(), errMsg );
        }

        // �������� ��� ���������            
        template<class TIter, class TObj, class TOutCont>    
        static
            void FindAllRange(TIter begin, TIter end, 
            bool (TObj::*memPredicate)(void) const, // method signature
            TOutCont& out,
            const std::string& errMsg)  
        {        
            ContWriter<TOutCont> writer(out);
            Finder<PredicateFunctor>::FindAll<TObj>(begin, end, memPredicate, writer);
            Assertion( writer.IsWrited(), errMsg );
        }

        template<class TIter, class TArg, class TObj, class TOutCont>    
        static
            void FindAllRange(TIter begin, TIter end, 
            TArg (TObj::*memGet)(void) const, // method signature
            const TArg& arg,
            TOutCont& out,
            const std::string& errMsg )
        {        
            ContWriter<TOutCont> writer(out);
            Finder<GetterFunctor>::FindAll<TObj>(begin, end, memGet, arg, writer);        
            Assertion( writer.IsWrited(), errMsg );
        }    

        template<class TIter, class TArg, class TObj, class TOutCont>                
        static
            void FindAllRange(TIter begin, TIter end, 
            const TArg&(TObj::*memGet)(void)const, // method signature
            const TArg& arg,
            TOutCont& out,
            const std::string& errMsg )
        {        
            ContWriter<TOutCont> writer(out);
            TIter i = Finder<GetterFunctor>::FindAll<TObj>(begin, end, memGet, arg, writer);
            Assertion( writer.IsWrited(), errMsg );
        }  

        // ������������� �������� � ����������
        template<class TIter, class TArg, class TObj, class TOutCont>    
        static
            void FindAllRange(TIter begin, TIter end, 
            TArg& (TObj::*memGet)(void), // method signature
            const TArg& arg,
            TOutCont& out,
            const std::string& errMsg )
        {        
            ContWriter<TOutCont> writer(out);
            Finder<NoConstGetterFunctor>::FindAll<TObj>(begin, end, memGet, arg, writer);        
            Assertion( writer.IsWrited(), errMsg );
        }    

        // ������������� �������� � ����������
        template<class TIter, class TArg, class TObj, class TOutCont>                
        static
            void FindAllRange(TIter begin, TIter end, 
            TArg* (TObj::*memGet)(void), // method signature
            const TArg& arg,
            TOutCont& out,
            const std::string& errMsg )
        {        
            ContWriter<TOutCont> writer(out);
            TIter i = Finder<PtrGetterFunctor>::FindAll<TObj>(begin, end, memGet, arg, writer);
            Assertion( writer.IsWrited(), errMsg );
        }  

        //=======================================================================================
        // FindAllCont - ��������� ���� ������������ ����� ���� ��������� �� ������� ���������� �
        // ������������ ��������� �� ��������� ������� � ��������

        // �������� c ����������                        
        template<class TCont, class TArg, class Fn, class TOutCont>    
        static
            void FindAllCont(TCont& inCont, 
            Fn f, 
            const TArg& arg,
            TOutCont& out,
            const std::string& errMsg)  
        {        
            FindAllRange(inCont.begin(), inCont.end(), f, arg, out, errMsg);
        }

        // �������� ��� ���������            
        template<class TCont, class Fn, class TOutCont>    
        static
            void FindAllCont(TCont& inCont, 
            Fn f, 
            TOutCont& out,
            const std::string& errMsg)  
        {        
            FindAllRange(inCont.begin(), inCont.end(), f, out, errMsg);
        }

   
    };

    //==============================================================================================

    // ������������� MemFind �� ������������� ���������� ��� ������������ ����������� ��� ��� ������� � THowDoThrow.
    template< class THowDoThrow >
    class MemFind< Detail::ThrowWithoutMsg<THowDoThrow> > : public FindBase
    {        
        static void Assertion(bool condition)
        {
            if (!condition)
                Detail::ThrowWithoutMsg<THowDoThrow>::Throw();
        }

    // User functions
    public:
        
        //=======================================================================================
        //FindRangeRetIter - ��������� �������, ������������ ��������, ������������ ����� ��
        // ���������, ������������� �� ��������� 4-� �������.


        // �������� c ����������            
        template<class TIter, class TArg, class TObj> 
        static
            TIter FindRangeRetIter(TIter begin, TIter end, 
            bool (TObj::*memPredicate)(const TArg&) const,  // method signature
            const TArg& arg )
        {        
            TIter i = Finder<PredicateFunctor>::Find<TObj>(begin, end, memPredicate, arg);
            Assertion( i != end );
            return i;
        }

        // �������� ��� ���������            
        template<class TIter, class TObj>    
        static
            TIter FindRangeRetIter(TIter begin, TIter end, 
            bool (TObj::*memPredicate)(void) const)  // method signature
        {        
            TIter i = Finder<PredicateFunctor>::Find<TObj>(begin, end, memPredicate);
            Assertion( i != end );
            return i;
        }

        template<class TIter, class TArg, class TObj>    
        static
            TIter FindRangeRetIter(TIter begin, TIter end, 
            TArg (TObj::*memGet)(void) const, // method signature
            const TArg& arg )
        {        
            TIter i = Finder<GetterFunctor>::Find<TObj>(begin, end, memGet, arg);        
            Assertion( i != end );
            return i;
        }    

        template<class TIter, class TArg, class TObj>                
        static
            TIter FindRangeRetIter(TIter begin, TIter end, const TArg&(TObj::*memGet)(void)const, const TArg& arg )
        {        
            TIter i = Finder<GetterFunctor>::Find<TObj>(begin, end, memGet, arg);
            Assertion( i != end );
            return i;
        }    

        // added
        template<class TIter, class TArg, class TObj>                
        static
            TIter FindRangeRetIter(TIter begin, TIter end,
            TArg&(TObj::*memGet)(void),
            const TArg& arg )
        {        
            TIter i = Finder<NoConstGetterFunctor>::Find<TObj>(begin, end, memGet, arg);
            Assertion(i != end);
            return i;
        }  

        // added
        template<class TIter, class TArg, class TObj>                
        static
            TIter FindRangeRetIter(TIter begin, TIter end,
            TArg*(TObj::*memGet)(void),
            const TArg& arg)
        {        
            TIter i = Finder<PtrGetterFunctor>::Find<TObj>(begin, end, memGet, arg);
            Assertion(i != end);
            return i;
        } 

        //==============================================================================================
        // FindRangeRetPtr - ��������� �������, ������������ ���������. ���������� ��� ������� ���� ���� �������
        // ��������� ��� ���. ����� � ��������� ����������.

        // Overload: without arg            
        template<class TIter, class TArg, class TFn> 
        static
            typename IterToPtrValue<TIter>::type    
            FindRangeRetPtr(TIter begin, TIter end, TFn mem, const TArg& arg)
        {        
            TIter i = FindRangeRetIter(begin, end, mem, arg);
            Assertion( i != end );
            if (i == end) return 0; // ���� NoThrow
            return IterContentToPtr(*i);            
        }

        // Overload: arg exist          
        template<class TIter, class TFn>
        static
            typename IterToPtrValue<TIter>::type    
            FindRangeRetPtr(TIter begin, TIter end, TFn mem)
        {        
            TIter i = FindRangeRetIter(begin, end, mem);
            Assertion( i != end );
            if (i == end) return 0; // ���� NoThrow
            return IterContentToPtr(*i);
        }

        //=======================================================================================    
        //FindContRetPtr - ��������� �������, ������������ ���������, ������������� ��� ������� ��������� ���������
        // � ������� ��� ���������. ����� �� ����������

        // Overload: without arg 
        template< class TContainer, class TFn>     
        static        
        typename ContainerTraits <TContainer>::ptr_value_type
            FindContRetPtr(TContainer& cont, TFn memFn)
        {        
            return FindRangeRetPtr(cont.begin(), cont.end(), memFn);
        }

        // Overload: arg exist
        template< class TContainer, class TFn, class TArg> 
        static        
        typename ContainerTraits <TContainer>::ptr_value_type
            FindContRetPtr(TContainer& cont, TFn memFn, const TArg& arg )
        {        
            return FindRangeRetPtr(cont.begin(), cont.end(), memFn, arg);
        }

        //=======================================================================================    
        //FindContRetIter - ��������� �������, ������������ ��������. ���������� ��� ������� ���� ���� �������
        // ��������� ��� ���. ����� �� ����������.

        // Overload: without arg 
        template< class TContainer, class TFn>     
        static
        typename ContainerTraits <TContainer>::iter_type
            FindContRetIter(TContainer& cont, TFn memFn)
        {        
            return FindRangeRetIter(cont.begin(), cont.end(), memFn);
        }

        // Overload: arg exist
        template< class TContainer, class TFn, class TArg> 
        static
        typename ContainerTraits <TContainer>::iter_type
            FindContRetIter(TContainer& cont, TFn memFn, const TArg& arg)
        {        
            return FindRangeRetIter(cont.begin(), cont.end(), memFn, arg);
        }

        //=======================================================================================
        // FindAllRange - ��������� �������, ������������ � ���������� �� ������ ��������
        // ��������� ��������� ���� ��������� �������� � ���������� ��������� ����������.


        // �������� c ����������            
        template<class TIter, class TArg, class TObj, class TOutCont> 
        static
            void FindAllRange(TIter begin, TIter end, 
            bool (TObj::*memPredicate)(const TArg&) const,  // method signature
            const TArg& arg,
            TOutCont& out)
        {        
            ContWriter<TOutCont> writer(out);
            Finder<PredicateFunctor>::FindAll<TObj>(begin, end, memPredicate, arg, writer);
            Assertion( writer.IsWrited() );
        }

        // �������� ��� ���������            
        template<class TIter, class TObj, class TOutCont>    
        static
            void FindAllRange(TIter begin, TIter end, 
            bool (TObj::*memPredicate)(void) const, // method signature
            TOutCont& out)  
        {        
            ContWriter<TOutCont> writer(out);
            Finder<PredicateFunctor>::FindAll<TObj>(begin, end, memPredicate, writer);
            Assertion( writer.IsWrited() );
        }

        template<class TIter, class TArg, class TObj, class TOutCont>    
        static
            void FindAllRange(TIter begin, TIter end, 
            TArg (TObj::*memGet)(void) const, // method signature
            const TArg& arg,
            TOutCont& out )
        {        
            ContWriter<TOutCont> writer(out);
            Finder<GetterFunctor>::FindAll<TObj>(begin, end, memGet, arg, writer);        
            Assertion( writer.IsWrited() );
        }    

        template<class TIter, class TArg, class TObj, class TOutCont>                
        static
            void FindAllRange(TIter begin, TIter end, 
            const TArg&(TObj::*memGet)(void)const, // method signature
            const TArg& arg,
            TOutCont& out )
        {        
            ContWriter<TOutCont> writer(out);
            /*TIter i = */Finder<GetterFunctor>::FindAll<TObj>(begin, end, memGet, arg, writer);
            Assertion( writer.IsWrited() );
        }  

        // ������������� �������� � ����������
        template<class TIter, class TArg, class TObj, class TOutCont>    
        static
            void FindAllRange(TIter begin, TIter end, 
            TArg& (TObj::*memGet)(void), // method signature
            const TArg& arg,
            TOutCont& out )
        {        
            ContWriter<TOutCont> writer(out);
            Finder<NoConstGetterFunctor>::FindAll<TObj>(begin, end, memGet, arg, writer);        
            Assertion( writer.IsWrited() );
        }    

        // ������������� �������� � ����������
        template<class TIter, class TArg, class TObj, class TOutCont>                
        static
            void FindAllRange(TIter begin, TIter end, 
            TArg* (TObj::*memGet)(void), // method signature
            const TArg& arg,
            TOutCont& out )
        {        
            ContWriter<TOutCont> writer(out);
            TIter i = Finder<PtrGetterFunctor>::FindAll<TObj>(begin, end, memGet, arg, writer);
            Assertion( writer.IsWrited() );
        }  

        //=======================================================================================
        // FindAllCont - ��������� ���� ������������ ����� ���� ��������� �� ������� ���������� �
        // ������������ ��������� �� ��������� ������� � ��������

        // �������� c ����������                        
        template<class TCont, class TArg, class Fn, class TOutCont>    
        static
            void FindAllCont(TCont& inCont, 
            Fn f, 
            const TArg& arg,
            TOutCont& out)  
        {        
            FindAllRange(inCont.begin(), inCont.end(), f, arg, out);
        }

        //// �������� c ����������                        
        //template<class TCont, class TArg, class Fn, class TOutCont>    
        //static
        //    void FindAllCont(TCont& inCont, 
        //    Fn f, 
        //    TArg& arg,
        //    TOutCont& out)  
        //{        
        //    FindAllRange(inCont.begin(), inCont.end(), f, arg, out);
        //}

        // �������� ��� ���������            
        template<class TCont, class Fn, class TOutCont>    
        static
            void FindAllCont(TCont& inCont, 
            Fn f, 
            TOutCont& out)  
        {        
            FindAllRange(inCont.begin(), inCont.end(), f, out);
        }
        
    };

} // namespace Utils


/*
*   MemFind<T> - ������������ ��� ������ ������� � ���������. �������� ������ �������� ����.
*   T ��������� ������� �� ��������� ����� (������� �� ������):
*   MemFind<> ��� MemFind<NoThrow> - ���������� ������� ��������� ��� �������� �����(� ����������� �� �������)
*   MemFind<EssAssertion> - �������� � ������������ ESS_ASSERT, ����� ������ �� �������� �������������;
*   MemFind<TutAssert> - �������� � ������������ TUT_ASSERT, ����� ������ �� �������� �������������;
*   MemFind<TException> - �������� � ������ ESS_THROW_MSG(TException, err), ��� err -- ���������� ������������� ������;
*
*   ����: ���� ���������� ��� ��������� ������ ��������� ESS_ASSERT ��� TUT_ASSERT � ������������ �������, 
*   �� ���� ������������ MemFind<ESS::Assertion ��� ESS::TutException>.
*
*   � �������� ����������� ����� ������������ ���������� STL, ���������� Qt, ���������� pointer container �� boost
*   ������� ����� ��������� �� ��������, ���� ��� ���������. �������� ������������� boost::shared_ptr<>
*   
*   �������� ������:
*   ��������� �����-������ #1 bool Name(const TArg& c) const; �������� ������ - currObj.Name(arg) == true;
*   ��������� �����-������ #2 bool Name(void) const;          �������� ������ - currObj.Name() == true;
*   
*   ��������� �����-������ #3 TArg GetName() const;           ��������  ������ - currObj.GetName() == arg;
*   ��������� �����-������ #4 const TArg& GetName() const;    ��������  ������ - currObj.GetName() == arg;
*   ��������� �����-������ #5 TArg& GetName();                ��������  ������ - currObj.GetName() == arg;
*
*   ������� ������
*   ������������� ����������� ������ �� ������ �� 4� ��������� ��������� �������� ���������� ����������
*   ��� ��������� ������� � ����� ���������� �������� ��� ��������� �� ��������� ������.
*
*   ���������� ������� ���������� ��������� ������������� �����:
*   Find[(First)/All][Cont/Range][RetPtr/(RetIter)]
*
*   ��������� FindAll ������ ���������� ���������, ������ ��� ���������� �� �����������. 
*
*
*   ����� ������ �������� ��� ���� �������, ����� ������������� TException:
*   
*   T* FindContRetPtr(cont, memFn)
*   T* FindContRetPtr(cont, memFn, arg)
*   Iter FindContRetIter(cont, memFn)
*   Iter FindContRetIter(cont, memFn, arg)
*   T* FindRangeRetPtr(iterBegin, iterEnd, memFn)
*   T* FindRangeRetPtr(iterBegin, iterEnd, memFn, arg)
*   Iter FindRangeRetIter(iterBegin, iterEnd, memFn)
*   Iter FindRangeRetIter(iterBegin, iterEnd, memFn, arg)    
*
*
*   ����, ����� ���� ���������:
*
*   void FindAllRange(iterBegin, iterEnd, memFn, arg, outCont)
*   void FindAllRange(iterBegin, iterEnd, memFn, outCont)
*   void FindAllCont(inCont, memFn, arg, outCont)
*   void FindAllCont(inCont, memFn, outCont)
*
*
*   ��� MemFind<TException>, ����� ������ ��������, err - std::string ������������ ��� TException:
*
*   T* FindContRetPtr(cont, memFn, err)
*   T* FindContRetPtr(cont, memFn, arg, err)
*   Iter FindContRetIter(cont, memFn, err)
*   Iter FindContRetIter(cont, memFn, arg, err)
*   T* FindRangeRetPtr(iterBegin, iterEnd, memFn, err)
*   T* FindRangeRetPtr(iterBegin, iterEnd, memFn, arg, err)
*   Iter FindRangeRetIter(iterBegin, iterEnd, memFn, err)
*   Iter FindRangeRetIter(iterBegin, iterEnd, memFn, arg, err)
*
*   ����, ��� ������ ���� ���������:
*
*   void FindAllRange(iterBegin, iterEnd, memFn, arg, outCont, err)
*   void FindAllRange(iterBegin, iterEnd, memFn, outCont, err)
*   void FindAllCont(inCont, memFn, arg, outCont, err)
*   void FindAllCont(inCont, memFn, outCont, err)
*
*   ����� memFn - ����� �-��� ������� ��������� #1, #3, #4, #5 ����� ��� ���� arg � ��������� #2 - ��� ���
*   
*
*
*/

#endif

