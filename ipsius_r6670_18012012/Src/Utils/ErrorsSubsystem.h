#ifndef __ERRORSSUBSYSTEM__
#define __ERRORSSUBSYSTEM__

#include "stdafx.h"

/*
      Asserts and exceptions subsystem

      зы. Я знаю, что макросы изобретение дьявола...
*/

namespace ESS
{
    using std::string;
    using std::endl;

    /* 
        Класс-исключение с информацией о месте, где оно произошло
        Базовый класс для всех исключений
    */
    class BaseException : public std::exception // , boost::noncopyable
    {
        const char *m_pFile;
        const char *m_pFunc;
        int m_line;
        string m_trace;
        string m_textMsg;
        bool m_msgAddStackTrace;

        mutable string m_msg;

        void SetMsg(string &msg, bool addStackTrace) const;
    
    public:

        BaseException(const string& textMsg, 
            const char *pFile, const char *pFunc, int line, 
            bool forcedStackTraceCapture = true);

        virtual ~BaseException() throw() {}

        // disable stack trace info in what()
        void NoStackTraceInMsg();

        const string& getTextMessage() const { return m_textMsg; }
        string getLocation() const;
        string getTextAndPlace() const;

        const char* what() const throw();  // override

    };

    // --------------------------------------------------------------

    /* 
        Определение нового типа исключения
        Макрос, чтобы не возиться с конструктором и деструктором
    */
    #define ESS_TYPEDEF_FULL(newExceptionType, baseExceptionType)           \
    class newExceptionType : public baseExceptionType				        \
    {												                        \
    public:											                        \
        newExceptionType(const std::string& textMsg, const char *pFile, const char *pFunc, int line)	\
        : baseExceptionType(textMsg, pFile, pFunc, line)		   	        \
        { }											                        \
        ~newExceptionType() throw() {}                                      \
    }


    
    // Исключение, выбрасываемое тестами
    ESS_TYPEDEF_FULL(TutException, BaseException);

    // Информация о точке возникновения фатальной ошибки
    ESS_TYPEDEF_FULL(Assertion, BaseException);

    // Базовый тип для "обычных" исключений
    // ESS_TYPEDEF_FULL не использую, для протаскивания параметра CStackTraceInExceptions
    class Exception : public BaseException
    {									
    public:								
        Exception(const std::string& textMsg, const char *pFile, const char *pFunc, int line)	
            : BaseException(textMsg, pFile, pFunc, line, false)					    
        { }	

        ~Exception() throw() {}                                      
    };

    // Упрощенная форма макроса ESS_TYPEDEF_FULL
    #define ESS_TYPEDEF(newExceptionType)  ESS_TYPEDEF_FULL(newExceptionType, ESS::Exception)

    // Эта функция вызывается из ESS_ASSERT и ESS_HALT
    // processHook = false for call from hook code (block recursion)
    bool DoAssert(const Assertion &e, bool processHook = true);

    // Вызывается из ESS_UNEXPECTED_EXCEPTION
    bool UnexpectedException(const std::exception &e, const char *pFile, const char *pFunc, int line);

    // --------------------------------------------------------------


    /*
        Шаблон класса -- исключение с дополнительным параметром типа TProperty
        Тип TProperty должен иметь конструктор копирования
        Для передачи тяжеловесных некопируемых объектов использовать 
        что-то типа boost::shared_ptr
    */
    template<class TProperty>
    class BaseExceptionT : public Exception
    {
        TProperty m_property;

    public:

        BaseExceptionT(const string& textMsg, const char *pFile, const char *pFunc, int line, const TProperty &p)
            : Exception(textMsg, pFile, pFunc, line), m_property(p)
        { }

        ~BaseExceptionT() throw() {}

        const TProperty& getProperty() const
        {
            return m_property;
        }

        typedef TProperty PropertyType;

    };

    /* 
        Определение базового типа исключения с долнительным свойство типа TProperty
        Макрос, чтобы не возиться с конструктором и деструктором
    */
    #define ESS_TYPEDEF_T(newExceptionType, TProperty)                      \
    class newExceptionType : public ESS::BaseExceptionT<TProperty>   	    \
    {												                        \
    public:											                        \
        newExceptionType(const std::string& textMsg, const char *pFile, const char *pFunc, int line, const TProperty &p)	\
        : ESS::BaseExceptionT<TProperty>(textMsg, pFile, pFunc, line, p)    \
        { }											                        \
        ~newExceptionType() throw() {}                                      \
    }

