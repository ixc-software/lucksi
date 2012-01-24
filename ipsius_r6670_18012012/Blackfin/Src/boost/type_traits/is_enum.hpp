#ifndef __IS_ENUM__
#define __IS_ENUM__

namespace boost
{
    
    template<class T>
    struct is_enum
    {
        typedef bool value_type;
        static const value_type value = true; // don't implemented
    };

       
}  // namespace boost

#endif