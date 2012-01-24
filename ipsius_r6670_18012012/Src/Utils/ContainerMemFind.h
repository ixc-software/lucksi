#ifndef FINDERBYMEMBER_H
#define FINDERBYMEMBER_H

#include "stdafx.h"
#include "IterTypeToValueType.h"
#include "ErrorsSubsystem.h"

/*
       MemFind<T> -- поиск объектов в контейнерах, подробное описание в конце этого файла

*/

namespace Utils
{

    namespace mpl = boost::mpl;    

    namespace Detail
    {
        // Способы выполнения assertions
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


        // аргумент шаблона MemFind по умолчанию. 
        // Выбирает специализацию не требующую строки для формирования исключений.
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

    // Базовая функциональная часть специализаций MemFind
    class FindBase
    {        

    protected:        
        
        // Метафункция преобразования типа в указатель на тип, если он уже таковым не является
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

        // Метафункция преобразования типа итератора в указатель на хранимый тип
        template<class TIter>
        class IterToPtrValue 
        {
            typedef typename IterTypeToValueType<TIter>::TValue TValue;

        public:

            typedef typename ValToPtrVal<TValue>::type type;

        };        

        // Метафункция извлечения типов переменной и итератора из типа контейнера
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

        // Функтор для предиката с аргументом
        // TArg  должен быть чистым те со снятым квалификатором иначе возможна ссылкана ссылку!
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
                // или return operator()(*pObj);
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

        // Специализация Функтора предиката без аргумента
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
                // или return operator()(*pObj);
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

        // Функтор для членов функций типа .getXXX()
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

        // added (todo объеденить с GetterFunctor)
        // Функтор для членов функций типа TArg* .getXXX() 
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

        // added (todo объеденить с GetterFunctor)
        // Функтор для членов функций типа .getXXX()
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

        // Выходной преобразователь типа в указатель на тип        
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

        // Специализируемый адаптер записи в контейнер
        template<class TContainer>
        class ContWriter        
        {
            TContainer& m_cont;
            bool m_wasWrited; // хотябы один раз выполнялась запись

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

            // Хотябы один раз выполнялась запись
            bool IsWrited() const
            {
                return m_wasWrited;
            }
        };

        //======================================================================

        // TODO Finder должен сам выбирать тип функтора?
        template< template<class,class,class> class Functor>
        class Finder
        {
            template<class TIter, class TFunctor>
            static TIter FindByFunctor(TIter begin, TIter end, TFunctor f)
            {
                TIter i;
                for (i = begin; i != end; ++i) // нет проверки валидности предела!
                    if ( f(*i) ) break;
                return i;
            }