    /* 
        Определение потомка для класса, созданного макросом ESS_TYPEDEF_T
    */
    #define ESS_TYPEDEF_T_EXTEND(newExceptionType, baseExceptionType)                   \
    class newExceptionType : public baseExceptionType   	                            \
    {												                                    \
    public:											                                    \
        newExceptionType(const std::string& textMsg, const char *pFile, const char *pFunc, int line, const baseExceptionType::PropertyType &p)	\
        : baseExceptionType(textMsg, pFile, pFunc, line, p)	    		                \
        { }											                                    \
        ~newExceptionType() throw() {}                                                  \
    }

    // --------------------------------------------------------------

    // Выбрасывание исключения (служебная)
    template<class T>
    bool RaiseException(T &t)
    {
        ProcessHook(&t);
        throw t;
        return true;
    }

    // Эта функция вызывается из TUT_ASSERT и ESS_THROW (служебная)
    template<typename T>
    bool DoException(const std::string& textMsg, const char *pFile, const char *pFunc, int line)
    {
        T t(textMsg, pFile, pFunc, line);
        return RaiseException(t);
    }

    template<typename T, typename TProperty>
    bool DoExceptionEx(const std::string& textMsg, const char *pFile, const char *pFunc, int line, const TProperty &p)
    {
        T t(textMsg, pFile, pFunc, line, p);
        return RaiseException(t);
    }

    // --------------------------------------------------------
    // Exceptions Hook System (experimental, thread unsafe!)

    class ExceptionHook;

    void RegisterHook(ExceptionHook *pHook);
    void UnregisterHook(ExceptionHook *pHook);
    void ProcessHook(const BaseException *pE);

    // базовый класс "перехватчик" исключений
    class ExceptionHook : boost::noncopyable
    {

    public:
        ExceptionHook()
        {
            RegisterHook(this);
        }

        virtual ~ExceptionHook()
        {
            UnregisterHook(this);
        }

        virtual void Hook(const BaseException *pE) = 0;
    };

    // это исключений прерывает выполнение hook обработчиков
    ESS_TYPEDEF(BreakHookChain);

    // это исключение позволяет "перекрыть" обрабатываемое через hook'и исключение другим типом
    ESS_TYPEDEF(HookRethrow);


    // -----------------------------------------------

    // ExceptionHook для подсчета перехваченных исключений типа TCountException
    template<typename TCountException>
    class ExceptionHookCounter : public ExceptionHook
    {
        int m_counter;

    public:

        ExceptionHookCounter() : m_counter(0)
        {
        }

        void Hook(const BaseException *pE)  // override
        {
            const TCountException *p = dynamic_cast<const TCountException*>(pE);
            if (p) m_counter++;
        }

        int getCounter() const { return m_counter; }

    };

    // ---------------------------------------------------

    // класс, для преобразования фатальной ошибки Assertion (ведущей к завершению работы программы) 
    // в исключение типа TAbortException
    template<class TAbortException = HookRethrow>
    class ExceptionHookRethrow : public ExceptionHook
    {
        int m_counter;

        enum { CInherCheck = boost::is_base_of<HookRethrow, TAbortException>::value };
        BOOST_STATIC_ASSERT( CInherCheck );

    public:

        ExceptionHookRethrow() : m_counter(0) {}

        void Hook(const BaseException *pE)  // override
        {
            if (dynamic_cast<const Assertion*>(pE))
            {
                m_counter++;
                ESS::DoException<TAbortException>("RETHROW " + pE->getTextAndPlace(), 0, 0, 0);
            }
        }

        int getCounter() const { return m_counter; }

    };


    // ---------------------------------------------------

    // never exit from here, if called from IRQ context
    bool HandleIrqContext(const char *pMsg, const char *pFile, const char *pFunc, int line);


}  // namespace ESS


// -------------------------------------------------------------

// Макрос для выбрасывания исключения заданного типа, производного от ESS::Exception
#define ESS_THROW(exceptionType)               ESS::DoException<exceptionType>(#exceptionType, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)
#define ESS_THROW_MSG(exceptionType, textMsg)  ESS::DoException<exceptionType>(textMsg,        __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)

// Макрос для выбрасывания исключения заданного типа с дополнительным свойством, значение которого передается вторым параметром макроса
#define ESS_THROW_T(exceptionType, p)  ESS::DoExceptionEx<exceptionType>(#exceptionType, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__, p)

/* 
    Макрос для проверки условия в ходе тестов, если условие ложно, выбрасывается
    исключение типа TutException
*/
#define TUT_ASSERT(_Expression)    (void)( (!!(_Expression)) || (ESS::HandleIrqContext(#_Expression, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)) || ( ESS::DoException<ESS::TutException>(#_Expression, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)) )

// Макрос для проверки условия, если условие ложно прекращается выполнение программы
#ifdef ENABLE_ESS_ASSERTIONS
#define ESS_ASSERT(_Expression)     (void)( (!!(_Expression)) || (ESS::HandleIrqContext(#_Expression, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)) || (ESS::DoAssert( ESS::Assertion(#_Expression, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__) )) )
#else
  #define ESS_ASSERT(_Expression)     /* nothing */
