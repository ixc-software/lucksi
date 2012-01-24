
#define TEMPLATE_SPEC(TemplType, Type)   \
    template<> struct TemplType<Type>    \
    {                                    \
        typedef bool value_type;         \
        static const value_type value = true;  \
    }       
    

    
    
