#ifndef INITVAR_H
#define INITVAR_H

#include "ErrorsSubsystem.h"

namespace Utils
{
    template<class T>
    class InitProp
    {
    public:

        InitProp() : m_initialized(false) {}            

        virtual ~InitProp(){}

        InitProp(const InitProp& rhs) 
        {
            m_var = rhs.m_var;
            m_initialized = rhs.m_initialized;
        }

        T& get()
        {
            Assert();
            return m_var;
        }

        const T& get() const
        {
            Assert();
            return m_var;
        }

        operator T&() 
        {
            Assert();
            return m_var;
        }

        operator const T&() const 
        {
            Assert();
            return m_var;
        }

        T* operator&()
        {
            Assert();
            return &m_var;
        }

        const T* operator&() const
        {
            Assert();
            return &m_var;
        }

        T& operator=(const T& t)
        {   
            m_initialized = true;
            return (m_var = t);            
        }

        bool IsInited() const
        {
            return m_initialized;
        }

        void Init(const T& var) 
        {
            ESS_ASSERT( !m_initialized );
            m_var = var;
            m_initialized = true;
        }

        void Clear()
        {   
            m_initialized = false;
        }
        
        bool OptionalCompare(const T& var) const
        {
			return (IsInited()) ? var == m_var : true;
        }

    protected:

        explicit InitProp(const T& var) : m_var(var), m_initialized(true) {}

    private:

        void Assert() const
        {
            ESS_ASSERT( m_initialized  );
        }

        T m_var;
        bool m_initialized;
    };

    template<class T, T var>
    class InitVar : public InitProp<T>
    {
    public:
        InitVar() : InitProp<T>(var) {}
    };
} //Utils

#endif

