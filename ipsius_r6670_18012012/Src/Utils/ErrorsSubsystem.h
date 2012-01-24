#ifndef __ERRORSSUBSYSTEM__
#define __ERRORSSUBSYSTEM__

#include "stdafx.h"

/*
      Asserts and exceptions subsystem

      ��. � ����, ��� ������� ����������� �������...
*/

namespace ESS
{
    using std::string;
    using std::endl;

    /* 
        �����-���������� � ����������� � �����, ��� ��� ���������
        ������� ����� ��� ���� ����������
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
        ����������� ������ ���� ����������
        ������, ����� �� �������� � ������������� � ������������
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


    
    // ����������, ������������� �������
    ESS_TYPEDEF_FULL(TutException, BaseException);

    // ���������� � ����� ������������� ��������� ������
    ESS_TYPEDEF_FULL(Assertion, BaseException);

    // ������� ��� ��� "�������" ����������
    // ESS_TYPEDEF_FULL �� ���������, ��� ������������� ��������� CStackTraceInExceptions
    class Exception : public BaseException
    {									
    public:								
        Exception(const std::string& textMsg, const char *pFile, const char *pFunc, int line)	
            : BaseException(textMsg, pFile, pFunc, line, false)					    
        { }	

        ~Exception() throw() {}                                      
    };

    // ���������� ����� ������� ESS_TYPEDEF_FULL
    #define ESS_TYPEDEF(newExceptionType)  ESS_TYPEDEF_FULL(newExceptionType, ESS::Exception)

    // ��� ������� ���������� �� ESS_ASSERT � ESS_HALT
    // processHook = false for call from hook code (block recursion)
    bool DoAssert(const Assertion &e, bool processHook = true);

    // ���������� �� ESS_UNEXPECTED_EXCEPTION
    bool UnexpectedException(const std::exception &e, const char *pFile, const char *pFunc, int line);

    // --------------------------------------------------------------


    /*
        ������ ������ -- ���������� � �������������� ���������� ���� TProperty
        ��� TProperty ������ ����� ����������� �����������
        ��� �������� ������������ ������������ �������� ������������ 
        ���-�� ���� boost::shared_ptr
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
        ����������� �������� ���� ���������� � ������������ �������� ���� TProperty
        ������, ����� �� �������� � ������������� � ������������
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
        ����������� ������� ��� ������, ���������� �������� ESS_TYPEDEF_T
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

    // ������������ ���������� (���������)
    template<class T>
    bool RaiseException(T &t)
    {
        ProcessHook(&t);
        throw t;
        return true;
    }

    // ��� ������� ���������� �� TUT_ASSERT � ESS_THROW (���������)
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

    // ������� ����� "�����������" ����������
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

    // ��� ���������� ��������� ���������� hook ������������
    ESS_TYPEDEF(BreakHookChain);

    // ��� ���������� ��������� "���������" �������������� ����� hook'� ���������� ������ �����
    ESS_TYPEDEF(HookRethrow);


    // -----------------------------------------------

    // ExceptionHook ��� �������� ������������� ���������� ���� TCountException
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

    // �����, ��� �������������� ��������� ������ Assertion (������� � ���������� ������ ���������) 
    // � ���������� ���� TAbortException
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

// ������ ��� ������������ ���������� ��������� ����, ������������ �� ESS::Exception
#define ESS_THROW(exceptionType)               ESS::DoException<exceptionType>(#exceptionType, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)
#define ESS_THROW_MSG(exceptionType, textMsg)  ESS::DoException<exceptionType>(textMsg,        __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)

// ������ ��� ������������ ���������� ��������� ���� � �������������� ���������, �������� �������� ���������� ������ ���������� �������
#define ESS_THROW_T(exceptionType, p)  ESS::DoExceptionEx<exceptionType>(#exceptionType, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__, p)

/* 
    ������ ��� �������� ������� � ���� ������, ���� ������� �����, �������������
    ���������� ���� TutException
*/
#define TUT_ASSERT(_Expression)    (void)( (!!(_Expression)) || (ESS::HandleIrqContext(#_Expression, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)) || ( ESS::DoException<ESS::TutException>(#_Expression, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)) )

// ������ ��� �������� �������, ���� ������� ����� ������������ ���������� ���������
#ifdef ENABLE_ESS_ASSERTIONS
#define ESS_ASSERT(_Expression)     (void)( (!!(_Expression)) || (ESS::HandleIrqContext(#_Expression, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__)) || (ESS::DoAssert( ESS::Assertion(#_Expression, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__) )) )
#else
  #define ESS_ASSERT(_Expression)     /* nothing */
#endif

// ��������� ���������� ��������� � ������� ��������� �� std::string
#define ESS_HALT(textMsg)  (ESS::HandleIrqContext("ESS_HALT", __FILE__, BOOST_CURRENT_FUNCTION, __LINE__), ESS::DoAssert( ESS::Assertion(textMsg, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__) ))

// ���������� ���������� ���������, �.�. ����� �� ����������
#define ESS_UNIMPLEMENTED ESS_HALT("Unimplemented!")

// ��������� ���������� ��������� �� ������� ������������ ����������
#define ESS_UNEXPECTED_EXCEPTION(unexpected) ( ESS::UnexpectedException(unexpected, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__) )

// -------------------------------------------------------------

/*

    *** ��������������� ������� ��������� ������ � ���������� ***

    ������, �������� ��������
    - ���������� � ����������� ������ �������� � ���� ���������� � ����� ���������� � ����� ������
    - �������� ����������, ������� �������� ����� �����
    - ����������� "�������������" ���������� ��� ��������� ����� �������� ��� ������� ��� ���������
    ���� ����������


    ������� ������������ ��� �������������� ����������� __FILE__, __LINE__ � ��������������
    ��������� ����� ����� ����������

    �������� ������ TUT_ASSERT �� ������ TEST_ASSERT �� ��������� ��������������
    ��������� ���� � ���������� ������������



    1. ������ TUT_ASSERT(expression)

    ������ ��� ������������ ���������� ���� TutException �� ����� ������������, 
    �.�. ����� ��� ����� ������������ ������������ ��������

    ������ #1

    TUT_ASSERT(msgCount == 10)

    ������ #2

    TUT_ASSERT(0 && "Timeout")



    2. ������ ESS_ASSERT(expression)

    ������ ��� ������������� ��������� ������, ��� ����������� ������� ������
    ��������� ���������������. �������� ���������� � ����� ������ ����������
    ����� ����� Assertion

    ������ #1

    ESS_ASSERT(pBuff)

    ������ #2

    ESS_ASSERT(0 && "Out of memory")




    3. ������� ESS_THROW(exceptionType), ESS_TYPEDEF_FULL(newExceptionType, baseType)

    ������ ESS_TYPEDEF_FULL ������� ��� �������� ������ ���� ���������� newExceptionType,
    ������������ �� baseType.
    ������ ESS_TYPEDEF -- ����������, � �������� �������� ������ ������ Exception

    ������ ESS_THROW ����������� ���������� ��������� ���� exceptionType.

    ������ #1

    ESS_TYPEDEF(IoError);

    ESS_TYPEDEF_FULL(FileOpen, IoError);
    ESS_TYPEDEF_FULL(FileWrite, IoError);

    ...

    if (!file.Open()) ESS_THROW(FileOpen);

    ...

    if (!file.Write()) ESS_THROW(FileWrite);


    4. Hook Exception System (experimental, thread unsafe!)

    ������������� ��� 
    - ��������� ������ ��� ���������� ESS
    - ����������� ��������� ���� �� ������������ ���������� ������ ����

    ������ ������������� -- � ������


    5. ������� ESS_TYPEDEF_T, ESS_TYPEDEF_T_EXTEND, ESS_THROW_T

      � �������, ����� � ������� ���������� ����� �������� �����-���� ����������,
    ����� ��� ���� � ����� �������������, ����� ������������ ���������� �� ���������
    ������ ����.

      ����� ���������� ����������� �������� ESS_TYPEDEF_T, ������ �������� ���
    ������ ���� ����������, ������ -- ��� ��� ��������.
      ��� ������� � �������� ���������� ������������ ����� getProperty(),
    ������������ ����������� ������. 

      ��� �������� ����� ����� ���������� ���������� ������������ ������ 
    ESS_TYPEDEF_T_EXTEND. ������ �������� -- ��� �������, ������ -- ��� ��������
    ������. 

      ��� ������������ ���������� ���������� ������������ ������ ESS_THROW_T,
    �������, ����� ���� ����������, �������� ������, ������� ���������� � 
    �������� ���������� �������-����������.

    ������:

    // ����������� �������� ���� ���������� NumericException �� ��������� ���� double
    ESS_TYPEDEF_T(NumericException, double);

    // ������� ������ NumericException
    ESS_TYPEDEF_T_EXTEND(NumericValueOverflow, NumericException); 
    ESS_TYPEDEF_T_EXTEND(NumericNegativeSqrt, NumericException);   

    // ...

    double val = 15.6;

    try
    {
       // ������������ ����������, � �������� ����������� �������� ��������� val
       ESS_THROW_T(NumericNegativeSqrt, val);  
    }
    catch(const NumericException &e)   // �������� ���������� �� ��� �������� ����
    {
       TUT_ASSERT(e.getProperty() == val);
    }



*/


#endif