#endif

// Остановка выполнения программы с выдачей сообщения из std::string
#define ESS_HALT(textMsg)  (ESS::HandleIrqContext("ESS_HALT", __FILE__, BOOST_CURRENT_FUNCTION, __LINE__), ESS::DoAssert( ESS::Assertion(textMsg, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__) ))

// Оставновка выполнения программы, т.к. метод не реализован
#define ESS_UNIMPLEMENTED ESS_HALT("Unimplemented!")

// Остановка выполнения программы по причине неизвестного исключения
#define ESS_UNEXPECTED_EXCEPTION(unexpected) ( ESS::UnexpectedException(unexpected, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__) )

// -------------------------------------------------------------

/*

    *** Унифицированная система обработки ошибок и исключений ***

    Задачи, решаемые системой
    - исключения и критические ошибки содержат в себе информацию о месте исключения и стеке вызова
    - иерархия исключений, быстрое описание новых типов
    - возможность "перехватывать" исключения для получения точки останова при отладке для заданного
    типа исключения


    Макросы используются для автоматической подстановки __FILE__, __LINE__ и автоматической
    генерации новых типов исключений

    Тестовый макрос TUT_ASSERT не назван TEST_ASSERT во избежание потенциального
    конфликта имен в глобальном пространстве



    1. Макрос TUT_ASSERT(expression)

    Служит для выбрасывания исключения типа TutException на этапе тестирования, 
    т.е. когда код теста обнаруживает некорректную ситуацию

    Пример #1

    TUT_ASSERT(msgCount == 10)

    Пример #2

    TUT_ASSERT(0 && "Timeout")



    2. Макрос ESS_ASSERT(expression)

    Служит для детектированя фатальных ошибок, при обнаружении которых работа
    программы останавливается. Передача информации о месте ошибки происходит
    через класс Assertion

    Пример #1

    ESS_ASSERT(pBuff)

    Пример #2

    ESS_ASSERT(0 && "Out of memory")




    3. Макросы ESS_THROW(exceptionType), ESS_TYPEDEF_FULL(newExceptionType, baseType)

    Макрос ESS_TYPEDEF_FULL служить для описания нового типа исключения newExceptionType,
    производного от baseType.
    Макрос ESS_TYPEDEF -- аналогичен, в качестве базового класса служит Exception

    Макрос ESS_THROW выбрасывает исключение заданного типа exceptionType.

    Пример #1

    ESS_TYPEDEF(IoError);

    ESS_TYPEDEF_FULL(FileOpen, IoError);
    ESS_TYPEDEF_FULL(FileWrite, IoError);

    ...

    if (!file.Open()) ESS_THROW(FileOpen);

    ...

    if (!file.Write()) ESS_THROW(FileWrite);


    4. Hook Exception System (experimental, thread unsafe!)

    Предназначена для 
    - написания тестов для подсистемы ESS
    - возможности поставить бряк на выбрасывание исключения любого типа

    Пример использования -- в тестах


    5. Макросы ESS_TYPEDEF_T, ESS_TYPEDEF_T_EXTEND, ESS_THROW_T

      В случаях, когда с помощью исключения нужно передать какую-либо информацию,
    кроме его типа и места возникновения, можно использовать исключения со свойством
    любого типа.

      Такое исключение описывается макросом ESS_TYPEDEF_T, первый параметр имя
    нового типа исключений, второй -- тип его свойства.
      Для доступа с свойству необходимо использовать метод getProperty(),
    возвращающий константную ссылку. 

      Для создания новых типов исключений необходимо использовать макрос 
    ESS_TYPEDEF_T_EXTEND. Первый параметр -- имя потомка, второй -- имя базового
    класса. 

      Для выбрасывания исключения необходимо использовать макрос ESS_THROW_T,
    который, кроме типа исключения, содержит данные, которые запишуться в 
    свойство созданного объекта-исключения.

    Пример:

    // определение базового типа исключения NumericException со свойством типа double
    ESS_TYPEDEF_T(NumericException, double);

    // потомки класса NumericException
    ESS_TYPEDEF_T_EXTEND(NumericValueOverflow, NumericException); 
    ESS_TYPEDEF_T_EXTEND(NumericNegativeSqrt, NumericException);   

    // ...

    double val = 15.6;

    try
    {
       // выбрасывание исключения, к которому подшивается значение перменной val
       ESS_THROW_T(NumericNegativeSqrt, val);  
    }
    catch(const NumericException &e)   // перехват исключения по его базовому типу
    {
       TUT_ASSERT(e.getProperty() == val);
    }



*/


#endif