            template<class TIter, class TFunctor, class TOut>
            static TIter FindAllByFunctor(TIter begin, TIter end, TFunctor f, ContWriter<TOut>& out)
            {
                TIter i;
                for (i = begin; i != end; ++i) // нет проверки валидности предела!
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
        или NoThrow - не использовать исключения, если не найденно возвращает итератор конца или нулевой указатель
        или EssAssertion - если не найденно ESS_ASSERT (без текста),
        или TutAsertion - если не найденно TUT_ASSERT (без текста), 
        или все типы исключений унаследованные от ESS::BaseException |*Exception*|, выбрасываемые с помощью ESS_THROW_MSG>
    */

    // Не специализированная по типу исключения версия.
    // Так сделано для того чтоб избежать конфликтов при перегрузке NoThrow версий методов с trow версиями,
    // отличающимися дополнительным аргументом сообщения об ошибке
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
        //FindRangeRetIter - Семейство функций, возвращающих итератор, производящий поиск по
        // диапазону, перегруженных по сигнатуре 4-х методов.


        // предикат c аргументом            
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

        // предикат без аргумента            
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
        // FindRangeRetPtr - семейство функций, возвращающих указатель. Перегрузка для случаев если поск требует
        // аргумента или нет. Поиск в диапазоне итераторов.

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
        //FindContRetPtr - семейство функций, возвращающих указатель. Перегрузка для случаев если поск требует
        // аргумента или нет. Поиск по контейнеру.

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
        //FindContRetIter - семейство функций, возвращающих итератор. Перегрузка для случаев если поск требует
        // аргумента или нет. Поиск по контейнеру.


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
        // FindAllRange - Семейство функций, записывающих в переданный по ссылке выходной
        // контейнер итераторы всех найденных объектов в переданном диапазоне итераторов.


        // предикат c аргументом            
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

        // предикат без аргумента            
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

        // неконстантный предикат с аргументом
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

        // неконстантный предикат с аргументом
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
        // FindAllCont - семейство фций производящих поиск всех элементов во входном контейнере и
        // записывающие итераторы на найденные объекты в выходной

        // предикат c аргументом                        
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

        // предикат без аргумента            
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

    // Специализация MemFind не выбрасывающая исключений или использующая утверждения так как описано в THowDoThrow.
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
        //FindRangeRetIter - Семейство функций, возвращающих итератор, производящий поиск по
        // диапазону, перегруженных по сигнатуре 4-х методов.


        // предикат c аргументом            
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

        // предикат без аргумента            
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
        // FindRangeRetPtr - семейство функций, возвращающих указатель. Перегрузка для случаев если поск требует
        // аргумента или нет. Поиск в диапазоне итераторов.

        // Overload: without arg            
        template<class TIter, class TArg, class TFn> 
        static
            typename IterToPtrValue<TIter>::type    
            FindRangeRetPtr(TIter begin, TIter end, TFn mem, const TArg& arg)
        {        
            TIter i = FindRangeRetIter(begin, end, mem, arg);
            Assertion( i != end );
            if (i == end) return 0; // если NoThrow
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
            if (i == end) return 0; // если NoThrow
            return IterContentToPtr(*i);
        }

        //=======================================================================================    
        //FindContRetPtr - семейство функций, возвращающих указатель, перегруженных для функций требующих аргумента
        // и функции без аргумента. Поиск по контейнеру

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
        //FindContRetIter - семейство функций, возвращающих итератор. Перегрузка для случаев если поск требует
        // аргумента или нет. Поиск по контейнеру.

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
        // FindAllRange - Семейство функций, записывающих в переданный по ссылке выходной
        // контейнер итераторы всех найденных объектов в переданном диапазоне итераторов.


        // предикат c аргументом            
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

        // предикат без аргумента            
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

        // неконстантный предикат с аргументом
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

        // неконстантный предикат с аргументом
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
        // FindAllCont - семейство фций производящих поиск всех элементов во входном контейнере и
        // записывающие итераторы на найденные объекты в выходной

        // предикат c аргументом                        
        template<class TCont, class TArg, class Fn, class TOutCont>    
        static
            void FindAllCont(TCont& inCont, 
            Fn f, 
            const TArg& arg,
            TOutCont& out)  
        {        
            FindAllRange(inCont.begin(), inCont.end(), f, arg, out);
        }

        //// предикат c аргументом                        
        //template<class TCont, class TArg, class Fn, class TOutCont>    
        //static
        //    void FindAllCont(TCont& inCont, 
        //    Fn f, 
        //    TArg& arg,
        //    TOutCont& out)  
        //{        
        //    FindAllRange(inCont.begin(), inCont.end(), f, arg, out);
        //}

        // предикат без аргумента            
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
*   MemFind<T> - предназначен для поиска объекта в коллекции. Критерии поиска описанны ниже.
*   T описывает реакцию на неудачный поиск (элемент не найден):
*   MemFind<> или MemFind<NoThrow> - возвращает нулевой указатель или итератор конца(в зависимости от функции)
*   MemFind<EssAssertion> - приводит к срабатыванию ESS_ASSERT, текст ошибки НЕ задается пользователем;
*   MemFind<TutAssert> - приводит к срабатыванию TUT_ASSERT, текст ошибки НЕ задается пользователем;
*   MemFind<TException> - приводит к вызову ESS_THROW_MSG(TException, err), где err -- переданная пользователем строка;
*
*   Прим: Если необходимо при неудачном поиске выполнить ESS_ASSERT или TUT_ASSERT с определенным текстом, 
*   то надо использовать MemFind<ESS::Assertion или ESS::TutException>.
*
*   В качестве контейнеров можно использовать контейнеры STL, контейнеры Qt, контейнеры pointer container из boost
*   Объекты могут храниться по значению, либо как указатели. Возможно использование boost::shared_ptr<>
*   
*   Критерии поиска:
*   сигнатура члена-метода #1 bool Name(const TArg& c) const; критерий выбора - currObj.Name(arg) == true;
*   сигнатура члена-метода #2 bool Name(void) const;          критерий выбора - currObj.Name() == true;
*   
*   сигнатура члена-метода #3 TArg GetName() const;           критерий  выбора - currObj.GetName() == arg;
*   сигнатура члена-метода #4 const TArg& GetName() const;    критерий  выбора - currObj.GetName() == arg;
*   сигнатура члена-метода #5 TArg& GetName();                критерий  выбора - currObj.GetName() == arg;
*
*   Функции поиска
*   предоставляют возможность поиска по любому из 4х критериев используя диапазон итераторов контейнера
*   или контейнер целиком и могут возвращать итератор или указатель на найденный объект.
*
*   Именование функций использует следующую мнемоническую схему:
*   Find[(First)/All][Cont/Range][RetPtr/(RetIter)]
*
*   Семейство FindAll всегда возвращает итераторы, версии для указателей не реализована. 
*
*
*   Поиск одного элемента для всех случаев, кроме использования TException:
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
*   тоже, поиск всех элементов:
*
*   void FindAllRange(iterBegin, iterEnd, memFn, arg, outCont)
*   void FindAllRange(iterBegin, iterEnd, memFn, outCont)
*   void FindAllCont(inCont, memFn, arg, outCont)
*   void FindAllCont(inCont, memFn, outCont)
*
*
*   для MemFind<TException>, поиск одного элемента, err - std::string передаваемая для TException:
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
*   тоже, для поиска всех элементов:
*
*   void FindAllRange(iterBegin, iterEnd, memFn, arg, outCont, err)
*   void FindAllRange(iterBegin, iterEnd, memFn, outCont, err)
*   void FindAllCont(inCont, memFn, arg, outCont, err)
*   void FindAllCont(inCont, memFn, outCont, err)
*
*   здесь memFn - метод ф-ция имеющая сигнатуру #1, #3, #4, #5 везде где есть arg и сигнатуру #2 - где нет
*   
*
*
*/

#endif

